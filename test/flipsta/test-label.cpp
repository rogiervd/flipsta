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

// In the simple case, the label does not change, and the tag returns the same
// value.
BOOST_AUTO_TEST_CASE (testLabelSimple) {
    typedef math::cost <float> Cost;
    Cost c (4.5f);

    typedef DefaultTagFor <Cost>::type Tag;
    Tag tag;
    CompressedLabelType <Tag, Cost>::type internal = compress (tag, c);

    static_assert (std::is_same <decltype (internal), Cost>::value, "");
    static_assert (std::is_same <
        ExpandedLabelType <Tag, Cost>::type, Cost>::value, "");

    BOOST_CHECK_EQUAL (c.value(), internal.value());

    math::cost <float> external = expand (tag, internal);
    BOOST_CHECK_EQUAL (external.value(), c.value());
}

/**
Check labels that are sequences.
Test the conversion in both ways.
The arguments a, b, and c give the internal representations of 'a', 'b', and
'c'.
*/
template <class Tag> void checkSequenceLabels (Tag & tag, int a, int b, int c) {
    typedef math::sequence <char> Sequence;
    typedef math::empty_sequence <char> EmptySequence;
    typedef math::single_sequence <char> SingleSequence;
    typedef math::optional_sequence <char> OptionalSequence;
    typedef math::sequence_annihilator <char> SequenceAnnihilator;

    static_assert (std::is_same <
        DefaultTagFor <Sequence>::type, Tag>::value, "");
    static_assert (std::is_same <
        DefaultTagFor <EmptySequence>::type, Tag>::value, "");
    static_assert (std::is_same <
        DefaultTagFor <SingleSequence>::type, Tag>::value, "");
    static_assert (std::is_same <
        DefaultTagFor <OptionalSequence>::type, Tag>::value, "");
    static_assert (std::is_same <
        DefaultTagFor <SequenceAnnihilator>::type, Tag>::value, "");

    typedef typename CompressedLabelType <Tag, Sequence>::type
        CompressedSequence;

    typedef typename Tag::Alphabet::dense_symbol_type CompressedSymbol;

    static_assert (std::is_same <
        CompressedSequence, math::sequence <CompressedSymbol>
        >::value, "");

    typedef typename ExpandedLabelType <Tag, CompressedSequence>::type
        SequenceAgain;
    static_assert (std::is_same <SequenceAgain, Sequence>::value, "");

    // Empty.
    {
        EmptySequence emptySequence;
        auto internalEmptySequence = compress (tag, emptySequence);
        BOOST_CHECK_EQUAL (size (internalEmptySequence.symbols()), 0);
    }

    // General sequence.
    {
        Sequence sequence (std::string ("ab"));
        auto internal = compress (tag, sequence);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 2);
        BOOST_CHECK_EQUAL (first (internal.symbols()).id(), a);
        BOOST_CHECK_EQUAL (second (internal.symbols()).id(), b);

        auto external = expand (tag, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 2);
        BOOST_CHECK_EQUAL (first (external.symbols()), 'a');
        BOOST_CHECK_EQUAL (second (external.symbols()), 'b');
    }
    {
        Sequence empty;
        auto internal = compress (tag, empty);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 0);

        auto external = expand (tag, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 0);
    }
    {
        Sequence annihilator = SequenceAnnihilator();
        auto internal = compress (tag, annihilator);
        BOOST_CHECK (math::is_annihilator <math::callable::times> (internal));

        auto external = expand (tag, internal);
        BOOST_CHECK (external.is_annihilator());
    }

    // Single.
    {
        SingleSequence b1 ('b');
        auto internal = compress (tag, b1);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 1);
        BOOST_CHECK_EQUAL (first (internal.symbols()).id(), b);

        auto external = expand (tag, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 1);
        BOOST_CHECK_EQUAL (first (external.symbols()), 'b');
    }
    {
        SingleSequence c1 ('c');
        auto internal = compress (tag, c1);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 1);
        BOOST_CHECK_EQUAL (first (internal.symbols()).id(), c);

        auto external = expand (tag, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 1);
        BOOST_CHECK_EQUAL (first (external.symbols()), 'c');
    }

    // Optional.
    {
        OptionalSequence empty01;
        auto internal = compress (tag, empty01);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 0);

        auto external = expand (tag, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 0);
    }
    {
        OptionalSequence a01 ('a');
        auto internal = compress (tag, a01);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 1);
        BOOST_CHECK_EQUAL (first (internal.symbols()).id(), a);

        auto external = expand (tag, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 1);
        BOOST_CHECK_EQUAL (first (external.symbols()), 'a');
    }
    {
        OptionalSequence c01 ('c');
        auto internal = compress (tag, c01);
        BOOST_CHECK_EQUAL (size (internal.symbols()), 1);
        BOOST_CHECK_EQUAL (first (internal.symbols()).id(), c);

        auto external = expand (tag, internal);
        BOOST_CHECK_EQUAL (size (external.symbols()), 1);
        BOOST_CHECK_EQUAL (first (external.symbols()), 'c');
    }

    // Annihilator.
    {
        SequenceAnnihilator annihilator;
        auto internal = compress (tag, annihilator);
        BOOST_CHECK (math::is_annihilator <math::callable::times> (internal));

        auto external = expand (tag, internal);
        BOOST_CHECK (math::is_annihilator <math::callable::times> (
            external));
    }
}

BOOST_AUTO_TEST_CASE (testLabelSequence) {
    typedef DefaultTagFor <math::sequence <char>>::type Tag;
    // Default-construct tag.
    {
        Tag tag;
        checkSequenceLabels (tag, 0, 1, 2);

        Tag tag2;
        BOOST_CHECK (!(tag == tag2));
    }
    // Pass in an alphabet explicitly.
    {
        auto alphabet = std::make_shared <math::alphabet <char>>();
        alphabet->add_symbol ('q');
        alphabet->add_symbol ('b');
        alphabet->add_symbol ('1');
        alphabet->add_symbol ('c');
        alphabet->add_symbol ('a');

        Tag tag (alphabet);
        checkSequenceLabels (tag, 4, 1, 3);

        Tag tag2 (alphabet);
        BOOST_CHECK (tag == tag2);
    }
}
BOOST_AUTO_TEST_SUITE_END()
