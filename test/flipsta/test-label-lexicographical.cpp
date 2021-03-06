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

#define BOOST_TEST_MODULE test_flipsta_label_lexicographical
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/label.hpp"

#include <string>

#include "range/std/container.hpp"

#include "math/cost.hpp"
#include "math/arithmetic_magma.hpp"

using range::size;
using range::first;
using range::second;
using range::third;

using namespace flipsta::label;

BOOST_AUTO_TEST_SUITE(test_suite_flipsta_label_lexicographical)

BOOST_AUTO_TEST_CASE (testLabelLexicographical) {
    // Lexicographical with only one type, and one that does not change, at
    // that.
    {
        typedef math::lexicographical <math::over <math::cost <double>>>
            Lexicographical;
        typedef DefaultDescriptorFor <Lexicographical>::type Descriptor;

        static_assert (std::is_same <Descriptor,
                CompositeDescriptor <
                    DefaultDescriptorFor <math::cost <double>>::type>
            >::value, "");

        Lexicographical p (6.25f);
        Descriptor descriptor;

        auto internal = compress (descriptor, p);
        static_assert (
            std::is_same <decltype (internal), Lexicographical>::value, "");
        BOOST_CHECK_EQUAL (first (internal.components()).value(), 6.25f);

        auto external = expand (descriptor, internal);
        static_assert (
            std::is_same <decltype (external), Lexicographical>::value, "");
        BOOST_CHECK_EQUAL (first (external.components()).value(), 6.25f);
    }
    // Lexicographical with two types.
    {
        typedef math::lexicographical <math::over <
                math::cost <float>, math::sequence <char>>>
            Lexicographical;
        typedef DefaultDescriptorFor <Lexicographical>::type Descriptor;

        static_assert (std::is_same <Descriptor,
            CompositeDescriptor <
                DefaultDescriptorFor <math::cost <float>>::type,
                DefaultDescriptorFor <math::sequence <char>>::type
            >>::value, "");

        Lexicographical p (1.5f, math::sequence <char> (std::string ("aba")));
        Descriptor descriptor;

        auto internal = compress (descriptor, p);
        BOOST_CHECK_EQUAL (first (internal.components()).value(), 1.5f);
        auto internalSequence = second (internal.components());
        BOOST_CHECK_EQUAL (first (internalSequence.symbols()).id(), 0);
        BOOST_CHECK_EQUAL (second (internalSequence.symbols()).id(), 1);
        BOOST_CHECK_EQUAL (third (internalSequence.symbols()).id(), 0);

        auto external = expand (descriptor, internal);
        BOOST_CHECK_EQUAL (first (external.components()).value(), 1.5f);
        auto externalSequence = second (external.components());
        BOOST_CHECK_EQUAL (first (externalSequence.symbols()), 'a');
        BOOST_CHECK_EQUAL (second (externalSequence.symbols()), 'b');
        BOOST_CHECK_EQUAL (third (externalSequence.symbols()), 'a');
    }
    // Lexicographical with two types: explicitly set the alphabet.
    {
        typedef math::lexicographical <math::over <
            math::cost <float>, math::sequence <char>>> Lexicographical;
        typedef DefaultDescriptorFor <Lexicographical>::type Descriptor;

        auto alphabet = std::make_shared <math::alphabet <char>>();
        alphabet->add_symbol ('q');
        alphabet->add_symbol ('?');
        auto b = alphabet->add_symbol ('b');
        alphabet->add_symbol ('c');
        auto a = alphabet->add_symbol ('a');

        Descriptor descriptor = Descriptor (
            NoDescriptor(), AlphabetDescriptor <char> (alphabet));

        BOOST_CHECK (first (descriptor.components()) == NoDescriptor());
        BOOST_CHECK (second (descriptor.components()).alphabet() == alphabet);

        Lexicographical p (1.5f, math::sequence <char> (std::string ("aba")));

        auto internal = compress (descriptor, p);
        BOOST_CHECK_EQUAL (first (internal.components()).value(), 1.5f);
        auto internalSequence = second (internal.components());
        BOOST_CHECK_EQUAL (first (internalSequence.symbols()).id(), a.id());
        BOOST_CHECK_EQUAL (second (internalSequence.symbols()).id(), b.id());
        BOOST_CHECK_EQUAL (third (internalSequence.symbols()).id(), a.id());

        auto external = expand (descriptor, internal);
        BOOST_CHECK_EQUAL (first (external.components()).value(), 1.5f);
        auto externalSequence = second (external.components());
        BOOST_CHECK_EQUAL (first (externalSequence.symbols()), 'a');
        BOOST_CHECK_EQUAL (second (externalSequence.symbols()), 'b');
        BOOST_CHECK_EQUAL (third (externalSequence.symbols()), 'a');
    }
}

BOOST_AUTO_TEST_SUITE_END()
