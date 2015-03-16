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

#define BOOST_TEST_MODULE test_automaton_semiring
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/automaton_semiring.hpp"

#include <vector>

#include "range/std/container.hpp"

#include "math/cost.hpp"
#include "math/check/check_magma.hpp"
#include "math/check/check_hash.hpp"

typedef math::cost <float> Cost;

typedef flipsta::AutomatonSemiring <char, Cost> AutomatonSemiring;
typedef flipsta::label::DefaultTagFor <AutomatonSemiring>::type LabelTag;

BOOST_AUTO_TEST_SUITE(test_automaton_semiring)

BOOST_AUTO_TEST_CASE (spot) {
    LabelTag labelTag;

    {
        AutomatonSemiring zero;
        BOOST_CHECK (zero.automaton().isNull());
        BOOST_CHECK (math::zero <AutomatonSemiring>() == zero);

        AutomatonSemiring one = math::one <AutomatonSemiring>();
        BOOST_CHECK (!one.automaton().isNull());
        BOOST_CHECK (one.automaton().startWeight() == Cost (0));
        BOOST_CHECK (one.automaton().state()->arcs().size() == 0);

        AutomatonSemiring six (Cost (6));
        BOOST_CHECK (!six.automaton().isNull());
        BOOST_CHECK (six.automaton().startWeight() == Cost (6));
        BOOST_CHECK (six.automaton().state()->arcs().size() == 0);

        {
            AutomatonSemiring alsoZero = six * zero;
            BOOST_CHECK (zero.automaton().isNull());
            BOOST_CHECK (math::zero <AutomatonSemiring>() == alsoZero);
        }
        {
            AutomatonSemiring alsoZero = zero * six;
            BOOST_CHECK (zero.automaton().isNull());
            BOOST_CHECK (math::zero <AutomatonSemiring>() == alsoZero);
        }

        auto finalState = six.automaton().state();

        AutomatonSemiring threeA (labelTag, Cost (3), 'a');
        BOOST_CHECK (!threeA.automaton().isNull());
        BOOST_CHECK_EQUAL (threeA.automaton().startWeight().value(), 3);
        BOOST_CHECK_EQUAL (threeA.automaton().state()->arcs().size(), 1);
        BOOST_CHECK_EQUAL (threeA.automaton().state()->arcs().begin()->first,
            'a');
        BOOST_CHECK (threeA.automaton().state()->arcs().begin()->second.state()
            == finalState);

        AutomatonSemiring nineA = six * threeA;
        BOOST_CHECK (!nineA.automaton().isNull());
        BOOST_CHECK_EQUAL (nineA.automaton().startWeight().value(), 9);
        BOOST_CHECK_EQUAL (nineA.automaton().state()->arcs().size(), 1);
        BOOST_CHECK_EQUAL (nineA.automaton().state()->arcs().begin()->first,
            'a');
        BOOST_CHECK (nineA.automaton().state()->arcs().begin()->second.state()
            == finalState);

        AutomatonSemiring alsoNineA = threeA * six;
        BOOST_CHECK (!alsoNineA.automaton().isNull());
        BOOST_CHECK_EQUAL (alsoNineA.automaton().startWeight().value(), 9);
        BOOST_CHECK_EQUAL (alsoNineA.automaton().state()->arcs().size(), 1);
        BOOST_CHECK_EQUAL (alsoNineA.automaton().state()->arcs().begin()->first,
            'a');
        BOOST_CHECK (
            alsoNineA.automaton().state()->arcs().begin()->second.state()
            == finalState);

        AutomatonSemiring fiveB (labelTag, Cost (5), 'b');
        BOOST_CHECK (!fiveB.automaton().isNull());
        BOOST_CHECK_EQUAL (fiveB.automaton().startWeight().value(), 5);
        BOOST_CHECK_EQUAL (fiveB.automaton().state()->arcs().size(), 1);
        BOOST_CHECK_EQUAL (fiveB.automaton().state()->arcs().begin()->first,
            'b');
        BOOST_CHECK (fiveB.automaton().state()->arcs().begin()->second.state()
            == finalState);

        AutomatonSemiring eightAB = threeA * fiveB;
        BOOST_CHECK (!eightAB.automaton().isNull());
        BOOST_CHECK_EQUAL (eightAB.automaton().startWeight().value(), 8);
        BOOST_CHECK_EQUAL (eightAB.automaton().state()->arcs().size(), 1);
        BOOST_CHECK_EQUAL (eightAB.automaton().state()->arcs().begin()->first,
            'a');
        BOOST_CHECK (eightAB.automaton().state()->arcs().begin()->second.state()
            == fiveB.automaton().state());

        AutomatonSemiring threeAfiveB = threeA + fiveB;
        BOOST_CHECK (!threeAfiveB.automaton().isNull());
        BOOST_CHECK_EQUAL (threeAfiveB.automaton().startWeight().value(), 3);
        BOOST_CHECK_EQUAL (threeAfiveB.automaton().state()->arcs().size(), 2);
        BOOST_CHECK_EQUAL (threeAfiveB.automaton().state()->arcs().find ('a')
            ->second.startWeight().value(), 0);
        BOOST_CHECK_EQUAL (threeAfiveB.automaton().state()->arcs().find ('a')
            ->second.state(), finalState);
        BOOST_CHECK_EQUAL (threeAfiveB.automaton().state()->arcs().find ('b')
            ->second.startWeight().value(), 2);
        BOOST_CHECK_EQUAL (threeAfiveB.automaton().state()->arcs().find ('b')
            ->second.state(), finalState);
    }
}

BOOST_AUTO_TEST_CASE (all) {
    LabelTag labelTag;
    {
        std::vector <AutomatonSemiring> examples;

        examples.push_back (AutomatonSemiring());
        examples.push_back (AutomatonSemiring (math::zero <Cost>()));
        examples.push_back (AutomatonSemiring (math::one <Cost>()));
        examples.push_back (AutomatonSemiring (Cost (5)));

        AutomatonSemiring s1 (labelTag, Cost (0), 'a');
        AutomatonSemiring s2 (labelTag, Cost (2), 'b');
        AutomatonSemiring s3 (labelTag, Cost (7), 'c');
        AutomatonSemiring s4 (labelTag, Cost (2), 'a');

        examples.push_back (s1);
        examples.push_back (s2);
        examples.push_back (s3);
        examples.push_back (s4);
        examples.push_back (s1 + s2 + s3);
        examples.push_back (s1 * s2);
        examples.push_back (s1 * s2 + s3);
        examples.push_back (s1 * s2 + s3 + s4);

        math::check_semiring <AutomatonSemiring, math::left> (
            math::times, math::plus, examples);
        math::check_hash (examples);
    }
}

BOOST_AUTO_TEST_SUITE_END()
