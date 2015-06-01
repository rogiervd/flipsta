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

#define BOOST_TEST_MODULE test_flipsta_automaton_weight
#include "utility/test/boost_unit_test.hpp"

#include <boost/exception/get_error_info.hpp>

#include "flipsta/automaton.hpp"

#include <iostream>

#include "range/core.hpp"
#include "range/for_each_macro.hpp"
#include "range/walk_size.hpp"

#include "math/sequence.hpp"
#include "math/cost.hpp"
#include "math/lexicographical.hpp"

/** \file
Test Automaton with weights.
*/

BOOST_AUTO_TEST_SUITE(test_suite_flipsta_automaton_weight)

using flipsta::forward;
using flipsta::backward;

using flipsta::hasState;
using flipsta::terminalStates;
using flipsta::terminalStatesCompressed;
using flipsta::terminalLabel;
using flipsta::terminalLabelCompressed;
using flipsta::arcsOn;
using flipsta::arcsOnCompressed;

using range::size;
using range::first;
using range::second;
using range::third;
using range::walk_size;

// Simple shortest-distance algorithm.
// Only works for noncyclical automata.
template <class GeneralLabel, class Automaton, class Direction>
    GeneralLabel shortestDistance (
        Automaton const & automaton, Direction forward)
{
    auto backward = flipsta::opposite (forward);
    typedef typename Automaton::State State;
    std::map <State, GeneralLabel> current;
    std::set <State> todo;
    GeneralLabel shortest = math::zero <GeneralLabel>();

    // Initialise.
    RANGE_FOR_EACH (stateAndLabel, terminalStates (automaton, forward)) {
        auto state = first (stateAndLabel);
        current.insert (std::make_pair (state, second (stateAndLabel)));
        todo.insert (state);
    }

    while (!todo.empty()) {
        auto sourcePosition = todo.begin();
        auto source = *sourcePosition;
        todo.erase (sourcePosition);

        auto labelPosition = current.find (source);
        assert (labelPosition != current.end());
        auto & label = labelPosition->second;
        RANGE_FOR_EACH (arc, arcsOn (automaton, forward, source)) {
            auto nextState = arc.state (forward);
            auto nextPosition = current.find (nextState);
            if (nextPosition == current.end())
                nextPosition = current.insert (std::make_pair (
                    nextState, math::zero <GeneralLabel>())).first;
            auto & nextLabel = nextPosition->second;
            nextLabel = nextLabel
                + flipsta::times (forward, label, arc.label());
            todo.insert (nextState);

            shortest = shortest + flipsta::times (forward,
                nextLabel, terminalLabel (automaton, backward, nextState));
        }
    }
    return shortest;
}

BOOST_AUTO_TEST_CASE (test_flipsta_automaton_weight) {
    typedef int State;
    typedef math::lexicographical <math::over <
        math::cost <double>, math::optional_sequence <char>>> Label;
    typedef math::lexicographical <math::over <
        math::cost <double>, math::sequence <char>>> GeneralLabel;

    typedef flipsta::Automaton <State, Label> Automaton;

    typedef math::empty_sequence <char> Empty;
    typedef Automaton::TerminalLabel TerminalLabel;
    static_assert (std::is_same <TerminalLabel,
        math::lexicographical <math::over <
            math::cost <double>, math::empty_sequence <char>>>>::value,
        "The terminal label must be the appropriate one-valued sequence");

    Automaton automaton;

    automaton.addState (1);
    automaton.addState (2);
    automaton.addState (3);
    automaton.addState (4);
    automaton.addState (5);

    automaton.addArc (1, 2, Label (1., 'a'));
    automaton.addArc (2, 4, Label (7., 'b'));
    automaton.addArc (2, 4, Label (5., 'c'));
    automaton.addArc (1, 3, Label (2., 'd'));
    automaton.addArc (3, 4, Label (3.5, 'e'));
    automaton.addArc (4, 5, Label (2.5, 'q'));

    automaton.setTerminalLabel (forward, 1, TerminalLabel (0., Empty()));
    automaton.setTerminalLabel (backward, 2, TerminalLabel (7., Empty()));
    automaton.setTerminalLabel (backward, 5, TerminalLabel (2., Empty()));
    // Set the label again, replacing the earlier value.
    automaton.setTerminalLabel (backward, 2, TerminalLabel (12., Empty()));

    BOOST_CHECK_EQUAL (
        first (terminalLabel (automaton, backward, 2).components()).value(),
        12.);

    // Compute the shortest distance in the forward direction.
    auto shortestForward = shortestDistance <GeneralLabel> (automaton, forward);
    // This should be states 1-3-4-5.
    // Weight: 0. + 2. + 3.5 + 2.5 + 2. = 10.
    // Symbols: d e q
    std::cout << "Shortest distance in forward direction is "
        << shortestForward << std::endl;

    BOOST_CHECK_EQUAL (first (shortestForward.components()).value(), 10.);
    auto && symbols = second (shortestForward.components()).symbols();
    BOOST_CHECK_EQUAL (size (symbols), 3);
    BOOST_CHECK_EQUAL (first (symbols), 'd');
    BOOST_CHECK_EQUAL (second (symbols), 'e');
    BOOST_CHECK_EQUAL (third (symbols), 'q');

    // Compute the shortest distance in the backward direction.
    auto shortestBackward =
        shortestDistance <GeneralLabel> (automaton, backward);
    // This should be exactly the same as the above.
    std::cout << "Shortest distance in backward direction is "
        << shortestBackward << std::endl;

    BOOST_CHECK (shortestBackward == shortestForward);
}

BOOST_AUTO_TEST_SUITE_END()
