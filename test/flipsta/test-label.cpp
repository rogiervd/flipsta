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

#define BOOST_TEST_MODULE test_flipsta_label
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

template <class ...> struct ShowTypes;

BOOST_AUTO_TEST_SUITE(flipsta_label_test_suite)

// In the simple case, the label does not change, and the descriptor returns
// the same value.
BOOST_AUTO_TEST_CASE (testLabelSimple) {
    typedef math::cost <float> Cost;
    Cost c (4.5f);

    typedef DefaultDescriptorFor <Cost>::type Descriptor;
    Descriptor descriptor;
    CompressedLabelType <Descriptor, Cost>::type internal
        = compress (descriptor, c);

    static_assert (std::is_same <decltype (internal), Cost>::value, "");
    static_assert (std::is_same <
        ExpandedLabelType <Descriptor, Cost>::type, Cost>::value, "");

    BOOST_CHECK_EQUAL (c.value(), internal.value());

    math::cost <float> external = expand (descriptor, internal);
    BOOST_CHECK_EQUAL (external.value(), c.value());
}

/**
Check labels that are sequences.
Test the conversion in both ways.
The arguments a, b, and c give the internal representations of 'a', 'b', and
'c'.
*/
template <class Descriptor>
    void checkSequenceLabels (Descriptor & descriptor, int a, int b, int c)
{
    typedef math::sequence <char> Sequence;
    typedef math::empty_sequence <char> EmptySequence;
    typedef math::single_sequence <char> SingleSequence;
    typedef math::optional_sequence <char> OptionalSequence;
    typedef math::sequence_annihilator <char> SequenceAnnihilator;

    static_assert (std::is_same <
        DefaultDescriptorFor <Sequence>::type, Descriptor>::value, "");
    static_assert (std::is_same <
        DefaultDescriptorFor <EmptySequence>::type, Descriptor>::value, "");
    static_assert (std::is_same <
        DefaultDescriptorFor <SingleSequence>::type, Descriptor>::value, "");
    static_assert (std::is_same <
        DefaultDescriptorFor <OptionalSequence>::type, Descriptor>::value, "");
    static_assert (std::is_same <
        DefaultDescriptorFor <SequenceAnnihilator>::type, Descriptor>::value,
        "");

    typedef typename CompressedLabelType <Descriptor, Sequence>::type
        CompressedSequence;

    typedef typename Descriptor::Alphabet::dense_symbol_type CompressedSymbol;

    static_assert (std::is_same <
        CompressedSequence, math::sequence <CompressedSymbol>
        >::value, "");

    typedef typename ExpandedLabelType <Descriptor, CompressedSequence>::type
        SequenceAgain;
    static_assert (std::is_same <SequenceAgain, Sequence>::value, "");

    // Empty.
    {
        EmptySequence emptySequence;
        auto internalEmptySequence = compress (descriptor, emptySequence);
        BOOST_CHECK_EQUAL (size (internalEmptySequence.symbols()), 0);
    }

    // General sequence.
    {
        Sequence sequence (std::string ("ab"));
        auto internal = compress (descriptor, sequence);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 2);
        BOOST_CHECK_EQUAL (first (internal.symbols()).id(), a);
        BOOST_CHECK_EQUAL (second (internal.symbols()).id(), b);

        auto external = expand (descriptor, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 2);
        BOOST_CHECK_EQUAL (first (external.symbols()), 'a');
        BOOST_CHECK_EQUAL (second (external.symbols()), 'b');
    }
    {
        Sequence empty;
        auto internal = compress (descriptor, empty);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 0);

        auto external = expand (descriptor, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 0);
    }
    {
        Sequence annihilator = SequenceAnnihilator();
        auto internal = compress (descriptor, annihilator);
        BOOST_CHECK (math::is_annihilator <math::callable::times> (internal));

        auto external = expand (descriptor, internal);
        BOOST_CHECK (external.is_annihilator());
    }

    // Single.
    {
        SingleSequence b1 ('b');
        auto internal = compress (descriptor, b1);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 1);
        BOOST_CHECK_EQUAL (first (internal.symbols()).id(), b);

        auto external = expand (descriptor, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 1);
        BOOST_CHECK_EQUAL (first (external.symbols()), 'b');
    }
    {
        SingleSequence c1 ('c');
        auto internal = compress (descriptor, c1);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 1);
        BOOST_CHECK_EQUAL (first (internal.symbols()).id(), c);

        auto external = expand (descriptor, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 1);
        BOOST_CHECK_EQUAL (first (external.symbols()), 'c');
    }

    // Optional.
    {
        OptionalSequence empty01;
        auto internal = compress (descriptor, empty01);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 0);

        auto external = expand (descriptor, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 0);
    }
    {
        OptionalSequence a01 ('a');
        auto internal = compress (descriptor, a01);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 1);
        BOOST_CHECK_EQUAL (first (internal.symbols()).id(), a);

        auto external = expand (descriptor, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 1);
        BOOST_CHECK_EQUAL (first (external.symbols()), 'a');
    }
    {
        OptionalSequence c01 ('c');
        auto internal = compress (descriptor, c01);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 1);
        BOOST_CHECK_EQUAL (first (internal.symbols()).id(), c);

        auto external = expand (descriptor, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 1);
        BOOST_CHECK_EQUAL (first (external.symbols()), 'c');
    }

    // Annihilator.
    {
        SequenceAnnihilator annihilator;
        auto internal = compress (descriptor, annihilator);
        BOOST_CHECK (math::is_annihilator <math::callable::times> (internal));

        auto external = expand (descriptor, internal);
        BOOST_CHECK (math::is_annihilator <math::callable::times> (
            external));
    }
}

BOOST_AUTO_TEST_CASE (testLabelSequence) {
    typedef DefaultDescriptorFor <math::sequence <char>>::type Descriptor;
    // Default-construct descriptor.
    {
        Descriptor descriptor;
        checkSequenceLabels (descriptor, 0, 1, 2);

        Descriptor descriptor2;
        BOOST_CHECK (!(descriptor == descriptor2));
    }
    // Pass in an alphabet explicitly.
    {
        auto alphabet = std::make_shared <math::alphabet <char>>();
        alphabet->add_symbol ('q');
        alphabet->add_symbol ('b');
        alphabet->add_symbol ('1');
        alphabet->add_symbol ('c');
        alphabet->add_symbol ('a');

        Descriptor descriptor (alphabet);
        checkSequenceLabels (descriptor, 4, 1, 3);

        Descriptor descriptor2 (alphabet);
        BOOST_CHECK (descriptor == descriptor2);
    }
}
BOOST_AUTO_TEST_SUITE_END()
