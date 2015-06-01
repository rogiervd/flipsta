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

#define BOOST_TEST_MODULE test_flipsta_acyclic_shortest_distance
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/shortest_distance.hpp"

#include "math/arithmetic_magma.hpp"

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

BOOST_AUTO_TEST_SUITE(test_suite_acyclic_shortest_distance)

// The example automata all use 'char' as the state type.
typedef char State;

template <class Distances, class Reference>
    void compare (Distances distances, Reference const & reference_)
{
    auto reference = range::view (reference_);

    while (!empty (reference)) {
        BOOST_CHECK (!empty (distances));
        if (empty (distances))
            return;

        auto d = chop_in_place (distances);
        auto r = chop_in_place (reference);
        BOOST_CHECK_EQUAL (first (d), first (r));
        BOOST_CHECK_EQUAL (second (d), second (r));
    }
    BOOST_CHECK (empty (distances));
}

BOOST_AUTO_TEST_CASE (testAcyclicShortestDistance) {
    auto automaton = acyclicExample();

    typedef math::cost <float> Cost;

    // Forward from 'c'.
    {
        std::vector <std::pair <State, Cost>> reference;
        reference.push_back (std::make_pair ('d', math::zero <Cost>()));
        reference.push_back (std::make_pair ('c', Cost (0)));
        reference.push_back (std::make_pair ('a', Cost (2)));
        reference.push_back (std::make_pair ('f', Cost (6)));
        reference.push_back (std::make_pair ('b', Cost (5)));
        reference.push_back (std::make_pair ('e', Cost (3)));

        compare (shortestDistanceAcyclicFrom (automaton, 'c', forward),
            reference);
    }

    // Forward from 'd'.
    {
        std::vector <std::pair <State, Cost>> reference;
        reference.push_back (std::make_pair ('d', Cost (0)));
        reference.push_back (std::make_pair ('c', Cost (5)));
        reference.push_back (std::make_pair ('a', Cost (3)));
        reference.push_back (std::make_pair ('f', Cost (10)));
        reference.push_back (std::make_pair ('b', Cost (7)));
        reference.push_back (std::make_pair ('e', Cost (5)));

        compare (shortestDistanceAcyclicFrom (automaton, 'd', forward),
            reference);
    }

    // Forward from 'd' with 0 and 'c' with 3.
    {
        std::vector <std::pair <char, Cost>> start;
        start.push_back (std::make_pair ('d', Cost (0)));
        start.push_back (std::make_pair ('c', Cost (3)));

        std::vector <std::pair <State, Cost>> reference;
        reference.push_back (std::make_pair ('d', Cost (0)));
        reference.push_back (std::make_pair ('c', Cost (3)));
        reference.push_back (std::make_pair ('a', Cost (3)));
        reference.push_back (std::make_pair ('f', Cost (9)));
        reference.push_back (std::make_pair ('b', Cost (7)));
        reference.push_back (std::make_pair ('e', Cost (5)));

        compare (shortestDistanceAcyclic (automaton, start, forward),
            reference);
    }

    /*
    Note that Cost is a commutative semiring, so the order of multiplication is
    not tested.
    For that, see test-shortest_distance-sequence.
    */

    // Backward from 'e'.
    {
        std::vector <std::pair <State, Cost>> reference;

        reference.push_back (std::make_pair ('e', Cost (0)));
        reference.push_back (std::make_pair ('b', Cost (-2)));
        reference.push_back (std::make_pair ('f', Cost (-3)));
        reference.push_back (std::make_pair ('a', Cost (2)));
        reference.push_back (std::make_pair ('c', Cost (3)));
        reference.push_back (std::make_pair ('d', Cost (5)));

        compare (shortestDistanceAcyclicFrom (automaton, 'e', backward),
            reference);
    }

    // Backward from 'b'.
    {
        std::vector <std::pair <State, Cost>> reference;

        reference.push_back (std::make_pair ('e', math::zero <Cost>()));
        reference.push_back (std::make_pair ('b', Cost (0)));
        reference.push_back (std::make_pair ('f', Cost (-1)));
        reference.push_back (std::make_pair ('a', Cost (4)));
        reference.push_back (std::make_pair ('c', Cost (5)));
        reference.push_back (std::make_pair ('d', Cost (7)));

        compare (shortestDistanceAcyclicFrom (automaton, 'b', backward),
            reference);
    }
}

BOOST_AUTO_TEST_SUITE_END()
