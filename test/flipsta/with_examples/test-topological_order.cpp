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
