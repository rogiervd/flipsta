/*
Copyright 2014, 2015 Rogier van Dalen.

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

#define BOOST_TEST_MODULE test_flipsta_automaton
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/automaton.hpp"

#include <boost/exception/get_error_info.hpp>

#include "range/core.hpp"
#include "range/for_each_macro.hpp"
#include "range/walk_size.hpp"

#include "math/sequence.hpp"

BOOST_AUTO_TEST_SUITE(test_suite_flipsta_automaton)

using flipsta::forward;
using flipsta::backward;

using flipsta::hasState;
using flipsta::terminalStates;
using flipsta::terminalStatesCompressed;
using flipsta::terminalLabel;
using flipsta::terminalLabelCompressed;
using flipsta::arcsOn;
using flipsta::arcsOnCompressed;

using range::first;
using range::second;
using range::walk_size;

typedef int State;
typedef math::single_sequence <char> Label;
typedef flipsta::Automaton <State, Label> Automaton;

void checkAutomaton (Automaton & automaton) {
    typedef math::empty_sequence <char> TerminalLabel;
    typedef math::sequence <char> GeneralLabel;

    static_assert (
        std::is_same <TerminalLabel, Automaton::TerminalLabel>::value, "");
    static_assert (
        std::is_same <flipsta::StateType <Automaton>::type, State>::value, "");

    std::shared_ptr <math::alphabet <char>> alphabet
        = flipsta::descriptor (automaton).alphabet();

    automaton.addState (1);
    BOOST_CHECK (!hasState (automaton, 0));
    BOOST_CHECK (hasState (automaton, 1));
    BOOST_CHECK (!hasState (automaton, 2));

    static_assert (flipsta::Has <
        flipsta::callable::HasState (Automaton, State)>::value, "");
    static_assert (flipsta::Has <
        flipsta::callable::States (Automaton)>::value, "");

    static_assert (flipsta::Has <flipsta::callable::TerminalStatesCompressed (
        Automaton, flipsta::Forward)>::value, "");
    static_assert (flipsta::Has <flipsta::callable::TerminalStatesCompressed (
        Automaton, flipsta::Backward)>::value, "");
    static_assert (!flipsta::Has <flipsta::callable::TerminalStatesCompressed (
        Automaton, int)>::value, "");

    static_assert (flipsta::Has <flipsta::callable::TerminalStates (
        Automaton, flipsta::Forward)>::value, "");
    static_assert (flipsta::Has <flipsta::callable::TerminalStates (
        Automaton, flipsta::Backward)>::value, "");
    static_assert (!flipsta::Has <flipsta::callable::TerminalStates (
        Automaton, int)>::value, "");

    static_assert (flipsta::Has <flipsta::callable::ArcsOnCompressed (
        Automaton, flipsta::Forward, State)>::value, "");
    static_assert (flipsta::Has <flipsta::callable::ArcsOnCompressed (
        Automaton, flipsta::Backward, State)>::value, "");
    static_assert (!flipsta::Has <flipsta::callable::ArcsOnCompressed (
        Automaton, int, State)>::value, "");

    static_assert (flipsta::Has <flipsta::callable::ArcsOn (
        Automaton, flipsta::Forward, State)>::value, "");
    static_assert (flipsta::Has <flipsta::callable::ArcsOn (
        Automaton, flipsta::Backward, State)>::value, "");
    static_assert (!flipsta::Has <flipsta::callable::ArcsOn (
        Automaton, int, State)>::value, "");

    BOOST_CHECK_THROW (automaton.addState (1), flipsta::StateExists);
    // And check that the state is given as error info.
    try {
        automaton.addState (1);
    } catch (flipsta::StateExists & error) {
        State const * state = boost::get_error_info <
            flipsta::TagErrorInfoStateType <State>::type> (error);
        BOOST_CHECK (!!state);
        if (state) {
            BOOST_CHECK_EQUAL (*state, 1);
        }
    }

    {
        auto const & states = flipsta::states (automaton);

        BOOST_CHECK_EQUAL (first (states), 1);
        BOOST_CHECK_EQUAL (walk_size (states), 1u);
    }

    automaton.addState (3);
    automaton.addState (2);
    BOOST_CHECK (hasState (automaton, 1));
    BOOST_CHECK (hasState (automaton, 2));
    BOOST_CHECK (hasState (automaton, 3));

    BOOST_CHECK_EQUAL (walk_size (flipsta::states (automaton)), 3u);
    // State 1 start state.
    automaton.setTerminalLabel (forward, 1, TerminalLabel());
    {
        auto && startStates = terminalStates (automaton, forward);
        BOOST_CHECK_EQUAL (walk_size (startStates), 1u);
        auto stateAndLabel = first (startStates);
        BOOST_CHECK_EQUAL (first (stateAndLabel), 1);
        auto label = second (stateAndLabel);
        BOOST_CHECK_EQUAL (label, TerminalLabel());
        BOOST_CHECK_EQUAL (label, math::one <Label>());

        BOOST_CHECK_EQUAL (terminalLabel (automaton, forward, 1),
            math::one <decltype (label)>());
        // 2 is not a start state, so the terminal label is returned as zero.
        BOOST_CHECK_EQUAL (terminalLabel (automaton, forward, 2),
            math::zero <decltype (label)>());
        // 5 does not exist, so the terminal label is returned as zero.
        BOOST_CHECK_EQUAL (terminalLabel (automaton, forward, 5),
            math::zero <decltype (label)>());
    }
    {
        auto && startStates = terminalStatesCompressed (
            automaton, forward);
        BOOST_CHECK_EQUAL (walk_size (startStates), 1u);
        auto stateAndLabel = first (startStates);
        BOOST_CHECK_EQUAL (first (stateAndLabel), 1);
        auto label = second (stateAndLabel);
        BOOST_CHECK_EQUAL (label, math::one <decltype (label)>());

        BOOST_CHECK (terminalLabelCompressed (automaton, forward, 1) ==
            math::one <decltype (label)>());
        // 2 is not a start state, so the terminal label is returned as zero.
        BOOST_CHECK (math::is_annihilator (math::times,
            terminalLabelCompressed (automaton, forward, 2)));
        // 5 does not exist, so the terminal label is returned as zero.
        BOOST_CHECK (math::is_annihilator (math::times,
            terminalLabelCompressed (automaton, forward, 5)));
    }

    // Set state 2 to also be a start state.
    automaton.setTerminalLabel (forward, 2, TerminalLabel());
    {
        auto const & startStates = terminalStates (automaton, forward);
        BOOST_CHECK_EQUAL (walk_size (startStates), 2u);
        RANGE_FOR_EACH (stateAndLabel, startStates) {
            auto state = first (stateAndLabel);
            BOOST_CHECK (state == 1 || state == 2);
            BOOST_CHECK_EQUAL (
                second (stateAndLabel), TerminalLabel());
        }
    }

    // State 1 not a start state any more, only state 2.
    automaton.setTerminalLabel (forward,
        1, math::sequence_annihilator <char>());
    {
        auto const & startStates = terminalStates (automaton, forward);
        BOOST_CHECK_EQUAL (walk_size (startStates), 1u);
        auto stateAndLabel = first (startStates);
        BOOST_CHECK_EQUAL (first (stateAndLabel), 2);
        BOOST_CHECK_EQUAL (second (stateAndLabel), TerminalLabel());
    }

    {
        // It is not possible to set a terminal label to a non-empty string.
        BOOST_CHECK_THROW (
            automaton.setTerminalLabel (backward, 3,
                GeneralLabel (std::string ("a"))),
            math::magma_not_convertible);
    }

    // Set state 3 as final state, using the GeneralLabel type.
    automaton.setTerminalLabel (backward, 3, GeneralLabel());
    {
        auto const & finalStates = terminalStates (
            automaton, backward);
        BOOST_CHECK_EQUAL (walk_size (finalStates), 1u);
        auto stateAndLabel = first (finalStates);
        BOOST_CHECK_EQUAL (first (stateAndLabel), 3);
        auto label = second (stateAndLabel);
        BOOST_CHECK_EQUAL (label, TerminalLabel());

        BOOST_CHECK (terminalLabel (automaton, backward, 3) ==
            math::one <decltype (label)>());
        BOOST_CHECK (math::is_annihilator (math::times,
            terminalLabel (automaton, backward, 2)));
        BOOST_CHECK (math::is_annihilator (math::times,
            terminalLabel (automaton, backward, 5)));
    }
    {
        auto const & finalStates = terminalStatesCompressed (
            automaton, backward);
        BOOST_CHECK_EQUAL (walk_size (finalStates), 1u);
        auto stateAndLabel = first (finalStates);
        BOOST_CHECK_EQUAL (first (stateAndLabel), 3);
        auto label = second (stateAndLabel);
        BOOST_CHECK_EQUAL (label, math::one <decltype (label)>());

        BOOST_CHECK (terminalLabelCompressed (automaton, backward, 3) ==
            math::one <decltype (label)>());
        BOOST_CHECK (math::is_annihilator (math::times,
            terminalLabelCompressed (automaton, backward, 2)));
        BOOST_CHECK (math::is_annihilator (math::times,
            terminalLabelCompressed (automaton, backward, 5)));
    }

    automaton.addArc (1, 2, Label ('a'));
    automaton.addArc (2, 2, Label ('b'));
    automaton.addArc (2, 3, Label ('c'));
    automaton.addArc (2, 3, Label ('d'));

    // Find arcs from source state.
    {
        auto arcsOutOf1 = arcsOn (automaton, forward, 1);
        BOOST_CHECK_EQUAL (walk_size (arcsOutOf1), 1u);
        {
            auto arc = first (arcsOutOf1);
            BOOST_CHECK_EQUAL (arc.state (backward), 1);
            BOOST_CHECK_EQUAL (arc.state (forward), 2);
            BOOST_CHECK_EQUAL (arc.label(), Label ('a'));
        }

        auto arcsOutOf2 = arcsOn (automaton, forward, 2);
        BOOST_CHECK_EQUAL (walk_size (arcsOutOf2), 3u);
        RANGE_FOR_EACH (arc, arcsOutOf2) {
            BOOST_CHECK_EQUAL (arc.state (backward), 2);
            if (arc.state (forward) == 2) {
                BOOST_CHECK_EQUAL (arc.label(), Label ('b'));
            } else {
                BOOST_CHECK_EQUAL (arc.state (forward), 3);
                BOOST_CHECK (arc.label() == Label ('c')
                    || arc.label() == Label ('d'));
            }
        }

        auto arcsOutOf3 = arcsOn (automaton, forward, 3);
        BOOST_CHECK (range::empty (arcsOutOf3));
    }

    // Find arcs from destination state.
    {
        auto arcs_into_1 = arcsOn (automaton, backward, 1);
        BOOST_CHECK (range::empty (arcs_into_1));

        auto arcs_into_2 = arcsOn (automaton, backward, 2);
        BOOST_CHECK_EQUAL (walk_size (arcs_into_2), 2u);
        RANGE_FOR_EACH (arc, arcs_into_2) {
            BOOST_CHECK_EQUAL (arc.state (forward), 2);
            if (arc.state (backward) == 1) {
                BOOST_CHECK_EQUAL (arc.label(), Label ('a'));
            } else {
                BOOST_CHECK_EQUAL (arc.state (backward), 2);
                BOOST_CHECK_EQUAL (arc.label(), Label ('b'));
            }
        }

        auto arcs_into_3 = arcsOn (automaton, backward, 3);
        BOOST_CHECK_EQUAL (walk_size (arcs_into_3), 2u);
        RANGE_FOR_EACH (arc, arcs_into_3) {
            BOOST_CHECK_EQUAL (arc.state (forward), 3);
            BOOST_CHECK_EQUAL (arc.state (backward), 2);
            BOOST_CHECK (arc.label() == Label ('c')
                || arc.label() == Label ('d'));
        }
    }

    // Check arcsOnCompressed.
    {
        auto internalArcsOutOf1 = arcsOnCompressed (automaton, forward, 1);
        BOOST_CHECK_EQUAL (walk_size (internalArcsOutOf1), 1u);
        {
            auto arc = first (internalArcsOutOf1);
            BOOST_CHECK_EQUAL (arc.state (backward), 1);
            BOOST_CHECK_EQUAL (arc.state (forward), 2);
            BOOST_CHECK (arc.label().symbol() == alphabet->get_dense ('a'));
        }
    }
}

BOOST_AUTO_TEST_CASE (test_flipstaAutomaton) {
    {
        Automaton automaton;
        checkAutomaton (automaton);
    }
    {
        auto alphabet = std::make_shared <math::alphabet <char>>();
        alphabet->add_symbol ('q');
        alphabet->add_symbol ('!');
        alphabet->add_symbol ('a');

        Automaton automaton (alphabet);
        // 'a' is the third symbol in the alphabet.
        checkAutomaton (automaton);
    }
}

BOOST_AUTO_TEST_SUITE_END()
