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
