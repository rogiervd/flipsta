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

#define BOOST_TEST_MODULE test_flipsta_shortest_distance_sequence
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

BOOST_AUTO_TEST_SUITE(test_suite_acyclic_shortest_distance_sequence)

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
    auto automaton = acyclicSequenceExample();

    typedef math::cost <float> Cost;
    typedef math::sequence <char> Sequence;
    typedef math::lexicographical <math::over <Cost, Sequence>> Lexicographical;

    // Forward from 'c'.
    {
        std::vector <std::pair <State, Lexicographical>> reference;
        reference.push_back (std::make_pair ('d',
            math::zero <Lexicographical>()));
        reference.push_back (std::make_pair ('c',
            Lexicographical (Cost (0), Sequence())));
        reference.push_back (std::make_pair ('a',
            Lexicographical (Cost (2), Sequence (std::string ("j")))));
        reference.push_back (std::make_pair ('f',
            Lexicographical (Cost (6), Sequence (std::string ("k")))));
        reference.push_back (std::make_pair ('b',
            Lexicographical (Cost (5), Sequence (std::string ("ko")))));
        reference.push_back (std::make_pair ('e',
            Lexicographical (Cost (3), Sequence (std::string ("kop")))));

        compare (shortestDistanceAcyclicFrom (automaton, 'c', forward),
            reference);
    }

    // Forward from 'd'.
    {
        std::vector <std::pair <State, Lexicographical>> reference;
        reference.push_back (std::make_pair ('d',
            Lexicographical (Cost (0), Sequence())));
        reference.push_back (std::make_pair ('c',
            Lexicographical (Cost (5), Sequence (std::string ("h")))));
        reference.push_back (std::make_pair ('a',
            Lexicographical (Cost (3), Sequence (std::string ("i")))));
        reference.push_back (std::make_pair ('f',
            Lexicographical (Cost (10), Sequence (std::string ("il")))));
        reference.push_back (std::make_pair ('b',
            Lexicographical (Cost (7), Sequence (std::string ("im")))));
        reference.push_back (std::make_pair ('e',
            Lexicographical (Cost (5), Sequence (std::string ("imp")))));

        // Without explicit result type.
        compare (shortestDistanceAcyclicFrom (automaton, 'd', forward),
            reference);
    }

    // Backward from 'e'.
    {
        std::vector <std::pair <State, Lexicographical>> reference;

        reference.push_back (std::make_pair ('e',
            Lexicographical (Cost (0), Sequence())));
        reference.push_back (std::make_pair ('b',
            Lexicographical (Cost (-2), Sequence (std::string ("p")))));
        reference.push_back (std::make_pair ('f',
            Lexicographical (Cost (-3), Sequence (std::string ("op")))));
        reference.push_back (std::make_pair ('a',
            Lexicographical (Cost (2), Sequence (std::string ("mp")))));
        reference.push_back (std::make_pair ('c',
            Lexicographical (Cost (3), Sequence (std::string ("kop")))));
        reference.push_back (std::make_pair ('d',
            Lexicographical (Cost (5), Sequence (std::string ("imp")))));

        compare (shortestDistanceAcyclicFrom (automaton, 'e', backward),
            reference);
    }

    // Backward from 'b'.
    {
        std::vector <std::pair <State, Lexicographical>> reference;

        reference.push_back (std::make_pair ('e',
            math::zero <Lexicographical>()));
        reference.push_back (std::make_pair ('b',
            Lexicographical (Cost (0), Sequence (std::string ("")))));
        reference.push_back (std::make_pair ('f',
            Lexicographical (Cost (-1), Sequence (std::string ("o")))));
        reference.push_back (std::make_pair ('a',
            Lexicographical (Cost (4), Sequence (std::string ("m")))));
        reference.push_back (std::make_pair ('c',
            Lexicographical (Cost (5), Sequence (std::string ("ko")))));
        reference.push_back (std::make_pair ('d',
            Lexicographical (Cost (7), Sequence (std::string ("im")))));

        compare (shortestDistanceAcyclicFrom (automaton, 'b', backward),
            reference);
    }
}

BOOST_AUTO_TEST_SUITE_END()
