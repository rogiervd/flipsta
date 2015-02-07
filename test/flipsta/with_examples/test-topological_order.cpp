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

#define BOOST_TEST_MODULE test_flipsta_topological_order
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/topological_order.hpp"

#include <boost/exception/get_error_info.hpp>

#include "math/arithmetic_magma.hpp"

#include "flipsta/automaton.hpp"

#include "example_automata.hpp"

using range::first;
using range::empty;
using range::size;
using range::chop_in_place;

using flipsta::forward;
using flipsta::backward;
using flipsta::AutomatonNotAcyclic;

BOOST_AUTO_TEST_SUITE(test_suite_topological_order)

template <class Automaton, class Direction> inline
    void runThroughTopologicalOrder (
        Automaton const & automaton, Direction const & direction)
{
    // Exhaust the range but do nothing else.
    RANGE_FOR_EACH (state, flipsta::topologicalOrder (automaton, direction))
        (void) state;
}

/**
Test that requesting the topological order of an automaton with a cycle cause an
exception to be thrown.
*/
BOOST_AUTO_TEST_CASE (testTopologicalOrderCycle) {
    typedef int State;
    typedef flipsta::Automaton <State, double> Automaton;
    {
        Automaton automaton;
        automaton.addState (1);
        automaton.addArc (1, 1, 5.);

        try {
            runThroughTopologicalOrder (automaton, forward);
            BOOST_CHECK_MESSAGE (false, "Exception expected.");
        } catch (AutomatonNotAcyclic & error) {
            State const * state = boost::get_error_info <
                flipsta::TagErrorInfoStateType <State>::type> (error);
            BOOST_CHECK (!!state);
            if (state) {
                BOOST_CHECK_EQUAL (*state, 1);
            }
        }

        BOOST_CHECK_THROW (runThroughTopologicalOrder (automaton, forward),
            AutomatonNotAcyclic);
        BOOST_CHECK_THROW (runThroughTopologicalOrder (automaton, backward),
            AutomatonNotAcyclic);
    }
    {
        Automaton automaton;
        automaton.addState (1);
        automaton.addState (2);
        automaton.addState (3);
        automaton.addArc (1, 2, 5.);
        automaton.addArc (2, 3, 5.);
        automaton.addArc (3, 1, 5.);

        BOOST_CHECK_THROW (runThroughTopologicalOrder (automaton, forward),
            AutomatonNotAcyclic);
        BOOST_CHECK_THROW (runThroughTopologicalOrder (automaton, backward),
            AutomatonNotAcyclic);
    }
}

BOOST_AUTO_TEST_CASE (testTopologicalOrder) {
    auto automaton = acyclicExample();

    {
        auto order = flipsta::topologicalOrder (automaton, forward);

        BOOST_CHECK_EQUAL (first (order), 'd');
        BOOST_CHECK_EQUAL (range::second (order), 'c');
        BOOST_CHECK_EQUAL (range::third (order), 'a');
        BOOST_CHECK_EQUAL (range::fourth (order), 'f');
        BOOST_CHECK_EQUAL (range::fifth (order), 'b');
        BOOST_CHECK_EQUAL (range::sixth (order), 'e');
        BOOST_CHECK_EQUAL (size (order), 6);
    }
    {
        auto order = flipsta::topologicalOrder (automaton, backward);

        // Traverse destructively.
        BOOST_CHECK_EQUAL (chop_in_place (order), 'e');
        BOOST_CHECK_EQUAL (chop_in_place (order), 'b');
        BOOST_CHECK_EQUAL (chop_in_place (order), 'f');
        BOOST_CHECK_EQUAL (chop_in_place (order), 'a');
        BOOST_CHECK_EQUAL (chop_in_place (order), 'c');
        BOOST_CHECK_EQUAL (chop_in_place (order), 'd');
        BOOST_CHECK (empty (order));
    }
}

BOOST_AUTO_TEST_SUITE_END()