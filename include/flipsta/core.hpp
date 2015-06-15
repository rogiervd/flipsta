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

/** \file
Basic handling of automata.
*/

#ifndef FLIPSTA_CORE_HPP_INCLUDED
#define FLIPSTA_CORE_HPP_INCLUDED

#include <type_traits>

#include <boost/utility/enable_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/and.hpp>

#include "utility/returns.hpp"
#include "utility/pointee.hpp"

#include "utility/nested_callable.hpp"

#include "rime/core.hpp"

#include "range/core.hpp"
#include "range/transform.hpp"
#include "range/tuple.hpp"

#include "math/magma.hpp"

#include "core/dense.hpp"
#include "core/callable.hpp"
#include "label.hpp"

namespace flipsta {

/* Directions. */

/**
\brief Empty type that indicates the forward direction in an automaton.
*/
struct Forward {};

/**
\brief Empty type that indicates the backward direction in an automaton.
*/
struct Backward {};

/**
\brief Evaluate to \c true iff \a Direction is a direction type, even if it is
reference- or cv-qualified.
*/
template <class Direction> struct IsDirection;

/// \cond DONT_DOCUMENT
template <class Direction> struct IsDirectionUnqualified : rime::false_type {};

template <class Direction> struct IsDirection
: IsDirectionUnqualified <typename std::decay <Direction>::type> {};

template <> struct IsDirectionUnqualified <Forward> : rime::true_type {};
template <> struct IsDirectionUnqualified <Backward> : rime::true_type {};
/// \endcond

/**
\brief Empty object that indicates the forward direction of traversing an
automaton.
*/
static constexpr Forward forward = {};

/**
\brief Empty object that indicates the backward direction of traversing an
automaton.
*/
static constexpr Backward backward = {};

/**
\brief Evaluate to the opposite direction: \ref Forward if \a Direction is
\ref Backward and vice versa.
*/
template <class Direction> struct Opposite;

/// \cond DONT_DOCUMENT
template <> struct Opposite <Forward> { typedef Backward type; };
template <> struct Opposite <Backward> { typedef Forward type; };
/// \endcond

/**
\brief Return the opposite direction.

E.g. <c>opposite (forward)</c> returns \c backward and vice versa.
*/
inline Backward opposite (Forward) { return Backward(); }
inline Forward opposite (Backward) { return Forward(); }

/** \brief
Evaluate to math::left if \a Direction is \ref Forward, and to math::right if
\a Direction is \ref Backward.
*/
template <class Direction> struct MathDirection;

/// \cond DONT_DOCUMENT
template <> struct MathDirection <Forward> { typedef math::left type; };
template <> struct MathDirection <Backward> { typedef math::right type; };
/// \endcond

/* Operations */

struct NotAnAutomaton;

/**
\brief Specialise this for an unqualified type to define it as an automaton.
*/
template <class Automaton> struct AutomatonTagUnqualified
{ typedef NotAnAutomaton type; };

/**
\brief Evaluate to a tag type for the automaton type.

To mark a type as an automaton, specialise \ref AutomatonTagUnqualified, which
receives an unqualified type.
*/
template <class Automaton> struct AutomatonTag
: AutomatonTagUnqualified <typename std::decay <Automaton>::type> {};

/**
\brief Evaluate to a tag type for the automaton type pointed to.

To mark a type as an automaton, specialise \ref AutomatonTagUnqualified, which
receives an unqualified type.
*/
template <class AutomatonPtr> struct PtrAutomatonTag
: AutomatonTag <typename utility::pointee <AutomatonPtr>::type> {};

/**
\brief Evaluate to \c true iff \a Automaton is an automaton.
*/
template <class Automaton> struct IsAutomaton
: boost::mpl::not_ <
    std::is_same <typename AutomatonTag <Automaton>::type, NotAnAutomaton>> {};

namespace operation {

    /**
    \brief Specialise this to return the descriptor used by the automaton.

    Alternatively, provide a \c .descriptor() member function.

    This powers the automatic conversion between expanded and compressed
    representations of labels.

    \tparam AutomatonTag The automaton tag (unqualified).
    \tparam Automaton
        The automaton type, reference-qualified and possibly const-qualified.
    */
    template <class AutomatonTag, class Automaton, class Enable = void>
        struct Descriptor
    /*{
        ... operator() (Automaton const &) const ...
    }*/;

    /**
    \brief Specialise this to return a list of states.

    Alternatively, implement a \c .states() member function.

    The order must be consistent between runs of the same program.
    E.g. the order must not depend on hashes based on pointers.

    \tparam AutomatonTag The automaton tag (unqualified).
    \tparam Automaton
        The automaton type, reference-qualified and possibly const-qualified.
    */
    template <class AutomatonTag, class Automaton, class Enable = void>
        struct States
    /*{
        ... operator() (Automaton const &) const ...
    }*/;

    /**
    \brief Specialise this to return whether a state is in an automaton.

    Alternatively, implement a <c>.hasState (State)</c> member function.

    \tparam AutomatonTag The automaton tag (unqualified).
    \tparam Automaton
        The automaton type, reference-qualified and possibly const-qualified.
    \tparam State The state of which the existence is to be checked.
    */
    template <class AutomatonTag, class Automaton, class State,
        class Enable = void>
    struct HasState
    /*{
        ... operator() (Automaton const &, State const &) const ...
    }*/;

    /**
    \brief Return a list of terminal states and their expanded label for
    Direction.

    They must be pairs (or equivalent range types) of the state and the label.

    This does not normally need to be implemented explicitly if
    \c TerminalStatesCompressed is implemented.
    The standard implementation uses \c TerminalStatesCompressed and converts
    the labels on the fly using \c descriptor().
    If this is not the desired behaviour, either specialise this or implement a
    <c>.terminalStates(Direction)</c> member function.

    The order must be consistent between runs of the same program.
    E.g. the order must not depend on hashes based on pointers.

    \tparam AutomatonTag The automaton tag (unqualified).
    \tparam Automaton
        The automaton type, reference-qualified and possibly const-qualified.
    \tparam Direction
        Indicates whether the interest is in the start states (\c Forward) or in
        the final states (\c Backward).
        This is unqualified.
    */
    template <class AutomatonTag, class Automaton, class Direction,
        class Enable = void>
    struct TerminalStates
    /*{
        ... operator() (Automaton &&, Direction const &) const ...
    }*/;

    /**
    \brief Specialise this to return a list of terminal states and their
    compressed label for Direction.

    They must be pairs (or equivalent range types) of the state and the label.
    Alternatively, implement a <c>.terminalStatesCompressed (Direction)<c/>
    member function.

    This is normally called automatically by TerminalStates, and put in a
    wrapper that transforms the labels to their expanded representation.

    The order must be consistent between runs of the same program.
    E.g. the order must not depend on hashes based on pointers.

    \tparam AutomatonTag The automaton tag (unqualified).
    \tparam Automaton
        The automaton type, reference-qualified and possibly const-qualified.
    \tparam Direction
        Indicates whether the interest is in the start states (\c Forward) or in
        the final states (\c Backward).
        This is unqualified.
    */
    template <class AutomatonTag, class Automaton, class Direction,
        class Enable = void>
    struct TerminalStatesCompressed
    /*{
        ... operator() (Automaton &&, Direction const &) const ...
    }*/;

    /**
    \brief Return the terminal label for a state.

    There is normally no need to implement this explicitly, because the default
    implementation automatically uses TerminalLabelCompressed and converts the
    label using Descriptor.

    If different behaviour is required, specialise this or implement a
    <c>.terminalLabel (State, Direction)</c> member function, to return the
    terminal label for a state.

    If the state is not a terminal label, <c>math::zero <Label>()</c> should be
    returned.
    rime::merge_magma can be used to merge different label types.

    \tparam AutomatonTag The automaton tag (unqualified).
    \tparam Automaton
        The automaton type, reference-qualified and possibly const-qualified.
    \tparam Direction
        Indicates whether the interest is in the start states (\c Forward) or in
        the final states (\c Backward).
        This is unqualified.
    \tparam State The state of which the label is to be returned.
    */
    template <class AutomatonTag, class Automaton, class Direction, class State,
        class Enable = void>
    struct TerminalLabel
    /*{
        ... operator() (Automaton &&, Direction const &, State &&) const ...
    }*/;

    /**
    \brief Specialise this member function, to return the compressed terminal
    label for a state.

    It is also possible to provide a
    <c>.terminalLabelCompressed (State, Direction)</c> member function.

    If the state is not a terminal label, <c>math::zero <CompressedLabel>()</c>
    should be returned.
    rime::merge_magma can be used to merge different label types.

    \tparam AutomatonTag The automaton tag (unqualified).
    \tparam Automaton
        The automaton type, reference-qualified and possibly const-qualified.
    \tparam Direction
        Indicates whether the interest is in the start states (\c Forward) or in
        the final states (\c Backward).
        This is unqualified.
    \tparam State The state of which the label is to be returned.
    */
    template <class AutomatonTag, class Automaton, class Direction, class State,
        class Enable = void>
    struct TerminalLabelCompressed
    /*{
        ... operator() (Automaton &&, Direction const &, State &&) const ...
    }*/;

    /**
    \brief Return a list of arcs attached to a state in Direction.

    For \a Forward, return the arcs that have \a state as their source state;
    for \a Backward, return the arcs that have \a state as their destination
    state.
    This does not normally need to be implemented explicitly if ArcsOnCompressed
    is implemented.

    The automatic default implementation calls ArcsOnCompressed and transforms
    the labels on the arcs on the fly to their expanded representation.
    If that is not the desired behaviour, specialise this or implement an
    <c>arcsOn (Direction, State &&)</c> member function.

    The order must be consistent between runs of the same program.

    \tparam AutomatonTag The automaton tag (unqualified).
    \tparam Automaton
        The automaton type, reference-qualified and possibly const-qualified.
    \tparam Direction
        Indicates whether the interest is in the start states (\c Forward) or in
        the final states (\c Backward).
        This is unqualified.
    \tparam State The state on which the arcs of interest are.
    */
    template <class AutomatonTag, class Automaton, class Direction, class State,
        class Enable = void>
    struct ArcsOn
    /*{
        ... operator() (Automaton &&, Direction const &, State &&) const ...
    }*/;

    /**
    \brief Return a list of arcs attached to a state in Direction.

    For \a Forward, return the arcs that have \a state as their source state;
    for \a Backward, return the arcs that have \a state as their destination
    state.
    The arcs carry the compressed label.

    Specialise this or implement an <c>arcsOn (Direction, State &&)</c> member
    function.

    The order must be consistent between runs of the same program.

    \tparam AutomatonTag The automaton tag (unqualified).
    \tparam Automaton
        The automaton type, reference-qualified and possibly const-qualified.
    \tparam Direction
        Indicates whether the interest is in the start states (\c Forward) or in
        the final states (\c Backward).
        This is unqualified.
    \tparam State The state on which the arcs of interest are.
    */
    template <class AutomatonTag, class Automaton, class Direction, class State,
        class Enable = void>
    struct ArcsOnCompressed
    /*{
        ... operator() (Automaton &&, Direction const &, State &&) const ...
    }*/;

    /**
    \brief Return the states of the automaton in topological order.

    The interface and the implementation for the general case are given in
    topologicalOrder.hpp.
    Usually, the general implementation is good enough.
    However, there may be a reason (space or time efficiency, say) to implement
    this for a specific type of automaton.
    In that case, specialise this or implement a
    <c>topologicalOrder (Direction)</c> member function.

    This must return a range that is guaranteed to remain valid until the
    automaton goes out of scope.
    The return value may, however, be a heavyweight range.
    The order must be consistent between runs of the same program.

    \tparam AutomatonTag The automaton tag (unqualified).
    \tparam Automaton
        The automaton type, reference-qualified and possibly const-qualified.
    \tparam Direction
        Indicates whether the interest is in the start states (\c Forward) or in
        the final states (\c Backward).
        This is unqualified.
    */
    template <class AutomatonTag, class Automaton, class Direction,
        class Enable = void>
    struct TopologicalOrder
    /*{
        ... operator() (Automaton &&, Direction const &) const ...
    }*/;

} // namespace operation

namespace callable {

    struct Descriptor {
        template <class Automaton> struct apply
        : operation::Descriptor <typename AutomatonTag <Automaton>::type,
            Automaton &&> {};

        template <class Automaton>
            auto operator() (Automaton && automaton) const
        RETURNS (apply <Automaton>() (std::forward <Automaton> (automaton)));
    };

    struct States {
        template <class Automaton> struct apply
        : operation::States <typename AutomatonTag <Automaton>::type,
            Automaton &&> {};

        template <class Automaton>
            auto operator() (Automaton && automaton) const
        RETURNS (apply <Automaton>() (std::forward <Automaton> (automaton)));
    };

    struct HasState {
        template <class Automaton, class State> struct apply
        : operation::HasState <typename AutomatonTag <Automaton>::type,
            Automaton &&, State> {};

        template <class Automaton, class State>
            auto operator() (Automaton && automaton, State && state) const
        RETURNS (apply <Automaton, State>() (
            std::forward <Automaton> (automaton),
            std::forward <State> (state)));
    };

    struct TerminalStates {
        template <class Automaton, class Direction> struct apply
        : operation::TerminalStates <typename AutomatonTag <Automaton>::type,
            Automaton &&, typename std::decay <Direction>::type> {};

        template <class Automaton, class Direction>
            auto operator() (
                Automaton && automaton, Direction const & direction) const
        RETURNS (apply <Automaton, Direction>() (
            std::forward <Automaton> (automaton), direction));
    };

    struct TerminalStatesCompressed {
        template <class Automaton, class Direction> struct apply
        : operation::TerminalStatesCompressed <
            typename AutomatonTag <Automaton>::type,
            Automaton &&, typename std::decay <Direction>::type> {};

        template <class Automaton, class Direction>
            auto operator() (Automaton && automaton,
                Direction const & direction) const
        RETURNS (apply <Automaton, Direction>() (
            std::forward <Automaton> (automaton), direction));
    };

    struct TerminalLabel {
        template <class Automaton, class Direction, class State> struct apply
        : operation::TerminalLabel <typename AutomatonTag <Automaton>::type,
            Automaton &&, typename std::decay <Direction>::type,
            State &&> {};

        template <class Automaton, class Direction, class State>
            auto operator() (Automaton && automaton,
                Direction const & direction, State && state) const
        RETURNS (apply <Automaton, Direction, State>() (
            std::forward <Automaton> (automaton), direction,
            std::forward <State> (state)));
    };

    struct TerminalLabelCompressed {
        template <class Automaton, class Direction, class State> struct apply
        : operation::TerminalLabelCompressed <
            typename AutomatonTag <Automaton>::type,
            Automaton &&, typename std::decay <Direction>::type,
            State &&> {};

        template <class Automaton, class Direction, class State>
            auto operator() (Automaton && automaton,
                Direction const & direction, State && state) const
        RETURNS (apply <Automaton, Direction, State>() (
            std::forward <Automaton> (automaton), direction,
            std::forward <State> (state)));
    };

    struct ArcsOn {
        template <class Automaton, class Direction, class State> struct apply
        : operation::ArcsOn <typename AutomatonTag <Automaton>::type,
            Automaton &&, typename std::decay <Direction>::type,
            State &&> {};

        template <class Automaton, class Direction, class State>
            auto operator() (Automaton && automaton,
                Direction const & direction, State && state) const
        RETURNS (apply <Automaton, Direction, State>() (
            std::forward <Automaton> (automaton), direction,
            std::forward <State> (state)));
    };

    struct ArcsOnCompressed {
        template <class Automaton, class Direction, class State> struct apply
        : operation::ArcsOnCompressed <typename AutomatonTag <Automaton>::type,
            Automaton &&, typename std::decay <Direction>::type,
            State &&> {};

        template <class Automaton, class Direction, class State>
            auto operator() (Automaton && automaton,
                Direction const & direction, State && state) const
        RETURNS (apply <Automaton, Direction, State>() (
            std::forward <Automaton> (automaton), direction,
            std::forward <State> (state)));
    };

    struct Times {
        template <class Left, class Right>
            auto operator() (Forward, Left && left, Right && right) const
        RETURNS (::math::times (
            std::forward <Left> (left), std::forward <Right> (right)));

        template <class Left, class Right>
            auto operator() (Backward, Left && left, Right && right) const
        RETURNS (::math::times (
            std::forward <Right> (right), std::forward <Left> (left)));
    };

} // namespace callable

/**
\brief The general type of state that the automaton uses.

If \a Automaton is not an automaton, then this does not contain any type, so
that SFINAE is possible.
*/
template <class Automaton, class Enable = void> struct StateType {};

/// \cond DONT_DOCUMENT
template <class Automaton> struct StateType <Automaton,
    typename boost::enable_if <IsAutomaton <Automaton>>::type>
{ typedef typename std::decay <Automaton>::type::State type; };
/// \endcond

/**
\brief The general type of state that the automaton pointed to uses.

If \a Automaton is not an automaton, then this does not contain any type, so
that SFINAE is possible.
*/
template <class AutomatonPtr> struct PtrStateType
: StateType <typename utility::pointee <AutomatonPtr>::type> {};

/**
\brief Compute the general type of label that the automaton uses.

If \a Automaton is not an automaton, then this does not contain any type, so
that SFINAE is possible.
*/
template <class Automaton, class Enable = void> struct LabelType {};

/// \cond DONT_DOCUMENT
template <class Automaton> struct LabelType <Automaton,
    typename boost::enable_if <IsAutomaton <Automaton>>::type>
{ typedef typename std::decay <Automaton>::type::Label type; };
/// \endcond

/**
\brief Compute the general type of label that the automaton pointed to uses.

If \a Automaton is not an automaton, then this does not contain any type, so
that SFINAE is possible.
*/
template <class AutomatonPtr> struct PtrLabelType
: LabelType <typename utility::pointee <AutomatonPtr>::type> {};

/**
\brief Compute the general type of descriptor that the automaton uses.

If \a Automaton is not an automaton, then this does not contain any type, so
that SFINAE is possible.
*/
template <class Automaton, class Enable = void> struct DescriptorType {};

/// \cond DONT_DOCUMENT
template <class Automaton> struct DescriptorType <Automaton,
    typename boost::enable_if <IsAutomaton <Automaton>>::type>
{ typedef typename std::decay <Automaton>::type::Descriptor type; };
/// \endcond

/** \brief
Compute the general type of descriptor that the automaton pointed to uses.

If \a Automaton is not an automaton, then this does not contain any type, so
that SFINAE is possible.
*/
template <class AutomatonPtr> struct PtrDescriptorType
: DescriptorType <typename utility::pointee <AutomatonPtr>::type> {};

/**
\brief Compute the compressed label type that the automaton uses.

If \a Automaton is not an automaton, then this does not contain any type, so
that SFINAE is possible.
*/
template <class Automaton, class Enable = void> struct CompressedLabelType {};

/// \cond DONT_DOCUMENT
template <class Automaton> struct CompressedLabelType <Automaton,
    typename boost::enable_if <IsAutomaton <Automaton>>::type>
: label::CompressedLabelType <typename DescriptorType <Automaton>::type,
    typename LabelType <Automaton>::type> {};
/// \endcond

/**
\brief Compute the compressed label type that the automaton pointed to uses.

If \a Automaton is not an automaton, then this does not contain any type, so
that SFINAE is possible.
*/
template <class AutomatonPtr> struct PtrCompressedLabelType
: CompressedLabelType <typename utility::pointee <AutomatonPtr>::type> {};

/**
\brief Return the descriptor that the automaton uses to convert between expanded
and compressed representations of labels.

\param automaton The automaton.
*/
static auto constexpr descriptor = callable::Descriptor();

/**
\brief Return A range of states in the automaton.

\param automaton The automaton.
*/
static auto constexpr states = callable::States();

/**
\brief Return \c true iff \a state is in \a automaton.

\param automaton The automaton.
\param state The state of interest.
*/
static auto constexpr hasState = callable::HasState();

/**
\brief Return a range of terminal states and their labels.

If \a direction is forward, then the initial states are returned; if
\a direction is backward, then the final states are returned.
The result type is a range of pairs, or another range type, of states and
labels.

\param automaton The automaton.
\param direction The direction from which the states of interest start.
*/
static auto constexpr terminalStates = callable::TerminalStates();

/**
\brief Return a range of terminal states and their compressed labels.

If \a direction is forward, then the initial states are returned; if
\a direction is backward, then the final states are returned.
The result type is a range of pairs, or another range type, of states and
labels.

\param automaton The automaton.
\param direction The direction from which the states of interest start.

\sa terminalStates
*/
static auto constexpr terminalStatesCompressed
    = callable::TerminalStatesCompressed();

/**
\brief Return the terminal label of a state.

If \a direction is \c forward, then this is the start label; if \a direction is
\c backward, then the final label is returned.

If the state is not a terminal state in direction, or the state does not
exist, return <c>math::zero\<Label>()</c>.

\param automaton
    The automaton to find the state in.
\param direction
    Indicates whether the interest is in a start state (\c Forward) or in
    a final state (\c Backward).
\param State
    The state for which the label is sought.
*/
static auto constexpr terminalLabel = callable::TerminalLabel();

/**
\brief Return the compressed terminal label of a state.

If \a direction is \c forward, then this is the start label; if \a direction is
\c backward, then the final label is returned.

If the state is not a terminal state in direction, or the state does not
exist, return <c>math::zero\<Label>()</c>.
*/
static auto constexpr terminalLabelCompressed
    = callable::TerminalLabelCompressed();

/**
\brief Return A range of all arcs connected to state \a state in the automaton.

If Direction is forward, the arcs out_of \a state are returned.
If Direction is backward, the arcs into \a state are returned.

\param automaton The automaton.
\param direction The direction to take from \a state.
\param state The state of interest.

\pre <c>hasState (automaton, state)</c>.
*/
static auto constexpr arcsOn = callable::ArcsOn();

/**
\brief Return A range of all arcs connected to state \a state in the automaton.

If Direction is forward, the arcs out_of \a state are returned.
If Direction is backward, the arcs into \a state are returned.
The labels on the arcs are in the compressed representation.

\param automaton The automaton.
\param direction The direction to take from \a state.
\param state The state of interest.

\pre <c>hasState (automaton, state)</c>.

\sa arcOn
*/
static auto constexpr arcsOnCompressed = callable::ArcsOnCompressed();

/**
\brief Call math::times, with the order of the two arguments depending on
\a direction.

This corresponds to the direction of traversal in automata: when traversing an
automaton in forward direction, the labels of an arc \c a and an arc \c b
that follows immediately are combined with <c>math::times (a, b)</c>.
When traversing the automaton in backward direction, the arguments must be
reversed.
This function does that.

\param direction
    The direction of traversal.
    Must be \c Forward or \c Backward.
\param left Left-hand operand.
\param right Right-hand operand.
*/
static auto constexpr times = callable::Times();

/**
Utilities to transform labels embedded in various types of structure.
*/
namespace transformation {

    /**
    \brief Function class that takes a range of pairs <c>(state, label)</c> and
    transforms them on the fly.

    The pairs can be of any range type.

    \todo In theory, this could transform the second element on the fly but
    return the first one straight away.
    This seems a pain to implement though, for possibly not so much gain.
    */
    class TransformLabelsForStates {
        // Closure that converts the second element using convertLabel.
        template <class ConvertLabel> struct ConvertStateAndLabel {
            ConvertLabel convertLabel;

            explicit ConvertStateAndLabel (ConvertLabel convertLabel)
            : convertLabel (convertLabel) {}

            // The pair coming in is not necessarily a pair.
            // It may have to be consumed carefully to make sure that this also
            // works on a range that allows traversal only once.
            template <class StateAndLabel_> struct result {
                typedef decltype (range::view_once (
                    std::declval <StateAndLabel_>())) StateAndLabel;
                typedef decltype (range::first (
                    std::declval <StateAndLabel const &>())) State;
                typedef decltype (range::first (range::drop (
                        std::declval <StateAndLabel &&>()))) Label;
                typedef decltype (std::declval <ConvertLabel const>() (
                    std::declval <Label>())) ConvertedLabel;

                typedef range::tuple <typename std::decay <State>::type,
                    typename std::decay <ConvertedLabel>::type> type;
            };

            template <class StateAndLabel>
                typename result <StateAndLabel>::type
                operator() (StateAndLabel && stateAndLabel) const
            {
                auto view = range::view_once (
                    std::forward <StateAndLabel> (stateAndLabel));
                auto state = range::first (view);
                auto label = convertLabel (range::first (range::drop (
                    std::move (view))));
                return typename result <StateAndLabel>::type (
                    std::move (state), std::move (label));
            }
        };

    public:
        template <class ConvertLabel, class Range>
            auto operator() (ConvertLabel convertLabel, Range && range) const
        RETURNS (range::transform (std::forward <Range> (range),
            ConvertStateAndLabel <ConvertLabel> (convertLabel)));
    };

    /**
    \brief An arc which is an underlying arc with the label transformed.
    */
    template <class Underlying, class ConvertLabel> class TransformedArc {
    private:
        Underlying underlying_;
        ConvertLabel convertLabel_;

        // GCC 4.6 needs this type to be wrapped in a template.
        template <class Direction> struct State {
            typedef decltype (std::declval <Underlying const &>().state (
                std::declval <Direction>())) type;
        };

    public:
        TransformedArc (Underlying && underlying, ConvertLabel convertLabel)
        : underlying_ (std::forward <Underlying> (underlying)),
            convertLabel_ (convertLabel) {}

        template <class Direction>
            typename State <Direction>::type
        state (Direction direction) const
        { return underlying_.state (direction); }

        auto label() const RETURNS (convertLabel_ (underlying_.label()));
    };

    /**
    \brief Function class that takes a range of arcs and returns a lazily range
    of the same arcs, with their labels transformed.
    */
    class TransformLabelsOnArcs {
        // Closure that converts the second element using convertLabel.
        template <class ConvertLabel> struct TransformArc {
            ConvertLabel convertLabel;

            explicit TransformArc (ConvertLabel convertLabel)
            : convertLabel (convertLabel) {}

            template <class Arc> auto operator() (Arc && arc) const
            RETURNS (TransformedArc <Arc, ConvertLabel> (
                std::forward <Arc> (arc), convertLabel));
        };

    public:
        template <class ConvertLabel, class Range>
            auto operator() (ConvertLabel convertLabel, Range && range) const
        RETURNS (range::transform (std::forward <Range> (range),
            TransformArc <ConvertLabel> (convertLabel)));
    };

} // namespace transformation

/* Default implementation of operations. */

namespace operation {

    /* TryAll.*/

    /**
    Derive from the first operation type, if is is implemented, otherwise the
    second one, if it is implemented, et cetera.
    If none are implemented, then derived from the last operation type.
    */
    template <class ... Operations> struct TryAll;

    template <class Operation> struct TryAll <Operation>
    : Operation {};

    template <class Operation, class ... Rest>
        struct TryAll <Operation, Rest ...>
    : boost::mpl::if_ <IsImplemented <Operation>,
        Operation, TryAll <Rest ...>>::type {};

    /**
    If \a Direction is a direction then derive from
    <c>TryAll \<Operations ...></c>; if not, derive from Unimplemented.
    */
    template <class Direction, class ... Operations> struct TryAllIfDirection
    : boost::mpl::if_ <IsDirection <Direction>,
        TryAll <Operations ...>, Unimplemented>::type {};

    /* EnableIfMember. */
    template <class AutomatonTag, class MemberResult>
        struct EnableIfMemberImplementation
    { typedef void type; };
    // Disable if the Automaton is not an automaton.
    template <class MemberResult>
        struct EnableIfMemberImplementation <NotAnAutomaton, MemberResult> {};

    /**
    Evaluate to void if Automaton is an automaton.
    MemberResult is passed as a trick to enable SFINAE at the site of
    instantiation.
    */
    template <class Automaton, class MemberResult> struct EnableIfMember
    : EnableIfMemberImplementation <
        typename AutomatonTag <Automaton>::type, MemberResult> {};

    /* Forwarding to member functions. */

    // States.
    // Call .descriptor() if it is available.
    template <class Automaton, class Enable = void> struct DescriptorMember
    : operation::Unimplemented {};

    template <class Automaton>
        struct DescriptorMember <Automaton, typename
            EnableIfMember <Automaton,
                decltype (std::declval <Automaton>().descriptor())>::type>
    {
        auto operator() (Automaton && automaton) const
        RETURNS (std::forward <Automaton> (automaton).descriptor());
    };

    // States.
    // Call .states() if it is available.
    template <class Automaton, class Enable = void> struct StatesMember
    : operation::Unimplemented {};

    template <class Automaton>
        struct StatesMember <Automaton, typename
            EnableIfMember <Automaton,
                decltype (std::declval <Automaton>().states())>::type>
    {
        auto operator() (Automaton && automaton) const
        RETURNS (std::forward <Automaton> (automaton).states());
    };

    // HasState.
    // Call .hasState() if it is available.
    template <class Automaton, class State, class Enable = void>
        struct HasStateMember
    : operation::Unimplemented {};

    template <class Automaton, class State>
        struct HasStateMember <Automaton, State, typename
            EnableIfMember <Automaton,
                decltype (std::declval <Automaton>().hasState(
                    std::declval <State>()))>::type>
    {
        auto operator() (Automaton && automaton, State && state) const
        RETURNS (std::forward <Automaton> (automaton).hasState (
            std::forward <State> (state)));
    };

    // TerminalStates.
    // Call .terminalStates() if it is available.
    template <class Automaton, class Direction, class Enable = void>
        struct TerminalStatesMember
    : operation::Unimplemented {};

    template <class Automaton, class Direction>
        struct TerminalStatesMember <Automaton, Direction, typename
            EnableIfMember <Automaton, decltype (std::declval <Automaton>()
                .terminalStates (std::declval <Direction>()))>::type>
    {
        auto operator() (
            Automaton && automaton, Direction const & direction) const
        RETURNS (
            std::forward <Automaton> (automaton).terminalStates (direction));
    };

    // TerminalStatesCompressed.
    // Call .terminalStatesCompressed() if it is available.
    template <class Automaton, class Direction, class Enable = void>
        struct TerminalStatesCompressedMember
    : operation::Unimplemented {};

    template <class Automaton, class Direction>
        struct TerminalStatesCompressedMember <Automaton, Direction, typename
            EnableIfMember <Automaton, decltype (std::declval <Automaton>()
                .terminalStatesCompressed (std::declval <Direction>()))>::type>
    {
        auto operator() (
            Automaton && automaton, Direction const & direction) const
        RETURNS (std::forward <Automaton> (automaton)
            .terminalStatesCompressed (direction));
    };

    // TerminalLabel.
    // Call .terminalLabel() if it is available.
    template <class Automaton, class Direction, class State,
            class Enable = void>
        struct TerminalLabelMember
    : operation::Unimplemented {};

    template <class Automaton, class Direction, class State>
        struct TerminalLabelMember <Automaton, Direction, State, typename
            EnableIfMember <Automaton,
                decltype (std::declval <Automaton>().terminalLabel(
                    std::declval <Direction>(), std::declval <State>()))>::type>
    {
        auto operator() (
            Automaton && automaton, Direction direction, State && state) const
        RETURNS (std::forward <Automaton> (automaton).terminalLabel (
            direction, std::forward <State> (state)));
    };

    // TerminalLabelCompressed.
    // Call .terminalLabelCompressed() if it is available.
    template <class Automaton, class Direction, class State,
            class Enable = void>
        struct TerminalLabelCompressedMember
    : operation::Unimplemented {};

    template <class Automaton, class Direction, class State>
        struct TerminalLabelCompressedMember <Automaton, Direction, State,
            typename EnableIfMember <Automaton,
                decltype (std::declval <Automaton>().terminalLabelCompressed(
                    std::declval <Direction>(), std::declval <State>()))>::type>
    {
        auto operator() (
            Automaton && automaton, Direction direction, State && state) const
        RETURNS (std::forward <Automaton> (automaton).terminalLabelCompressed (
            direction, std::forward <State> (state)));
    };

    // ArcsOn.
    // Call .arcsOn() if it is available.
    template <class Automaton, class Direction, class State,
        class Enable = void>
    struct ArcsOnMember
    : operation::Unimplemented {};

    template <class Automaton, class Direction, class State>
        struct ArcsOnMember <Automaton, Direction, State, typename
            EnableIfMember <Automaton, decltype (std::declval <Automaton>()
                .arcsOn (std::declval <Direction>(),
                    std::declval <State>()))>::type>
    {
        auto operator() (Automaton && automaton, Direction direction,
            State && state) const
        RETURNS (std::forward <Automaton> (automaton)
            .arcsOn (direction, std::forward <State> (state)));
    };

    // ArcsOnCompressed.
    // Call .arcsOnCompressed() if it is available.
    template <class Automaton, class Direction, class State,
        class Enable = void>
    struct ArcsOnCompressedMember
    : operation::Unimplemented {};

    template <class Automaton, class Direction, class State>
        struct ArcsOnCompressedMember <Automaton, Direction, State, typename
            EnableIfMember <Automaton, decltype (std::declval <Automaton>()
                .arcsOnCompressed (std::declval <Direction>(),
                    std::declval <State>()))>::type>
    {
        auto operator() (Automaton && automaton, Direction direction,
            State && state) const
        RETURNS (std::forward <Automaton> (automaton)
            .arcsOnCompressed (direction, std::forward <State> (state)));
    };

    /* Descriptor. */
    template <class AutomatonTag, class Automaton, class Enable /*= void*/>
        struct Descriptor
    : TryAll <DescriptorMember <Automaton>> {};

    /* States. */
    template <class AutomatonTag, class Automaton, class Enable /*= void*/>
        struct States
    : TryAll <StatesMember <Automaton>> {};

    /* HasState. */
    template <class AutomatonTag, class Automaton, class State,
        class Enable /*= void*/>
    struct HasState
    : TryAll <HasStateMember <Automaton, State>> {};

    /* TerminalStates. */
    /*
    Automatic implementation.
    Takes the output of TerminalStatesCompressed and converts the range of
    <c>(state, compressedLabel)</c> into a range of
    <c>(state, expandedLabel)</c> on the fly.
    */
    template <class Automaton, class Direction, class Enable = void>
        struct TerminalStatesAutomatic
    : Unimplemented {};

    template <class Automaton, class Direction>
        struct TerminalStatesAutomatic <Automaton, Direction, typename
            boost::enable_if <boost::mpl::and_<
                Has <callable::Descriptor (Automaton)>,
                Has <callable::TerminalStatesCompressed (Automaton, Direction)>>
            >::type>
    {
        auto operator() (Automaton && automaton, Direction direction) const
        RETURNS (transformation::TransformLabelsForStates() (
            ::flipsta::descriptor (automaton).expand(),
            ::flipsta::terminalStatesCompressed (
                std::forward <Automaton> (automaton), direction)));
    };

    template <class AutomatonTag, class Automaton, class Direction,
        class Enable /*= void*/>
    struct TerminalStates
    : TryAllIfDirection <Direction,
        TerminalStatesMember <Automaton, Direction>,
        TerminalStatesAutomatic <Automaton, Direction>> {};

    /* TerminalStatesCompressed. */
    template <class AutomatonTag, class Automaton, class Direction,
        class Enable /*= void*/>
    struct TerminalStatesCompressed
    : TryAllIfDirection <Direction,
        TerminalStatesCompressedMember <Automaton, Direction>> {};

    /* TerminalLabel. */
    /*
    Automatic implementation.
    Converts the output of TerminalLabelCompressed and to an expanded label.
    */
    template <class Automaton, class Direction, class State,
        class Enable = void>
    struct TerminalLabelAutomatic
    : Unimplemented {};

    template <class Automaton, class Direction, class State>
        struct TerminalLabelAutomatic <Automaton, Direction, State, typename
            boost::enable_if <boost::mpl::and_<
                Has <callable::Descriptor (Automaton)>,
                Has <callable::TerminalLabelCompressed (
                    Automaton, Direction, State)>>
            >::type>
    {
        auto operator() (
            Automaton && automaton, Direction direction, State && state) const
        RETURNS (::flipsta::descriptor (automaton).expand() (
            ::flipsta::terminalLabelCompressed (
                std::forward <Automaton> (automaton), direction,
                std::forward <State> (state))));
    };

    template <class AutomatonTag, class Automaton, class Direction, class State,
        class Enable /*= void*/>
    struct TerminalLabel
    : TryAllIfDirection <Direction,
        TerminalLabelMember <Automaton, Direction, State>,
        TerminalLabelAutomatic <Automaton, Direction, State>> {};

    /* TerminalLabelCompressed. */
    template <class AutomatonTag, class Automaton, class Direction, class State,
        class Enable /*= void*/>
    struct TerminalLabelCompressed
    : TryAllIfDirection <Direction,
        TerminalLabelCompressedMember <Automaton, Direction, State>> {};

    /* ArcsOn. */
    /*
    Automatic implementation.
    Takes the output of ArcsOnCompressed and converts the range of arcs with an
    compressed label into arcs with the corresponding expanded label.
    */
    template <class Automaton, class Direction, class State,
        class Enable = void>
    struct ArcsOnAutomatic
    : Unimplemented {};

    template <class Automaton, class Direction, class State>
        struct ArcsOnAutomatic <Automaton, Direction, State, typename
            boost::enable_if <boost::mpl::and_<
                Has <callable::Descriptor (Automaton)>,
                Has <callable::ArcsOnCompressed (Automaton, Direction, State)>>
            >::type>
    {
        auto operator() (Automaton && automaton,
            Direction direction, State && state) const
        RETURNS (transformation::TransformLabelsOnArcs() (
            ::flipsta::descriptor (automaton).expand(),
            ::flipsta::arcsOnCompressed (
                std::forward <Automaton> (automaton),
                direction, std::forward <State> (state))));
    };

    template <class AutomatonTag, class Automaton, class Direction, class State,
        class Enable /*= void*/>
    struct ArcsOn
    : TryAllIfDirection <Direction,
        ArcsOnMember <Automaton, Direction, State>,
        ArcsOnAutomatic <Automaton, Direction, State>> {};

    /* ArcsOnCompressed. */
    template <class AutomatonTag, class Automaton, class Direction, class State,
        class Enable /*= void*/>
    struct ArcsOnCompressed
    : TryAllIfDirection <Direction,
        ArcsOnCompressedMember <Automaton, Direction, State>> {};

} // namespace operation.

} // namespace flipsta

#endif // FLIPSTA_CORE_HPP_INCLUDED
