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

#ifndef FLIPSTA_TOPOLOGICAL_ORDER_HPP_INCLUDED
#define FLIPSTA_TOPOLOGICAL_ORDER_HPP_INCLUDED

#include <deque>

#include "range/std/container.hpp"
#include "range/view_shared.hpp"

#include "core.hpp"
#include "error.hpp"
#include "traverse.hpp"

#include "range/for_each_macro.hpp"

namespace flipsta {

/*
This file is in three parts:
1.  The connection between the interface and the implementation.
2.  The interface.
3.  The default implementation.
    (Automata can provide a specialised implementation.)
*/

/* Part 1: The connection between the interface and the implementation. */

namespace operation {

    // This is declared in core.hpp but repeated here.
    template <class AutomatonTag, class Automaton, class Direction,
        class Enable /*= void*/>
    struct TopologicalOrder;

} // namespace operation

/* Part 2: The interface.*/

namespace callable {

    struct TopologicalOrder {
        template <class ...> struct apply;

        template <class Automaton, class Direction>
            struct apply <Automaton, Direction>
        : operation::TopologicalOrder <typename AutomatonTag <Automaton>::type,
            Automaton, typename std::decay <Direction>::type> {};

        template <class Automaton, class Direction>
            auto operator() (Automaton && automaton,
                Direction const & direction) const
        RETURNS (apply <Automaton, Direction>() (
            std::forward <Automaton> (automaton), direction));
    };

} // namespace callable

/** \brief
Return the states of the automaton in topological order.

This may be computed lazily.
The automaton must remain in memory and unaltered while the range that is
returned is being used.

The standard implementation holds a container of arcs.
It is implemented iff \ref traverse is implemented for the automaton and the
direction.
It takes linear time in the number of transitions and linear space in the number
of states to build this.

A specialised implementation can be provided for some automata.
An example is automata based on products of automata, for which this can be a
lazy list which takes up less space.

\param automaton The automaton to find the topological order of.
\param direction The direction to use, of type Forward or Backward.

\throw AutomatonNotAcyclic
    Iff the automaton is not acyclic (so that topological order is not defined).
    In the default implementation, the exception will have
    <c>errorInfoState \<State></c> attached to it with the state that was
    detected to have a path to itself.
    The exception may be thrown during traversal.

\todo The automatic implementation currently always claims to be implemented.
    This may not be true, if "traverse" cannot be implemented (e.g. for a
    direction).
    This may have to be indicated.
*/
static auto constexpr topologicalOrder = callable::TopologicalOrder();

/* Part 3: The default implementation. */

namespace operation {

    // TopologicalOrder.
    // Call .topologicalOrder() if it is available.
    template <class Automaton, class Direction, class Enable = void>
        struct TopologicalOrderMember
    : operation::Unimplemented {};

    template <class Automaton, class Direction>
        struct TopologicalOrderMember <Automaton, Direction, typename
            EnableIfMember <Automaton, decltype (std::declval <Automaton>()
                .topologicalOrder (std::declval <Direction>()))>::type>
    {
        auto operator() (
            Automaton && automaton, Direction const & direction) const
        RETURNS (
            std::forward <Automaton> (automaton).topologicalOrder (direction));
    };

    /* TopologicalOrder. */
    /*
    Automatic implementation.
    */
    template <class Automaton, class Direction, class Enable = void>
        struct TopologicalOrderAutomatic
    : Unimplemented {};

    template <class Automaton, class Direction>
        struct TopologicalOrderAutomatic <Automaton, Direction>
    {
        range::view_of_shared <
            std::deque <typename StateType <Automaton>::type>>
        operator() (Automaton && automaton, Direction direction) const
        {
            typedef typename StateType <Automaton>::type State;
            std::deque <State> result;
            RANGE_FOR_EACH (report, traverse (automaton, direction)) {
                if (report.event == TraversalEvent::finishVisit)
                    result.push_front (report.state);
                else {
                    if (report.event == TraversalEvent::backState)
                        throw AutomatonNotAcyclic()
                            << errorInfoState <State> (report.state);
                }
                // Other reports from the depth-first traversal are ignored.
            }
            return range::view_shared (std::move (result));
        }
    };

    template <class AutomatonTag, class Automaton, class Direction,
        class Enable /*= void*/>
    struct TopologicalOrder
    : TryAllIfDirection <Direction,
        TopologicalOrderMember <Automaton, Direction>,
        TopologicalOrderAutomatic <Automaton, Direction>> {};

} // namespace operation

} // namespace flipsta

#endif // FLIPSTA_TOPOLOGICAL_ORDER_HPP_INCLUDED
