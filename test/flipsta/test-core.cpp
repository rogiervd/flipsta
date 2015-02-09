/*
Copyright 2014 Rogier van Dalen.

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

#define BOOST_TEST_MODULE test_core
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/core.hpp"

#include <string>

#include "math/sequence.hpp"

BOOST_AUTO_TEST_SUITE(test_suite_core)

struct NotAnAutomaton {
    bool hasState (int) const;
    int states() const;
    double terminalStates (flipsta::Forward) const;
};

using flipsta::times;
using flipsta::forward;
using flipsta::backward;

using flipsta::Has;
using flipsta::Forward;
using flipsta::Backward;

namespace callable = flipsta::callable;

BOOST_AUTO_TEST_CASE (test_direction) {
    static_assert (flipsta::IsDirection <Forward>::value, "");
    static_assert (flipsta::IsDirection <Backward>::value, "");
    static_assert (!flipsta::IsDirection <void>::value, "");
    static_assert (!flipsta::IsDirection <int>::value, "");

    static_assert (std::is_same <
        flipsta::Opposite <Forward>::type, Backward>::value, "");
    static_assert (std::is_same <
        flipsta::Opposite <Backward>::type, Forward>::value, "");

    static_assert (std::is_same <
        decltype (flipsta::opposite (forward)), Backward>::value, "");
    static_assert (std::is_same <
        decltype (flipsta::opposite (backward)), Forward>::value, "");
}

BOOST_AUTO_TEST_CASE (test_operations) {
    static_assert (!Has <callable::Tag (NotAnAutomaton)>::value, "");

    static_assert (!Has <callable::States (NotAnAutomaton)>::value, "");

    static_assert (!Has <callable::HasState (int, int)>::value, "");
    static_assert (!Has <callable::HasState (NotAnAutomaton, int)>::value, "");

    static_assert (!Has <callable::TerminalStates (
        NotAnAutomaton, Forward)>::value, "");
    static_assert (!Has <callable::TerminalStates (
        NotAnAutomaton, Backward)>::value, "");
    static_assert (!Has <callable::TerminalStates (
        NotAnAutomaton, int)>::value, "");


    static_assert (!Has <callable::TerminalStatesCompressed (
        NotAnAutomaton, Forward)>::value, "");
    static_assert (!Has <callable::TerminalStatesCompressed (
        NotAnAutomaton, Backward)>::value, "");
    static_assert (!Has <callable::TerminalStatesCompressed (
        NotAnAutomaton, int)>::value, "");

    static_assert (!Has <callable::ArcsOn (
        NotAnAutomaton, Forward, int)>::value, "");
    static_assert (!Has <callable::ArcsOn (
        NotAnAutomaton, Backward, int)>::value, "");
    static_assert (!Has <callable::ArcsOn (
        NotAnAutomaton, int, int)>::value, "");
}

BOOST_AUTO_TEST_CASE (test_times) {
    math::empty_sequence <char> empty;
    math::single_sequence <char> a ('a');
    math::single_sequence <char> b ('b');

    BOOST_CHECK_EQUAL (flipsta::times (forward, a, empty), a);
    BOOST_CHECK_EQUAL (flipsta::times (forward, empty, a), a);
    BOOST_CHECK_EQUAL (flipsta::times (backward, a, empty), a);
    BOOST_CHECK_EQUAL (flipsta::times (backward, empty, a), a);

    BOOST_CHECK_EQUAL (flipsta::times (forward, a, b),
        math::sequence <char> (std::string ("ab")));
    BOOST_CHECK_EQUAL (flipsta::times (backward, a, b),
        math::sequence <char> (std::string ("ba")));
}

BOOST_AUTO_TEST_SUITE_END()
