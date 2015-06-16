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
Test readAutomaton.
The first argument must be the file name of a symbol table, and the second
the file name of the automaton.

This currently merely checks only one automaton.
Then again, since reading the automaton is about reading different lines and
passing them correctly to the actual automaton, so this should be sufficient
currently.
*/

#define BOOST_TEST_MODULE test_automaton
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/att/automaton.hpp"

#include <iostream>
#include <ostream>

#include <boost/exception/all.hpp>

#include "range/walk_size.hpp"

using flipsta::hasState;
using flipsta::arcsOn;
using flipsta::forward;
using flipsta::backward;

using range::walk_size;
using range::first;
using range::second;
using range::third;

BOOST_AUTO_TEST_SUITE(test_readAutomaton)

BOOST_AUTO_TEST_CASE (from_example) {
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char ** argv = boost::unit_test::framework::master_test_suite().argv;

    // Otherwise there are no files to test on.
    BOOST_REQUIRE_EQUAL (argc, 3);

    try {
        auto symbolTable = flipsta::att::readSymbolTable (argv [1]);
        auto automaton = flipsta::att::readAutomaton (argv [2],
            *symbolTable, *symbolTable);

        BOOST_CHECK (hasState (*automaton, std::size_t (0)));
        BOOST_CHECK (hasState (*automaton, std::size_t (1)));
        BOOST_CHECK (hasState (*automaton, std::size_t (2)));
        BOOST_CHECK (hasState (*automaton, std::size_t (3)));
        BOOST_CHECK (hasState (*automaton, std::size_t (4)));

        // One start state with cost 0.
        {
            auto startStates = flipsta::terminalStates (
                *automaton, flipsta::forward);
            BOOST_CHECK_EQUAL (range::walk_size (startStates), 1u);
            auto weight = third (second (first (startStates)).components());
            BOOST_CHECK_EQUAL (weight.value(), 0);
        }

        // Two end states.
        {
            auto endStates = flipsta::terminalStates (
                *automaton, flipsta::backward);
            BOOST_CHECK_EQUAL (range::walk_size (endStates), 2u);
            RANGE_FOR_EACH (endState, endStates) {
                if (first (endState) == 3) {
                    auto finalLabel = second (endState).components();
                    BOOST_CHECK_EQUAL (third (finalLabel).value(), 0.);
                } else {
                    BOOST_CHECK_EQUAL (first (endState), 4);
                    auto finalLabel = second (endState).components();
                    BOOST_CHECK_EQUAL (third (finalLabel).value(), 2.);
                }
            }
        }

        // Transitions: from state 0.
        {
            auto arcs =  flipsta::arcsOn (*automaton, forward, 0);
            BOOST_CHECK_EQUAL (walk_size (arcs), 1);
            RANGE_FOR_EACH (arc, arcs) {
                auto components = arc.label().components();
                std::cout << first (components)
                    << ' ' << second (components)
                    << ' ' << third (components) << std::endl;
            }

            auto arc = first (arcs);
            BOOST_CHECK_EQUAL (arc.state (backward), 0);
            BOOST_CHECK_EQUAL (arc.state (forward), 1);
            auto components = arc.label().components();
            BOOST_CHECK_EQUAL (first (components).symbol().get(), "a");
            BOOST_CHECK_EQUAL (second (components).symbol().get(), "a");
            BOOST_CHECK_EQUAL (third (components).value(), 0);
        }
        // Into state 2.
        {
            auto arcs =  flipsta::arcsOn (*automaton, backward, 2);
            BOOST_CHECK_EQUAL (walk_size (arcs), 2);
            RANGE_FOR_EACH (arc, arcs) {
                auto components = arc.label().components();
                std::cout << first (components)
                    << ' ' << second (components)
                    << ' ' << third (components) << std::endl;
            }

            auto testArc = first (arcs);
            bool swap =
                (first (testArc.label().components()).symbol().get() == "c");
            auto firstArc = swap ? second (arcs) : first (arcs);
            auto secondArc = swap ? first (arcs) : second (arcs);

            {
                BOOST_CHECK_EQUAL (firstArc.state (backward), 1);
                BOOST_CHECK_EQUAL (firstArc.state (forward), 2);
                auto components = firstArc.label().components();
                BOOST_CHECK_EQUAL (first (components).symbol().get(), "b");
                BOOST_CHECK_EQUAL (second (components).symbol().get(), "b");
                BOOST_CHECK_EQUAL (third (components).value(), 2);
            }

            {
                BOOST_CHECK_EQUAL (secondArc.state (backward), 1);
                BOOST_CHECK_EQUAL (secondArc.state (forward), 2);
                auto components = secondArc.label().components();
                BOOST_CHECK_EQUAL (first (components).symbol().get(), "c");
                BOOST_CHECK_EQUAL (second (components).symbol().get(), "c");
                BOOST_CHECK_EQUAL (third (components).value(), 0);
            }
        }
    } catch (boost::exception &e) {
        std::cerr << "Unexpected error while parsing AT&T-style automaton.\n";
        flipsta::explainException (std::cerr, e);

        BOOST_FAIL ("No exception should have been thrown.");
    }
}

BOOST_AUTO_TEST_SUITE_END()
