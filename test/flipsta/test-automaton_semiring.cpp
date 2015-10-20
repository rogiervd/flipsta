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

#define BOOST_TEST_MODULE test_automaton_semiring
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/automaton_semiring.hpp"

#include <vector>

#include "range/std/container.hpp"

#include "math/cost.hpp"
#include "math/check/report_check_magma_boost_test.hpp"

typedef math::cost <float> Cost;

typedef flipsta::AutomatonSemiring <char, Cost> AutomatonSemiring;
typedef flipsta::label::DefaultDescriptorFor <AutomatonSemiring>::type
    Descriptor;

BOOST_AUTO_TEST_SUITE(test_automaton_semiring)

BOOST_AUTO_TEST_CASE (spot) {
    Descriptor descriptor;

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

        AutomatonSemiring threeA (descriptor, Cost (3), 'a');
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

        AutomatonSemiring fiveB (descriptor, Cost (5), 'b');
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
    Descriptor descriptor;
    {
        std::vector <AutomatonSemiring> unequal_examples;

        unequal_examples.push_back (AutomatonSemiring (math::zero <Cost>()));
        unequal_examples.push_back (AutomatonSemiring (math::one <Cost>()));
        unequal_examples.push_back (AutomatonSemiring (Cost (5)));

        AutomatonSemiring s1 (descriptor, Cost (0), 'a');
        AutomatonSemiring s2 (descriptor, Cost (2), 'b');
        AutomatonSemiring s3 (descriptor, Cost (7), 'c');
        AutomatonSemiring s4 (descriptor, Cost (2), 'a');

        unequal_examples.push_back (s1);
        unequal_examples.push_back (s2);
        unequal_examples.push_back (s3);
        unequal_examples.push_back (s4);
        unequal_examples.push_back (s1 + s2 + s3);
        unequal_examples.push_back (s1 * s2);
        unequal_examples.push_back (s1 * s2 + s3);
        unequal_examples.push_back (s1 * s2 + s3 + s4);

        std::vector <AutomatonSemiring> examples = unequal_examples;

        examples.push_back (AutomatonSemiring());

        math::report_check_semiring <AutomatonSemiring, math::left> (
            math::times, math::plus, unequal_examples, examples);
        math::report_check_hash (examples);
    }
}

BOOST_AUTO_TEST_SUITE_END()
