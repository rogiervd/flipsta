/*
Copyright 2015 Rogier van Dalen.

This file is part of the Flipsta library.

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#include "range/core.hpp"
#include "range/for_each_macro.hpp"

#include "range/tuple.hpp"

#include "math/magma.hpp"

#include "core.hpp"
#include "label.hpp"
#include "topological_order.hpp"

namespace flipsta {

// The lazy range of compressed labels that will be returned.
template <class Automaton, class Direction>
    class ShortestDistanceAcyclicRange;

namespace callable {

    struct ShortestDistanceAcyclic;
    struct ShortestDistanceAcyclicFrom;
    struct ShortestDistanceAcyclicCompressed;
    struct ShortestDistanceAcyclicFromCompressed;

    namespace shortest_distance_detail {

        template <class Automaton, class Direction>
            struct CanBeImplemented
        : math::is::semiring <typename MathDirection <
                typename Opposite <Direction>::type>::type,
            math::callable::times, math::callable::plus,
            typename LabelType <Automaton>::type> {};

        template <class Automaton, class Direction>
            struct AcyclicShortestDistanceResult
        {
            typedef decltype (tag (std::declval <Automaton>()).expand()) Expand;
            typedef typename std::result_of <
                transformation::TransformLabelsForStates (
                    Expand, ShortestDistanceAcyclicRange <Automaton, Direction>)
                >::type type;
        };

        template <class Automaton, class Direction, class Enable = void>
            struct ShortestDistanceAcyclic
        : operation::Unimplemented {};
        template <class Automaton, class Direction, class Enable = void>
            struct ShortestDistanceAcyclicFrom
        : operation::Unimplemented {};
        template <class Automaton, class Direction, class Enable = void>
            struct ShortestDistanceAcyclicCompressed
        : operation::Unimplemented {};
        template <class Automaton, class Direction, class Enable = void>
            struct ShortestDistanceAcyclicFromCompressed
        : operation::Unimplemented {};

        // Compressed versions.
        template <class Automaton, class Direction>
            struct ShortestDistanceAcyclicCompressed <Automaton, Direction,
                typename boost::enable_if <
                    CanBeImplemented <Automaton, Direction>>::type>
        {
            template <class InitialStates>
                ShortestDistanceAcyclicRange <Automaton, Direction>
                    operator() (Automaton && automaton,
                        InitialStates && initialStates,
                        Direction const & direction) const
            {
                static_assert (range::is_range <InitialStates>::value,
                    "InitialStates must be a range of (state, label).");

                return ShortestDistanceAcyclicRange <Automaton, Direction> (
                    std::forward <Automaton> (automaton),
                    std::forward <InitialStates> (initialStates));
            }
        };

        template <class Automaton, class Direction>
            struct ShortestDistanceAcyclicFromCompressed <Automaton, Direction,
                typename boost::enable_if <
                    CanBeImplemented <Automaton, Direction>>::type>
        {
            ShortestDistanceAcyclicRange <Automaton, Direction>
                operator() (Automaton && automaton,
                    typename StateType <Automaton>::type const & state,
                    Direction const & direction)
                const
            {
                auto one = math::one <
                    typename CompressedLabelType <Automaton>::type>();
                return ShortestDistanceAcyclicRange <Automaton, Direction> (
                    std::forward <Automaton> (automaton),
                    range::make_tuple (range::make_tuple (state, one)));
            }
        };

        // Expanded versions.
        template <class Automaton, class Direction>
            struct ShortestDistanceAcyclic <Automaton, Direction,
                typename boost::enable_if <
                    CanBeImplemented <Automaton, Direction>>::type>
        {
            template <class InitialStates> typename
                AcyclicShortestDistanceResult <Automaton, Direction>::type
                    operator() (Automaton && automaton,
                        InitialStates && initialStates,
                        Direction const & direction) const
            {
                static_assert (range::is_range <InitialStates>::value,
                    "InitialStates must be a range of (state, label).");

                // Make a compressed version of the initial states.
                auto compress = flipsta::tag (automaton).compress();
                auto compressedInitialStates =
                    transformation::TransformLabelsForStates() (
                        compress, std::forward <InitialStates> (initialStates));

                // Return the expanded version of the compressed states.
                // Use the tag before moving the automaton.
                auto expand = flipsta::tag (automaton).expand();
                ShortestDistanceAcyclicCompressed <Automaton, Direction>
                    implementation;
                return transformation::TransformLabelsForStates() (expand,
                    implementation (std::forward <Automaton> (automaton),
                        std::move (compressedInitialStates), direction));
            }
        };

        template <class Automaton, class Direction>
            struct ShortestDistanceAcyclicFrom <Automaton, Direction,
                typename boost::enable_if <
                    CanBeImplemented <Automaton, Direction>>::type>
        {
            typename AcyclicShortestDistanceResult <Automaton, Direction>::type
                operator() (Automaton && automaton,
                    typename StateType <Automaton>::type const & state,
                    Direction const & direction)
                const
            {
                // Use the tag before moving the automaton.
                auto expand = flipsta::tag (automaton).expand();
                ShortestDistanceAcyclicFromCompressed <Automaton, Direction>
                    implementation;
                return transformation::TransformLabelsForStates() (expand,
                    implementation (std::forward <Automaton> (automaton),
                        state, direction));
            }
        };

        /// Callable that adheres to the nested callable protocol.
        template <template <class, class, class> class Apply> struct Callable {
            template <class ...> struct apply : operation::Unimplemented {};

            template <class Automaton, class Initial, class Direction>
                struct apply <Automaton, Initial, Direction>
            : Apply <Automaton, typename std::decay <Direction>::type, void> {};

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
    The automaton to traverse.
    If this is a temporary, a copy will be kept in the returned range.
    This could incur a copy, which might be expensive.
    If this is a reference, this reference will be kept in the returned range.
    It is the responsibility of the caller to take care that the automaton does
    not go out of scope before the range does.
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
template <class Automaton, class Direction> class ShortestDistanceAcyclicRange {
private:
    typedef typename std::decay <decltype (range::view (topologicalOrder (
            std::declval <Automaton>(), std::declval <Direction>())))>::type
        Order;

    static_assert (range::is_homogeneous <Order>::value,
        "Only implemented for homogeneous topologicalOrder.");

    typedef typename StateType <Automaton>::type State;

    typedef typename label::GeneraliseSemiring <
        typename std::decay <Automaton>::type::CompressedLabel>::type Label;

    Automaton automaton;
    Order order;
    // denseCover is set to false, because we will remove distances as soon as
    // we can.
    Map <State, Label, true, false> distances;

public:
    /**
    Initialise.
    \param automaton
        The automaton to compute the shortest distances over.
    \param initialStates
        Initial weights for states.
        This is used only once, during construction.
    */
    template <class InitialStates> ShortestDistanceAcyclicRange (
        Automaton && automaton, InitialStates && initialStates)
    : automaton (std::forward <Automaton> (automaton)),
        order (topologicalOrder (this->automaton, Direction())),
        distances (math::zero <Label>(),
            std::forward <InitialStates> (initialStates)) {}

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
        RANGE_FOR_EACH (arc, arcsOnCompressed (automaton, Direction(), state)) {
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
    template <class Automaton, class Direction>
        struct tag_of_qualified <
            flipsta::ShortestDistanceAcyclicRange <Automaton, Direction>>
    { typedef flipsta::ShortestDistanceAcyclicRangeTag type; };

} // namespace range

#endif // FLIPSTA_SHORTEST_DISTANCE_HPP_INCLUDED
