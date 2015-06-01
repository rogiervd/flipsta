/*
Copyright 2014, 2015 Rogier van Dalen.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef FLIPSTA_AUTOMATON_HPP_INCLUDED
#define FLIPSTA_AUTOMATON_HPP_INCLUDED

#include <type_traits>
#include <set>
#include <map>

#include <boost/mpl/if.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include "utility/returns.hpp"

#include "rime/call_if.hpp"

#include "range/iterator_range.hpp"
#include "range/std/container.hpp"
#include "range/std/tuple.hpp"

#include "math/magma.hpp"

#include "core.hpp"
#include "label.hpp"
#include "error.hpp"
#include "arc.hpp"

namespace flipsta {

/**
\brief An automaton that stores its states and arcs explicitly.

All access operations are supported.

\tparam State The state type.
\tparam Label The label type on arcs.
\tparam TerminalLabel
    (optional)
    The label type for initial and final states.
    If Label is a symbol, this may need to be a different type to indicate an
    empty symbol sequence.
    If it is not given, it is set to the result type of calling
    <c>math::one \<Label>()</c>.
*/
template <class State, class Label, class TerminalLabel = void>
    class Automaton;

struct ExplicitAutomatonTag;

/// \cond DONT_DOCUMENT
template <class State, class Label, class TerminalLabel>
    struct AutomatonTagUnqualified <Automaton <State, Label, TerminalLabel>>
{ typedef ExplicitAutomatonTag type; };
/// \endcond

template <class State_, class Label_, class TerminalLabel_> class Automaton {
public:
    /**
    The state type, equal to the template parameter.
    */
    typedef State_ State;

    /**
    The label type, equal to the template parameter.
    */
    typedef Label_ Label;

    /**
    The terminal label type, by default
    */
    typedef typename boost::mpl::if_ <
            std::is_same <TerminalLabel_, void>,
            typename label::GetDefaultTerminalLabel <Label>::type,
            TerminalLabel_
        >::type TerminalLabel;

    static_assert (std::is_same <
        typename math::magma_tag <Label>::type,
        typename math::magma_tag <TerminalLabel>::type>::value,
        "The Label and TerminalLabel types must be in the same semiring.");

    typedef typename label::DefaultDescriptorFor <Label>::type Descriptor;

    typedef typename label::CompressedLabelType <Descriptor, Label>::type
        CompressedLabel;
    typedef typename label::CompressedLabelType <Descriptor, TerminalLabel
        >::type CompressedTerminalLabel;

    typedef ExplicitArc <State, CompressedLabel> Arc;

private:
    Descriptor descriptor_;

    // States must be findable in O(1) time, but the order must be consistent.
    typedef boost::multi_index_container <
        State,
        boost::multi_index::indexed_by <
            boost::multi_index::sequenced<>,
            boost::multi_index::hashed_unique <
                boost::multi_index::identity <State>>>
        > States;

    typedef std::pair <State, CompressedTerminalLabel> TerminalStateLabel;

    // The lists of terminal states must also be findable in O(1) time, and
    // the order must also be consistent.
    typedef boost::multi_index_container <
        TerminalStateLabel,
        boost::multi_index::indexed_by <
            boost::multi_index::sequenced<>,
            boost::multi_index::hashed_unique <
                boost::multi_index::member <
                    TerminalStateLabel, State, &TerminalStateLabel::first>>>
        > TerminalStates;

    typedef Forward BySource;
    typedef Backward ByDestination;

    /// Boost.MultiIndex key extractor that finds the source state.
    struct GetSource {
        typedef State result_type;

        result_type const & operator() (Arc const & arc) const
        { return arc.state (backward); }
    };

    /// Boost.MultiIndex key extractor that finds the destination state.
    struct GetDestination {
        typedef State result_type;

        result_type const & operator() (Arc const & arc) const
        { return arc.state (forward); }
    };

    /*
    arcsOn() must return the arcs in the same order every run of the program.
    Though I do not think that Boost.MultiIndex specifies this, there is no
    reason why the order would be different.
    */
    typedef boost::multi_index_container <
        Arc,
        boost::multi_index::indexed_by <
            boost::multi_index::hashed_non_unique <
                boost::multi_index::tag <BySource>, GetSource>,
            boost::multi_index::hashed_non_unique <
                boost::multi_index::tag <ByDestination>, GetDestination>
            >
        > Arcs;

    States states_;
    TerminalStates initialStates_;
    TerminalStates finalStates_;
    Arcs arcs_;

    TerminalStates const & terminalStatesContainer (Forward) const
    { return initialStates_; }
    TerminalStates & terminalStatesContainer (Forward)
    { return initialStates_; }

    TerminalStates const & terminalStatesContainer (Backward) const
    { return finalStates_; }
    TerminalStates & terminalStatesContainer (Backward)
    { return finalStates_; }

    struct SetTerminalLabelImplementation {
        template <class TerminalLabel2> void operator() (
            TerminalStates & terminalStates, Descriptor const & descriptor,
            State const & state, TerminalLabel2 const & label_) const
        {
            // Convert label to terminal label.
            // (For example, sequences may have to be empty.)
            TerminalLabel label (label_);
            std::pair <State, CompressedTerminalLabel> newEntry (
                state, label::compress (descriptor, label));
            auto result = terminalStates.push_back (newEntry);
            // Fails if the state already had a label.
            // sudo push_back.
            if (!result.second)
                terminalStates.replace (result.first, newEntry);
        }
    };

    struct UnsetTerminalLabelImplementation {
        template <class ZeroTerminalLabel> void operator() (
            TerminalStates & terminalStates, Descriptor const &,
            State const & state, ZeroTerminalLabel const &) const
        { terminalStates.template get <1>().erase (state); }
    };

public:
    /**
    \brief Initialise with no states, no arcs, and a default-constructed
    descriptor.
    */
    Automaton() : descriptor_() {}

    /**
    \brief Initialise with no states, no arcs, and the descriptor as given.
    \param descriptor The value of the descriptor to use.
    */
    explicit Automaton (Descriptor const & descriptor)
    : descriptor_ (descriptor) {}

    /* Mutable methods. */

    /**
    \brief Add a new state to the automaton.
    \throw StateExists iff the state is already in the automaton.
    */
    void addState (State const & state) {
        if (this->hasState (state))
            throw StateExists() << errorInfoState <State> (state);
        states_.push_back (state);
    }

    /**
    \brief Add an arc to the automaton.

    The states must already be in the automaton.
    \param source The source state.
    \param destination The destination state.
    \param label the label on the arc.
    \throw StateNotFound if the source or destination state does not exist.
    */
    void addArc (State const & source, State const & destination,
        Label const & label)
    {
        if (!this->hasState (source))
            throw StateNotFound() << errorInfoState <State> (source);
        if (!this->hasState (destination))
            throw StateNotFound() << errorInfoState <State> (destination);
        arcs_.insert (
            Arc (forward, source, destination,
                label::compress (descriptor_, label)));
    }

    /**
    \brief Set the initial or final label for a state.

    If the label equals semiring-zero, the state is removed from the set of
    terminal states.
    If the label is non-zero, the state is added with the label, or if the
    state is already a terminal state, then the label is updated.
    \param direction If Direction is forward, set the initial state label;
        if it is backward, set the final state label.
    \param state The state to set as a terminal state.
        This must be in the automaton.
    \param label The label to set on the state.
        The label may be of a different type from TerminalLabel, but it must
        either be equal to semiring-zero, or it must be convertible to
        TerminalLabel.
    */
    template <class Direction, class TerminalLabel2>
        void setTerminalLabel (Direction direction,
            State const & state, TerminalLabel2 const & label)
    {
        if (!hasState (state))
            throw StateNotFound() << errorInfoState <State> (state);
        static_assert (std::is_same <
            typename math::magma_tag <TerminalLabel>::type,
            typename math::magma_tag <TerminalLabel2>::type>::value,
            "The terminal label passed must be of the same magma.");

        rime::call_if (label == math::zero <TerminalLabel>(),
            UnsetTerminalLabelImplementation(),
            SetTerminalLabelImplementation(),
            terminalStatesContainer (direction), descriptor_, state, label);
    }

    /* Methods for immutable access. */
    /// \cond DONT_DOCUMENT
    // (These merely implement the general access functions.)
    Descriptor const & descriptor() const { return descriptor_; }

    auto states() const RETURNS (range::make_iterator_range (states_));

    bool hasState (State const & state) const {
        auto & stateIndex = states_.template get<1>();
        return stateIndex.find (state) != stateIndex.end();
    }

    template <class Direction>
        typename std::result_of <
            range::callable::make_iterator_range (TerminalStates const &)>::type
        terminalStatesCompressed (Direction direction) const
    { return range::make_iterator_range (terminalStatesContainer (direction)); }

private:
    /// Return zero in the internal label type.
    struct ReturnZeroCompressed {
        template <class Iterator> auto operator() (Iterator const &) const
        RETURNS (math::zero <CompressedTerminalLabel>());
    };

    /// Return the second element of the pair the iterator points to.
    struct DereferenceSecond {
        template <class Iterator>
            auto operator() (Iterator const & iterator) const
        RETURNS (iterator->second);
    };

public:
    template <class Direction>
        typename label::GeneraliseToZero <CompressedTerminalLabel>::type
            terminalLabelCompressed (Direction direction, State const & state)
            const
    {
        auto const & states =
            terminalStatesContainer (direction).template get<1>();
        auto position = states.find (state);
        return rime::call_if <math::merge_magma> (position == states.end(),
            ReturnZeroCompressed(), DereferenceSecond(), position);
    }

    template <class Direction>
        range::iterator_range <typename
            Arcs::template index <Direction>::type::const_iterator>
        arcsOnCompressed (Direction, State const & state) const
    {
        auto startAndEnd = arcs_.template get <Direction>().equal_range (state);
        return range::make_iterator_range (
            startAndEnd.first, startAndEnd.second);
    }
    /// \endcond
};

} // namespace flipsta

#endif // FLIPSTA_AUTOMATON_HPP_INCLUDED
