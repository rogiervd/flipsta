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
Check that the order of the result states(), terminalStates(), arcsOn() is
always the same, given the same input.
This means, in particular, that a different hash (such as for a state which is
a pointer) should not make any difference.

This is hard to check.
The following randomises the order in memory that states are in.
If the class then returns them in the same order every time the test is run,
it should be fine.
*/

#define BOOST_TEST_MODULE test_flipsta_automaton_state_order
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/automaton.hpp"

#include <memory>
#include <algorithm>
#include <random>
#include <iostream>

#include "range/core.hpp"
#include "range/walk_size.hpp"
#include "range/take.hpp"
#include "range/count.hpp"
#include "range/zip.hpp"

#include "math/arithmetic_magma.hpp"

#include "flipsta/core/hash_helper.hpp"

BOOST_AUTO_TEST_SUITE(test_suite_flipsta_automaton_state_order)

using flipsta::forward;
using flipsta::backward;

using flipsta::hasState;
using flipsta::terminalStates;
using flipsta::terminalStatesCompressed;
using flipsta::terminalLabel;
using flipsta::terminalLabelCompressed;
using flipsta::arcsOn;
using flipsta::arcsOnCompressed;

using range::empty;
using range::size;
using range::first;
using range::second;
using range::drop;

using range::view;
using range::walk_size;
using range::count;
using range::take;
using range::zip;

/**
Deliberately try to fool Automaton into mixing up the order of states.
*/
BOOST_AUTO_TEST_CASE (test_flipstaAutomaton_stateOrder) {
    typedef std::shared_ptr <int> State;
    typedef flipsta::Automaton <State, float> Automaton;

    std::vector <State> states;
    states.push_back (std::make_shared <int> (-1));
    states.push_back (std::make_shared <int> (-1));
    states.push_back (std::make_shared <int> (-1));
    states.push_back (std::make_shared <int> (-1));
    states.push_back (std::make_shared <int> (-1));

    // Mix up the states.
    std::random_device random_device;
    std::mt19937 generator (random_device());
    std::shuffle (states.begin(), states.end(), generator);

    // The probability of the states being sorted is now 2**-5.

    Automaton automaton;
    RANGE_FOR_EACH (i, count (size (states))) {
        // Make the states recognisable.
        *states [i] = i;
        automaton.addState (states [i]);
    }

    // Check that the states are given in a consistent order.
    {
        auto s = view (flipsta::states (automaton));
        auto reference = view (states);
        BOOST_CHECK_EQUAL (walk_size (s), walk_size (reference));
        RANGE_FOR_EACH (statePair, zip (s, reference)) {
            BOOST_CHECK_EQUAL (first (statePair), second (statePair));
        }
    }

    State a = states [0];
    State b = states [1];
    State c = states [2];
    State d = states [3];
    State e = states [4];

    automaton.setTerminalLabel (forward, b, 1.f);
    automaton.setTerminalLabel (forward, a, 5.f);
    automaton.setTerminalLabel (forward, c, 7.f);
    automaton.setTerminalLabel (forward, a, 3.f);

    // Check that the start states are given in a consistent order.
    {
        auto s = view (flipsta::terminalStates (automaton, forward));
        // The first three states, in order their label was set.
        std::vector <State> reference;
        reference.push_back (b);
        reference.push_back (a);
        reference.push_back (c);

        BOOST_CHECK_EQUAL (walk_size (s), size (reference));
        RANGE_FOR_EACH (statePair, zip (s, reference)) {
            BOOST_CHECK_EQUAL (first (first (statePair)), second (statePair));
        }
    }

    automaton.setTerminalLabel (backward, e, 4.f);
    automaton.setTerminalLabel (backward, d, 6.f);
    automaton.setTerminalLabel (backward, c, 8.f);
    automaton.setTerminalLabel (backward, e, 2.f);

    // Check that the start states are given in a consistent order.
    {
        auto s = view (flipsta::terminalStates (automaton, backward));
        // The first three states, in order their label was set.
        std::vector <State> reference;
        reference.push_back (e);
        reference.push_back (d);
        reference.push_back (c);

        BOOST_CHECK_EQUAL (walk_size (s), size (reference));
        RANGE_FOR_EACH (statePair, zip (s, reference)) {
            BOOST_CHECK_EQUAL (first (first (statePair)), second (statePair));
        }
    }

    // Check that the arcs on a state (on e) are given in a consistent order.
    automaton.addArc (a, c, 12);
    automaton.addArc (a, e, 13);
    automaton.addArc (e, c, 14);
    automaton.addArc (e, a, 15);
    automaton.addArc (d, c, 16);
    automaton.addArc (e, d, 17);
    {
        auto s = view (flipsta::arcsOn (automaton, forward, e));
        // The first three states, in the order that this turns out to be.
        std::vector <State> reference;
        reference.push_back (d);
        reference.push_back (a);
        reference.push_back (c);

        BOOST_CHECK_EQUAL (walk_size (s), size (reference));
        RANGE_FOR_EACH (statePair, zip (s, reference)) {
            BOOST_CHECK_EQUAL (
                first (statePair).state (forward), second (statePair));
            // std::cout << *first (statePair).state (forward) << ' '
            //     << *second (statePair) << std::endl;
        }
    }

    automaton.addArc (e, c, 18);
    {
        auto s = view (flipsta::arcsOn (automaton, forward, e));
        // The first three states, in the order that this turns out to be.
        std::vector <State> reference;
        reference.push_back (c);
        reference.push_back (d);
        reference.push_back (a);
        reference.push_back (c);

        BOOST_CHECK_EQUAL (walk_size (s), size (reference));
        RANGE_FOR_EACH (statePair, zip (s, reference)) {
            BOOST_CHECK_EQUAL (
                first (statePair).state (forward), second (statePair));
            std::cout << *first (statePair).state (forward) << ' '
                << *second (statePair) << std::endl;
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
