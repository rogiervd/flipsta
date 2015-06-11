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
    template <class AutomatonTag, class AutomatonPtr, class Direction,
        class Enable /*= void*/>
    struct TopologicalOrder;

} // namespace operation

/* Part 2: The interface.*/

namespace callable {

    struct TopologicalOrder {
        template <class ...> struct apply;

        template <class AutomatonPtr, class Direction>
            struct apply <AutomatonPtr, Direction>
        : operation::TopologicalOrder <typename
            PtrAutomatonTag <AutomatonPtr>::type,
            AutomatonPtr, typename std::decay <Direction>::type> {};

        template <class AutomatonPtr, class Direction>
            auto operator() (AutomatonPtr && automaton,
                Direction const & direction) const
        RETURNS (apply <AutomatonPtr, Direction>() (
            std::forward <AutomatonPtr> (automaton), direction));
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

\param automaton
    Pointer to the automaton to find the topological order of.
    If the order is computed lazily, then a copy of this pointer may be kept
    until the result type is destructed.
\param direction
    The direction to use, of type Forward or Backward.

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
    template <class AutomatonPtr, class Direction, class Enable = void>
        struct TopologicalOrderMember
    : operation::Unimplemented {};

    template <class AutomatonPtr, class Direction>
        struct TopologicalOrderMember <AutomatonPtr, Direction, typename
            EnableIfMember <AutomatonPtr, decltype (
                std::declval <AutomatonPtr>()->topologicalOrder (
                    std::declval <Direction>()))>::type>
    {
        auto operator() (
            AutomatonPtr && automaton, Direction const & direction) const
        RETURNS (std::forward <AutomatonPtr> (automaton)
            ->topologicalOrder (direction));
    };

    /* TopologicalOrder. */
    /*
    Automatic implementation.
    */
    template <class AutomatonPtr, class Direction, class Enable = void>
        struct TopologicalOrderAutomatic
    : Unimplemented {};

    template <class AutomatonPtr, class Direction>
        struct TopologicalOrderAutomatic <AutomatonPtr, Direction>
    {
        typedef typename PtrStateType <AutomatonPtr>::type State;

        range::view_of_shared <std::deque <State>>
            operator() (AutomatonPtr && automaton, Direction direction)
            const
        {
            std::deque <State> result;
            RANGE_FOR_EACH (report, traverse (
                std::forward <AutomatonPtr> (automaton), direction))
            {
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

    template <class AutomatonTag, class AutomatonPtr, class Direction,
        class Enable /*= void*/>
    struct TopologicalOrder
    : TryAllIfDirection <Direction,
        TopologicalOrderMember <AutomatonPtr, Direction>,
        TopologicalOrderAutomatic <AutomatonPtr, Direction>> {};

} // namespace operation

} // namespace flipsta

#endif // FLIPSTA_TOPOLOGICAL_ORDER_HPP_INCLUDED
