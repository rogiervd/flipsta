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

#ifndef FLIPSTA_TRAVERSE_HPP_INCLUDED
#define FLIPSTA_TRAVERSE_HPP_INCLUDED

#include <type_traits>
#include <cassert>

#include "utility/storage.hpp"

#include "range/core.hpp"
#include "range/for_each_macro.hpp"

#include "core.hpp"

#include "map.hpp"
#include "queue.hpp"

namespace flipsta {

template <class Automaton, class Direction> class DepthFirstTraversalRange;

/**
\brief Traverse the automaton and return (lazily) a range of states marked with
their meaning.

The automaton must remain unchanged while the resulting range is being used.
The range is non-copyable but it is moveable.

This uses "depth-first search" (for those in the know) to find a spanning
forest, and keeps going until the each state have been tried as roots.
(Either the state has been seen before, or it is the root of a new tree.)
The elements of the resulting range are of type <c>TraversedState \<State></c>.
This contains a state, and an enum indicating what stage of traversal the state
was involved in.
Usually you will want to filter only elements with a limited sets of indicator.

If you do not know how depth-first search works, here is what you need to know
to use this function.
\li \c newRoot is emitted once for each root of a tree.
    This will happen at least once, and it may happen for each state.
    If the states are in a topological order, then this will happen once.
    The next event emitted will be the same state, with \c visit.
\li \c visit is emitted exactly once for each state, when it is first
    discovered.
\li \c finishVisit is emitted exactly once for each state, when the visit to it
    finishes.
    Since states are visited recursively, the order of states with
    \c finishVisit will be different from the order of those with \c visit.
    The order of the states with \c finishVisit is reverse topological order.
\li \c backState is emitted when a state is rediscovered that is being visited.
    If this occurs at all, the automaton has at least as many cycles as this
    occurs.
    If this does not occur, the automaton is acyclic.
\li \c forwardOrCrossState is emitted when a state is rediscovered that has been
    visited.
    It may be part of the same tree or in a different one.
    (More detailed classification is possible, but would take time and is not
    that useful.)

The number of elements in the returned range, and the total time complexity, is
\f$\Theta(n)\f$ where n is the number of states in the automaton.
While the returned range is being consumed, space use also rises to
\f$\Theta(n)\f$.

\param automaton
    The automaton to traverse.
    If this is a temporary, a copy will be kept in the returned range.
    This could incur a copy, which might be expensive.
    If this is a reference, this reference will be kept in the returned range.
    It is the responsibility of the caller to take care that the automaton does
    not go out of scope before the range does.

    The automaton must have \c states() and \c arcsOnCompressed.
\param direction
    The direction in which to traverse the automaton.
*/
template <class Automaton, class Direction> inline
    auto traverse (Automaton && automaton, Direction direction)
RETURNS (DepthFirstTraversalRange <Automaton, Direction> (
    std::forward <Automaton> (automaton)));

/**
Indicate the meaning of the state during depth-first traversal.
*/
enum class TraversalEvent {
    /// Indicates the state will be the root of a tree.
    newRoot,
    /// Indicates that a newly discovered state is now being visited.
    visit,
    /// Indicates that the visit to the state has finished.
    finishVisit,
    /// Indicates that the state was rediscovered while it was being
    /// visited.
    backState,
    /// Indicates that the state was rediscovered while it was not being
    /// visited any more.
    forwardOrCrossState
};

/** \brief
Event generated while traversing an automaton using depth-first search.
*/
template <class State> struct TraversedState {
    /// The state that this event concerns.
    State state;
    /// The type of event.
    TraversalEvent event;

    TraversedState (State const & state, TraversalEvent event)
    : state (state), event (event) {}
};

struct DepthFirstTraversalRangeTag;

/** \brief
Lazy range that contains elements of type <c>TraversedState \<State></c> that
arise while traversing the automaton in a depth-first search.

This is non-copyable but it is moveable.
*/
template <class Automaton, class Direction> class DepthFirstTraversalRange {
public:
    typedef typename std::decay <Automaton>::type::State State;
    typedef TraversedState <State> Report;

private:
    typedef typename DecayedResultOf <
        range::callable::view (callable::States (Automaton))>::type Roots;

    typedef typename DecayedResultOf <
            range::callable::view (
                callable::ArcsOnCompressed (Automaton, Direction, State))
        >::type Arcs;

    /**
    Indicate whether a state is not yet been visited, being visited, or has
    been visited.
    */
    enum VisitStatus { unvisited, visiting, visited };

    /**
    Position in the recursion down the tree.
    Since we do not actually use recursion, the position (state being visited
    and arc being followed) must be tracked explicitly.
    */
    struct Position {
        State state;
        bool visiting;
        Arcs arcs;

        Position (Automaton const & automaton, State const & state)
        : state (state), visiting (false),
            arcs (arcsOnCompressed (automaton, Direction(), state)) {}
    };

    /*
    This is what this class would do if there were such a thing as a "yield"
    statement (defined here as working magically).

    void generateFrom (State const & state) {
        visitStatus.set (state, visiting);
        yield Report (state, TraversalEvent::visit);

        RANGE_FOR_EACH (arc, arcsOnCompressed (automaton, Direction(), state)) {
            State next = arc.state (Direction());
            if (visitStatus [next] == unvisited)
                generateFrom (next);
            else if (visitStatus [next] == visiting)
                yield Report (state, TraversalEvent::backState);
            else {
                assert (visitStatus [next] == visited);
                yield Report (state, TraversalEvent::forwardOrCrossState);
            }
        }
        yield Report (state, TraversalEvent::finishVisit);
        visitStatus.set (state, visited);
    }

    void generate (Automaton && automaton) {
        auto roots = range::view (states (this->automaton));
        while (!range::empty (roots)) {
            auto root = range::chop_in_place (roots);
            if (visitStatus [root] != visited) {
                yield Report (root, TraversalEvent::newRoot);
                generatorFrom (root);
            }
        }
    }
    */

    // If Automaton is a reference type, then store it as an assignable type.
    typename utility::storage::store <Automaton>::type automaton_;
    Automaton && automaton() const { return automaton_; }

    /**
    A view on the roots of the trees still left to traverse.
    If \a queue is empty, then this must either be empty, or the first state
    must be one that has not been visited yet.
    This way, "empty()" can be implemented in constant time.

    If \a queue is non-empty, a tree is being visited.
    The first element of this may form part of that tree, so at such time it is
    not required that the first state of this has not been visited yet.

    This is the same "roots" variable as in the code of "generate" above.
    */
    Roots roots;

    /**
    Keep track of which states have been visited and which ones have not, and
    which ones are being visited.
    The last possibility is of vital importance for cycle detection.
    Spanning trees are visited recursively, so if a state is being visited and
    it is found again, then the graph is cyclic.
    */
    Map <State, VisitStatus, true, true> visitStatus;

    /**
    Keep track of which states are being visited and which arcs are being
    followed.
    In a recursive implementation, this would go on the stack, but here we keep
    it explicitly in a LIFO queue.
    This is the representation of the call stack in the code of "generateFrom"
    above.
    */
    LifoQueue <Position> queue;

    void assertInvariants() const {
        assert (!queue.empty() || range::empty (roots) ||
            visitStatus [range::first (roots)] == unvisited);
    }

public:
    DepthFirstTraversalRange (Automaton && automaton)
    : automaton_ (std::forward <Automaton> (automaton)),
        roots (range::view (states (this->automaton()))),
        visitStatus (unvisited), queue()
    { assertInvariants(); }

    DepthFirstTraversalRange (DepthFirstTraversalRange const &) = delete;
    DepthFirstTraversalRange & operator= (DepthFirstTraversalRange const &)
        = delete;

    DepthFirstTraversalRange (DepthFirstTraversalRange && that)
    : automaton_ (std::forward <Automaton> (that.automaton_)),
        roots (std::move (that.roots)),
        visitStatus (std::move (that.visitStatus)),
        queue (std::move (that.queue)) {}

    DepthFirstTraversalRange & operator= (DepthFirstTraversalRange && that) {
        this->automaton_ = std::forward <Automaton> (that.automaton_);
        this->roots = std::move (that.roots);
        this->visitStatus = std::move (that.visitStatus);
        this->queue = std::move (that.queue);
        return *this;
    }

    bool empty (::direction::front) const
    { return queue.empty() && range::empty (roots); }

    /**
    Return the next element.
    \internal
    Since there is no "yield" statement in C++, we have to leave the object in a
    recognisable state each time we return.
    With a few \c if statements, we should essentially be restarting just after
    the previous return statement.
    That is the way this code should be read.
    */
    Report chop_in_place (::direction::front const & front) {
        assertInvariants();
        assert (!empty (front));

        if (queue.empty()) {
            // The first element of "roots" must be a state that has not
            // been seen yet, so push it onto the queue.
            auto root = range::chop_in_place (roots);
            queue.push (Position (automaton(), root));
            return Report (root, TraversalEvent::newRoot);
        }

        while (true) {
            // If necessary, report that a new state is being visited.
            Position & position = queue.head();
            if (!position.visiting) {
                position.visiting = true;
                visitStatus.set (position.state, visiting);
                return Report (position.state, TraversalEvent::visit);
            }

            // Go through the arcs attached to this state and report on them.
            while (true) {
                // If the state is finished, report that.
                if (range::empty (position.arcs)) {
                    // Save state because it will go out of scope.
                    State state = position.state;
                    visitStatus.set (state, visited);
                    queue.pop();

                    if (queue.empty()) {
                        // The current tree is empty.
                        // Before returning, we need to get "roots" in valid
                        // state, with as its first element a root that has not
                        // been seen yet, or empty.
                        while (!range::empty (roots)) {
                            auto root = range::first (roots);
                            if (visitStatus [root] == unvisited)
                                // roots now starts with a good start state.
                                break;
                            range::chop_in_place (roots);
                        }
                    }

                    return Report (state, TraversalEvent::finishVisit);
                }

                // Consider the state that the next arc leads to.
                auto && arc = range::chop_in_place (position.arcs);
                auto && next = arc.state (Direction());
                auto status = visitStatus [next];
                if (status == unvisited) {
                    assert (status == unvisited);
                    // Visit the next state.
                    queue.push (Position (automaton(), next));
                    // Start from the top to deal with the new head of the
                    // queue.
                    break;
                } else if (status == visiting) {
                    return Report (next, TraversalEvent::backState);
                    // Don't revisit this state.
                } else {
                    assert (status == visited);
                    return Report (next, TraversalEvent::forwardOrCrossState);
                    // Don't revisit this state.
                }
            }
        }
    }
};

} // namespace flipsta

namespace range {

    template <class Automaton, class Direction> struct tag_of_qualified <
        flipsta::DepthFirstTraversalRange <Automaton, Direction>>
    { typedef flipsta::DepthFirstTraversalRangeTag type; };

    namespace operation {

        template <class Range> struct chop <
            flipsta::DepthFirstTraversalRangeTag, direction::front, Range &&>
        : chop_by_chop_in_place <flipsta::DepthFirstTraversalRangeTag,
            direction::front, Range &&> {};

    } // namespace operation

} // namespace range

#endif // FLIPSTA_TRAVERSE_HPP_INCLUDED
