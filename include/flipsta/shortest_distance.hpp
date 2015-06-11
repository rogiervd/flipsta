/*
Copyright 2015 Rogier van Dalen.

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
Shortest-distance algorithms.
\todo Split this up in shortest_distance/... .hpp?
*/

#ifndef FLIPSTA_SHORTEST_DISTANCE_HPP_INCLUDED
#define FLIPSTA_SHORTEST_DISTANCE_HPP_INCLUDED

#include <type_traits>
#include <utility>

#include <boost/utility/enable_if.hpp>

#include "utility/pointee.hpp"
#include "utility/unique_ptr.hpp"

#include "range/core.hpp"
#include "range/for_each_macro.hpp"
#include "range/transform.hpp"
#include "range/tuple.hpp"

#include "math/magma.hpp"

#include "core.hpp"
#include "label.hpp"
#include "topological_order.hpp"

namespace flipsta {

// The lazy range of compressed labels that will be returned.
template <class AutomatonPtr, class Direction>
    class ShortestDistanceAcyclicRange;

namespace callable {

    struct ShortestDistanceAcyclic;
    struct ShortestDistanceAcyclicFrom;
    struct ShortestDistanceAcyclicCompressed;
    struct ShortestDistanceAcyclicFromCompressed;

    namespace shortest_distance_detail {

        template <class AutomatonPtr, class Direction>
            struct CanBeImplemented
        : math::is::semiring <typename MathDirection <
                typename Opposite <Direction>::type>::type,
            math::callable::times, math::callable::plus,
            typename PtrLabelType <AutomatonPtr>::type> {};

        template <class AutomatonPtr, class Direction>
            struct AcyclicShortestDistanceResult
        {
            typedef decltype (
                descriptor (*std::declval <AutomatonPtr>()).expand()) Expand;
            typedef typename std::result_of <
                transformation::TransformLabelsForStates (
                    Expand, ShortestDistanceAcyclicRange <
                        typename std::decay <AutomatonPtr>::type, Direction>)
                >::type type;
        };

        template <class AutomatonPtr, class Direction, class Enable = void>
            struct ShortestDistanceAcyclic
        : operation::Unimplemented {};
        template <class AutomatonPtr, class Direction, class Enable = void>
            struct ShortestDistanceAcyclicFrom
        : operation::Unimplemented {};
        template <class AutomatonPtr, class Direction, class Enable = void>
            struct ShortestDistanceAcyclicCompressed
        : operation::Unimplemented {};
        template <class AutomatonPtr, class Direction, class Enable = void>
            struct ShortestDistanceAcyclicFromCompressed
        : operation::Unimplemented {};

        // Compressed versions.
        template <class AutomatonPtr, class Direction>
            struct ShortestDistanceAcyclicCompressed <AutomatonPtr, Direction,
                typename boost::enable_if <
                    CanBeImplemented <AutomatonPtr, Direction>>::type>
        {
            template <class InitialStates>
                ShortestDistanceAcyclicRange <
                        typename std::decay <AutomatonPtr>::type, Direction>
                    operator() (AutomatonPtr && automaton,
                        InitialStates && initialStates,
                        Direction const & direction) const
            {
                static_assert (range::is_range <InitialStates>::value,
                    "InitialStates must be a range of (state, label).");

                return ShortestDistanceAcyclicRange <
                        typename std::decay <AutomatonPtr>::type, Direction> (
                    std::forward <AutomatonPtr> (automaton),
                    std::forward <InitialStates> (initialStates));
            }
        };

        template <class AutomatonPtr, class Direction>
            struct ShortestDistanceAcyclicFromCompressed <
                AutomatonPtr, Direction, typename boost::enable_if <
                    CanBeImplemented <AutomatonPtr, Direction>>::type>
        {
            ShortestDistanceAcyclicRange <
                    typename std::decay <AutomatonPtr>::type, Direction>
                operator() (AutomatonPtr && automaton,
                    typename PtrStateType <AutomatonPtr>::type const & state,
                    Direction const & direction)
                const
            {
                auto one = math::one <
                    typename PtrCompressedLabelType <AutomatonPtr>::type>();
                return ShortestDistanceAcyclicRange <
                    typename std::decay <AutomatonPtr>::type, Direction> (
                    std::forward <AutomatonPtr> (automaton),
                    range::make_tuple (range::make_tuple (state, one)));
            }
        };

        // Expanded versions.
        template <class AutomatonPtr, class Direction>
            struct ShortestDistanceAcyclic <AutomatonPtr, Direction,
                typename boost::enable_if <
                    CanBeImplemented <AutomatonPtr, Direction>>::type>
        {
            template <class InitialStates> typename
                AcyclicShortestDistanceResult <AutomatonPtr, Direction>::type
                    operator() (AutomatonPtr && automaton,
                        InitialStates && initialStates,
                        Direction const & direction) const
            {
                static_assert (range::is_range <InitialStates>::value,
                    "InitialStates must be a range of (state, label).");

                // Make a compressed version of the initial states.
                auto compress = flipsta::descriptor (*automaton).compress();
                auto compressedInitialStates =
                    transformation::TransformLabelsForStates() (
                        compress, std::forward <InitialStates> (initialStates));

                // Return the expanded version of the compressed states.
                // Use the descriptor before moving the automaton.
                auto expand = flipsta::descriptor (*automaton).expand();
                ShortestDistanceAcyclicCompressed <AutomatonPtr, Direction>
                    implementation;
                return transformation::TransformLabelsForStates() (expand,
                    implementation (std::forward <AutomatonPtr> (automaton),
                        std::move (compressedInitialStates), direction));
            }
        };

        template <class AutomatonPtr, class Direction>
            struct ShortestDistanceAcyclicFrom <AutomatonPtr, Direction,
                typename boost::enable_if <
                    CanBeImplemented <AutomatonPtr, Direction>>::type>
        {
            typename AcyclicShortestDistanceResult <
                    AutomatonPtr, Direction>::type
                operator() (AutomatonPtr && automaton,
                    typename PtrStateType <AutomatonPtr>::type const & state,
                    Direction const & direction)
                const
            {
                // Use the descriptor before moving the automaton pointer.
                auto expand = flipsta::descriptor (*automaton).expand();
                ShortestDistanceAcyclicFromCompressed <AutomatonPtr, Direction>
                    implementation;
                return transformation::TransformLabelsForStates() (expand,
                    implementation (std::forward <AutomatonPtr> (automaton),
                        state, direction));
            }
        };

        /// Callable that adheres to the nested callable protocol.
        template <template <class, class, class> class Apply> struct Callable {
            template <class ...> struct apply : operation::Unimplemented {};

            template <class AutomatonPtr, class Initial, class Direction>
                struct apply <AutomatonPtr, Initial, Direction>
            : Apply <AutomatonPtr, typename std::decay <Direction>::type, void>
            {};

            template <class ... Arguments>
                auto operator() (Arguments && ... arguments) const
            RETURNS (apply <Arguments ...>() (
                std::forward <Arguments> (arguments) ...));
        };

    } // namespace shortest_distance_detail

    struct ShortestDistanceAcyclic
    : shortest_distance_detail::Callable <
        shortest_distance_detail::ShortestDistanceAcyclic> {};

    struct ShortestDistanceAcyclicFrom
    : shortest_distance_detail::Callable <
        shortest_distance_detail::ShortestDistanceAcyclicFrom> {};

    struct ShortestDistanceAcyclicCompressed
    : shortest_distance_detail::Callable <
        shortest_distance_detail::ShortestDistanceAcyclicCompressed> {};

    struct ShortestDistanceAcyclicFromCompressed
    : shortest_distance_detail::Callable <
        shortest_distance_detail::ShortestDistanceAcyclicFromCompressed> {};

} // namespace callable

static auto constexpr shortestDistanceAcyclicCompressed =
    callable::ShortestDistanceAcyclicCompressed();
static auto constexpr shortestDistanceAcyclicFromCompressed =
    callable::ShortestDistanceAcyclicFromCompressed();

/** \brief
Compute the shortest distance from source states to every other state in an
acyclic automaton.

The result type is a lazy range with pairs <c>(state, label)</c>.

The automaton must remain unchanged while the resulting range is being used.

This is only available in the forward direction if the label is a right
semiring, and only available in the backward direction if the label is a left
semiring.

The number of elements in the returned range, and the total time complexity, is
\f$\Theta(n)\f$ where n is the number of states in the automaton.

Space use depends on the properties of the automaton.
Apart from the source states, weights are kept in memory only for states that
have an arc from a state that has been emitted are kept in memory.
As a state and its weight are emitted, it goes out of memory.
However, traversal of the states uses topological order, which by default uses
\f$\Theta(n)\f$ space.

\param automaton
    Pointer to the automaton to traverse.
    A copy of the pointer will be kept, and destructed when the range is
    destructed.
    The pointer must be copyable, and therefore cannot be a unique_ptr.
    The automaton should not change while the range is used.

    The automaton must have \c states() and \c arcsOnCompressed.

\param initialStates
    Range of pairs of (state, distance) giving the compressed initial labels to
    assign to states.

\param direction
    The direction in which to traverse the automaton.
*/
static auto constexpr shortestDistanceAcyclic =
    callable::ShortestDistanceAcyclic();

/** \brief
Compute the shortest distance from a single source state to every other state
in an acyclic automaton.

\sa shortestDistanceAcyclic

\param automaton
    The automaton to traverse.

\param initialState
    The one initial state that will be assigned label math::one().

\param direction
    The direction in which to traverse the automaton.
*/
static auto constexpr shortestDistanceAcyclicFrom =
    callable::ShortestDistanceAcyclicFrom();


/** \brief
A lazy list of states and the shortest distances to them.

This tries to save memory by only keeping track of states one arc ahead.

The automaton is traversed in topological order.
For each state, the arcs going out of it are "relaxed", that is, the
intermediate shortest distances to the destinations are updated.
After that, the state can be forgotten.
*/
template <class AutomatonPtr, class Direction>
    class ShortestDistanceAcyclicRange
{
private:
    static_assert (std::is_same <AutomatonPtr,
        typename std::decay <AutomatonPtr>::type>::value,
        "AutomatonPtr must be unqualified.");

    static_assert (!utility::is_unique_ptr <AutomatonPtr>::value,
        "Sorry, the pointer to the automaton must be copyable, and "
        "therefore cannot be a unique_ptr. "
        "It needs to be shared internally. "
        "You may want to use shared_ptr instead.");
    static_assert (
        std::is_constructible <AutomatonPtr, AutomatonPtr const &>::value,
        "Sorry, the pointer to the automaton must be copyable. "
        "It needs to be shared internally. "
        "You may want to use, say, std::shared_ptr.");

    typedef typename std::decay <decltype (topologicalOrder (
            std::declval <AutomatonPtr>(), std::declval <Direction>()))>::type
        Order;

    static_assert (range::is_homogeneous <Order>::value,
        "Only implemented for homogeneous topologicalOrder.");

    typedef typename utility::pointee <AutomatonPtr>::type Automaton;

    typedef typename StateType <Automaton>::type State;

    typedef typename label::GeneraliseSemiring <
        typename std::decay <Automaton>::type::CompressedLabel>::type Label;

    AutomatonPtr automaton;
    Order order;
    // denseCover is set to false, because we will remove distances as soon as
    // we can.
    Map <State, Label, true, false> distances;

    /**
    Functor that returns any pair (state, weight) as-is, but throws if the
    automaton does not contain the state.
    */
    class PassThroughIfStateExists {
        AutomatonPtr automaton_;
    public:
        PassThroughIfStateExists (AutomatonPtr const & automaton)
        : automaton_ (automaton) {}

        template <class StateAndWeight> StateAndWeight operator() (
            StateAndWeight && state_and_weight) const
        {
            auto && state = range::first (state_and_weight);
            if (!automaton_->hasState (state))
                throw StateNotFound() << errorInfoState <State> (state);
            return std::forward <StateAndWeight> (state_and_weight);
        }
    };

public:
    /**
    Initialise.
    \param automaton
        The automaton to compute the shortest distances over.
    \param initialStates
        Initial weights for states.
        This is used only once, during construction.

    \throw AutomatonNotFound
        iff any state in \a initialStates is not in the automaton.
    */
    template <class InitialStates> ShortestDistanceAcyclicRange (
            AutomatonPtr const & automaton, InitialStates && initialStates)
    : automaton (automaton),
        order (topologicalOrder (automaton, Direction())),
        distances (math::zero <Label>(),
            range::transform (PassThroughIfStateExists (automaton),
                std::forward <InitialStates> (initialStates))) {}

    bool empty (::direction::front) const
    { return range::empty (order); }

    /** \brief
    Return the next state and the shortest distance to it.
    */
    std::pair <State, Label> chop_in_place (::direction::front) {
        // Save state and distance temporarily.
        State state = range::chop_in_place (order);
        Label stateDistance = distances [state];
        // After relaxing all arcs out of this state, we do not need the
        // distance to this state any more, so remove it to save memory.
        distances.remove (state);
        RANGE_FOR_EACH (arc, arcsOnCompressed (*automaton, Direction(), state))
        {
            // Relax this arc.
            State next = arc.state (Direction());
            auto newLabel = times (Direction(), stateDistance, arc.label());
            distances.set (next, distances [next] + newLabel);
        }
        return std::make_pair (state, std::move (stateDistance));
    }
};

struct ShortestDistanceAcyclicRangeTag;

} // namespace flipsta

namespace range {

    // Mark ShortestDistanceAcyclicRange as a range.
    template <class AutomatonPtr, class Direction>
        struct tag_of_qualified <
            flipsta::ShortestDistanceAcyclicRange <AutomatonPtr, Direction>>
    { typedef flipsta::ShortestDistanceAcyclicRangeTag type; };

} // namespace range

#endif // FLIPSTA_SHORTEST_DISTANCE_HPP_INCLUDED
