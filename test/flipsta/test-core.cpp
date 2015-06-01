/*
Copyright 2014 Rogier van Dalen.

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
    static_assert (!Has <callable::Descriptor (NotAnAutomaton)>::value, "");

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
