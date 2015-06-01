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

/** \file
Test the acyclic shortest distance algorithm on a left and right semiring:
sequences.
*/

#define BOOST_TEST_MODULE test_flipsta_shortest_distance_affix
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/shortest_distance.hpp"

#include "math/sequence.hpp"

#include "flipsta/automaton.hpp"

#include "example_automata.hpp"

using range::first;
using range::second;
using range::empty;
using range::chop_in_place;

using flipsta::forward;
using flipsta::backward;
using flipsta::AutomatonNotAcyclic;
using flipsta::shortestDistanceAcyclic;
using flipsta::shortestDistanceAcyclicFrom;

BOOST_AUTO_TEST_SUITE(test_suite_acyclic_shortest_distance_affix)

template <class Range> inline
    typename range::decayed_result_of <
        range::callable::chop_in_place (Range &)>::type
    getLast (Range && range)
{
    auto result = chop_in_place (range);
    while (!empty (range))
        result = chop_in_place (range);
    return std::move (result);
}

// Note that math::sequence <..., math::right> is a right semiring, and
// similar for math::left.
static_assert (math::is::semiring <math::right,
    math::callable::times, math::callable::plus,
    math::sequence <char, math::right>>::value, "");

static_assert (!math::is::semiring <math::left,
    math::callable::times, math::callable::plus,
    math::sequence <char, math::right>>::value, "");

BOOST_AUTO_TEST_CASE (testAcyclicShortestDistancePrefix) {
    auto automaton = prefixExample();

    static_assert (!flipsta::Has <
        flipsta::callable::ShortestDistanceAcyclicFrom (
            decltype (automaton), int, flipsta::Forward)>::value, "");
    static_assert (flipsta::Has <
        flipsta::callable::ShortestDistanceAcyclicFrom (
            decltype (automaton), int, flipsta::Backward)>::value, "");

    // From state 10, the common prefix is "a".
    {
        auto result = shortestDistanceAcyclicFrom (automaton, 10, backward);

        auto last = getLast (std::move (result));

        BOOST_CHECK_EQUAL (first (last), 1);
        math::sequence <char, math::left> referencePrefix (std::string ("a"));
        BOOST_CHECK_EQUAL (second (last), referencePrefix);
    }
    // From state 3, the common prefix is "ab".
    {
        auto result = shortestDistanceAcyclicFrom (automaton, 3, backward);

        auto last = getLast (std::move (result));

        BOOST_CHECK_EQUAL (first (last), 1);
        math::sequence <char, math::left> referencePrefix (std::string ("ab"));
        BOOST_CHECK_EQUAL (second (last), referencePrefix);
    }
}

BOOST_AUTO_TEST_CASE (testAcyclicShortestDistanceSuffix) {
    auto automaton = suffixExample();

    static_assert (flipsta::Has <
        flipsta::callable::ShortestDistanceAcyclicFrom (
            decltype (automaton), int, flipsta::Forward)>::value, "");
    static_assert (!flipsta::Has <
        flipsta::callable::ShortestDistanceAcyclicFrom (
            decltype (automaton), int, flipsta::Backward)>::value, "");

    // From state 1, the common prefix is "yz".
    {
        auto result = shortestDistanceAcyclicFrom (automaton, 1, forward);
        auto last = getLast (std::move (result));

        BOOST_CHECK_EQUAL (first (last), 10);
        math::sequence <char, math::right> referenceSuffix (std::string ("yz"));
        BOOST_CHECK_EQUAL (second (last), referenceSuffix);
    }
    // From state 7, the common prefix is "z".
    {
        auto result = shortestDistanceAcyclicFrom (automaton, 7, forward);
        auto last = getLast (std::move (result));

        BOOST_CHECK_EQUAL (first (last), 10);
        math::sequence <char, math::right> referenceSuffix (std::string ("z"));
        BOOST_CHECK_EQUAL (second (last), referenceSuffix);
    }
}

BOOST_AUTO_TEST_SUITE_END()
