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
This checks flipsta::traverse.
The order in which this runs is not fully defined, so this test checks whether
the results correspond to an order, which has been manually checked.
It may be better to formulate invariants that must be satisfied instead.
*/

#define BOOST_TEST_MODULE test_flipsta_traverse
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/traverse.hpp"

#include "math/arithmetic_magma.hpp"

#include "flipsta/automaton.hpp"

#include "tracked_state.hpp"

using range::first;
using range::empty;
using range::chop;
using range::chop_in_place;

using flipsta::forward;
using flipsta::backward;

BOOST_AUTO_TEST_SUITE(test_suite_traverse)

template <class Range> inline void printReports (Range && range) {
    RANGE_FOR_EACH (report, range::view_once (std::forward <Range> (range))) {
        typedef flipsta::TraversalEvent Event;

        std::cout << report.state << ' ';

        switch (report.event) {
        case Event::newRoot:
            std::cout << "newRoot";
            break;
        case Event::visit:
            std::cout << "visit";
            break;
        case Event::backState:
            std::cout << "backState";
            break;
        case Event::forwardOrCrossState:
            std::cout << "forwardOrCrossState";
            break;
        case Event::finishVisit:
            std::cout << "finishVisit";
            break;
        }
        std::cout << std::endl;
    }
}

template <class State, class Direction, class Automaton, class Reference>
    void compare (Automaton const & automaton, Reference const & reference_)
{
    auto traversedStates = flipsta::traverse (automaton, Direction());
    static_assert (std::is_constructible <
        decltype (traversedStates), decltype (traversedStates) &&>::value,
        "The range must be moveable.");
    static_assert (!std::is_constructible <
        decltype (traversedStates), decltype (traversedStates) const &>::value,
        "The range is not copyable.");

    typedef flipsta::TraversedState <State> Report;

    unsigned count = 0;
    auto reference = range::view (reference_);
    while (!empty (traversedStates)) {
        // Interleave both "chop_in_place" and "chop".
        if (count & 0x1) {
            Report report = chop_in_place (traversedStates);
            BOOST_CHECK_EQUAL (report.state, first (reference).state);
            BOOST_CHECK (report.event == first (reference).event);
        } else {
            auto chopped = chop (std::move (traversedStates));
            Report report = chopped.move_first();
            traversedStates = chopped.move_rest();
            BOOST_CHECK_EQUAL (report.state, first (reference).state);
            BOOST_CHECK (report.event == first (reference).event);
        }
        BOOST_CHECK (!empty (reference));
        if (empty (reference))
            break;
        reference = range::drop (reference);
        ++ count;
    }
    BOOST_CHECK (empty (reference));
}

template <class State> void checkTraverseSimple() {
    typedef flipsta::Automaton <State, float> Automaton;

    Automaton automaton;

    automaton.addState (1);
    automaton.addState (2);
    automaton.addState (3);

    automaton.addArc (1, 1, .5);
    automaton.addArc (1, 2, 4);
    automaton.addArc (1, 3, 2);
    automaton.addArc (2, 1, -5);
    automaton.addArc (3, 2, 10.5);

    typedef flipsta::TraversedState <State> Report;
    typedef flipsta::TraversalEvent Event;

    // Forward.
    {
        std::vector <Report> reference;
        reference.push_back (Report (1, Event::newRoot));
        reference.push_back (Report (1, Event::visit));
        reference.push_back (Report (3, Event::visit));
        reference.push_back (Report (2, Event::visit));
        reference.push_back (Report (1, Event::backState));
        reference.push_back (Report (2, Event::finishVisit));
        reference.push_back (Report (3, Event::finishVisit));
        reference.push_back (Report (2, Event::forwardOrCrossState));
        reference.push_back (Report (1, Event::backState));
        reference.push_back (Report (1, Event::finishVisit));

        /*
        std::cout << "****************" << std::endl;
        printReports (traverse (automaton, forward));
        std::cout << "***" << std::endl;
        printReports (reference);
        */

        compare <State, flipsta::Forward> (automaton, reference);
    }
    // Backward.
    {
        std::vector <Report> reference;
        reference.push_back (Report (1, Event::newRoot));
        reference.push_back (Report (1, Event::visit));
        reference.push_back (Report (2, Event::visit));
        reference.push_back (Report (3, Event::visit));
        reference.push_back (Report (1, Event::backState));

        reference.push_back (Report (3, Event::finishVisit));
        reference.push_back (Report (1, Event::backState));
        reference.push_back (Report (2, Event::finishVisit));
        reference.push_back (Report (1, Event::backState));

        reference.push_back (Report (1, Event::finishVisit));

        /*
        std::cout << "****************" << std::endl;
        printReports (traverse (automaton, backward));
        std::cout << "***" << std::endl;
        printReports (reference);
        */

        compare <State, flipsta::Backward> (automaton, reference);
    }
}

BOOST_AUTO_TEST_CASE (testTraverseSimple) {
    checkTraverseSimple <int>();
    checkTraverseSimple <flipsta::Dense <int>>();
}

// Check that moving the result of traverse() is a cheap operation.
BOOST_AUTO_TEST_CASE (testTraverseMove) {
    typedef TrackedState State;
    typedef flipsta::Automaton <State, float> Automaton;

    utility::tracked_registry registry;
    {
        Automaton automaton;

        automaton.addState (State (registry, 1));
        automaton.addState (State (registry, 2));
        automaton.addState (State (registry, 3));

        automaton.addArc (State (registry, 1), State (registry, 1), .5);
        automaton.addArc (State (registry, 1), State (registry, 2), 4);
        automaton.addArc (State (registry, 1), State (registry, 3), 2);
        automaton.addArc (State (registry, 2), State (registry, 1), -5);
        automaton.addArc (State (registry, 3), State (registry, 2), 10.5);

        auto traversedStates = traverse (automaton, forward);

        chop_in_place (traversedStates);
        chop_in_place (traversedStates);
        chop_in_place (traversedStates);
        chop_in_place (traversedStates);

        auto traversedStates2 = traverse (automaton, forward);

        int valueConstructCount = registry.value_construct_count();
        int copyCount = registry.copy_count();
        int moveCount = registry.move_count();
        int copyAssignCount = registry.copy_assign_count();
        int moveAssignCount = registry.move_assign_count();
        int swapCount = registry.swap_count();
        // int destructCount = registry.destruct_count();
        // int destructMovedCount = registry.destruct_moved_count();

        // Move-construction.
        auto traversedStatesNew = std::move (traversedStates);
        // Move-assignment.
        traversedStates2 = std::move (traversedStatesNew);

        // Both of these should be simple pointer operations.
        BOOST_CHECK_EQUAL (valueConstructCount,
            registry.value_construct_count());
        BOOST_CHECK_EQUAL (copyCount, registry.copy_count());
        BOOST_CHECK_EQUAL (moveCount, registry.move_count());
        BOOST_CHECK_EQUAL (copyAssignCount, registry.copy_assign_count());
        BOOST_CHECK_EQUAL (moveAssignCount, registry.move_assign_count());
        BOOST_CHECK_EQUAL (swapCount, registry.swap_count());
        // BOOST_CHECK_EQUAL (destructCount, registry.destruct_count());
        // BOOST_CHECK_EQUAL (destructMovedCount,
        //     registry.destruct_moved_count());
    }
}

/**
Test with multiple root nodes.
*/
template <class State> void checkTraverseComplex() {
    typedef flipsta::Automaton <State, float> Automaton;

    Automaton automaton;

    automaton.addState ('a');
    automaton.addState ('b');
    automaton.addState ('c');
    automaton.addState ('d');
    automaton.addState ('e');
    automaton.addState ('f');
    automaton.addState ('g');
    automaton.addState ('h');

    automaton.addArc ('a', 'b', 1);
    automaton.addArc ('b', 'c', 1);
    automaton.addArc ('a', 'c', 1);
    automaton.addArc ('a', 'f', 1);
    automaton.addArc ('f', 'g', 1);
    automaton.addArc ('g', 'a', 1);
    automaton.addArc ('g', 'c', 1);

    automaton.addArc ('d', 'e', 1);
    automaton.addArc ('d', 'a', 1);
    automaton.addArc ('d', 'f', 1);
    automaton.addArc ('e', 'f', 1);

    typedef flipsta::TraversedState <State> Report;
    typedef flipsta::TraversalEvent Event;

    std::vector <Report> reference;
    reference.push_back (Report ('a', Event::newRoot));
    reference.push_back (Report ('a', Event::visit));

    reference.push_back (Report ('f', Event::visit));
    reference.push_back (Report ('g', Event::visit));
    reference.push_back (Report ('c', Event::visit));
    reference.push_back (Report ('c', Event::finishVisit));
    reference.push_back (Report ('a', Event::backState));
    reference.push_back (Report ('g', Event::finishVisit));
    reference.push_back (Report ('f', Event::finishVisit));

    reference.push_back (Report ('c', Event::forwardOrCrossState));

    reference.push_back (Report ('b', Event::visit));
    reference.push_back (Report ('c', Event::forwardOrCrossState));
    reference.push_back (Report ('b', Event::finishVisit));
    reference.push_back (Report ('a', Event::finishVisit));

    reference.push_back (Report ('d', Event::newRoot));
    reference.push_back (Report ('d', Event::visit));
    reference.push_back (Report ('f', Event::forwardOrCrossState));
    reference.push_back (Report ('a', Event::forwardOrCrossState));
    reference.push_back (Report ('e', Event::visit));
    reference.push_back (Report ('f', Event::forwardOrCrossState));
    reference.push_back (Report ('e', Event::finishVisit));
    reference.push_back (Report ('d', Event::finishVisit));

    reference.push_back (Report ('h', Event::newRoot));
    reference.push_back (Report ('h', Event::visit));
    reference.push_back (Report ('h', Event::finishVisit));

    /*
    std::cout << "****************" << std::endl;
    printReports (traverse (automaton, forward));
    std::cout << "***" << std::endl;
    printReports (reference);
    */

    compare <State, flipsta::Forward> (automaton, reference);
}

BOOST_AUTO_TEST_CASE (testTraverseComplex) {
    checkTraverseComplex <char>();
    checkTraverseComplex <flipsta::Dense <char>>();
}

BOOST_AUTO_TEST_SUITE_END()
