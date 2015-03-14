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

#define BOOST_TEST_MODULE test_flipsta_label_product
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

BOOST_AUTO_TEST_SUITE (test_suite_flipsta_label_product)

BOOST_AUTO_TEST_CASE (testLabelProduct) {
    // Product with only one type, and one that does not change, at that.
    {
        typedef math::product <math::over <math::cost <double>>> product;
        typedef DefaultTagFor <product>::type Tag;

        static_assert (std::is_same <Tag,
                CompositeTag <DefaultTagFor <math::cost <double>>::type>
            >::value, "");

        product p (6.25f);
        Tag tag;

        auto internal = compress (tag, p);
        static_assert (std::is_same <decltype (internal), product>::value, "");
        BOOST_CHECK_EQUAL (first (internal.components()).value(), 6.25f);

        auto external = expand (tag, internal);
        static_assert (std::is_same <decltype (external), product>::value, "");
        BOOST_CHECK_EQUAL (first (external.components()).value(), 6.25f);
    }
    // Product with two types.
    {
        typedef math::product <math::over <float, math::sequence <char>>>
            product;
        typedef DefaultTagFor <product>::type Tag;

        static_assert (std::is_same <Tag,
            CompositeTag <
                DefaultTagFor <float>::type,
                DefaultTagFor <math::sequence <char>>::type
            >>::value, "");

        product p (1.5f, math::sequence <char> (std::string ("aba")));
        Tag tag;

        auto internal = compress (tag, p);
        BOOST_CHECK_EQUAL (first (internal.components()), 1.5f);
        auto internalSequence = second (internal.components());
        BOOST_CHECK_EQUAL (first (internalSequence.symbols()).id(), 0);
        BOOST_CHECK_EQUAL (second (internalSequence.symbols()).id(), 1);
        BOOST_CHECK_EQUAL (third (internalSequence.symbols()).id(), 0);

        auto external = expand (tag, internal);
        BOOST_CHECK_EQUAL (first (external.components()), 1.5f);
        auto externalSequence = second (external.components());
        BOOST_CHECK_EQUAL (first (externalSequence.symbols()), 'a');
        BOOST_CHECK_EQUAL (second (externalSequence.symbols()), 'b');
        BOOST_CHECK_EQUAL (third (externalSequence.symbols()), 'a');
    }
    // Product with two types: explicitly set the alphabet.
    {
        typedef math::product <math::over <float, math::sequence <char>>>
            product;
        typedef DefaultTagFor <product>::type Tag;

        auto alphabet = std::make_shared <math::alphabet <char>>();
        alphabet->add_symbol ('q');
        alphabet->add_symbol ('?');
        auto b = alphabet->add_symbol ('b');
        alphabet->add_symbol ('c');
        auto a = alphabet->add_symbol ('a');

        Tag tag = Tag (NoTag(), AlphabetTag <char> (alphabet));

        BOOST_CHECK (first (tag.tags()) == NoTag());
        BOOST_CHECK (second (tag.tags()).alphabet() == alphabet);

        BOOST_CHECK (first (tag.tags()) == NoTag());
        BOOST_CHECK (second (tag.tags()).alphabet() == alphabet);

        product p (1.5f, math::sequence <char> (std::string ("aba")));

        auto internal = compress (tag, p);
        BOOST_CHECK_EQUAL (first (internal.components()), 1.5f);
        auto internalSequence = second (internal.components());
        BOOST_CHECK_EQUAL (first (internalSequence.symbols()).id(), a.id());
        BOOST_CHECK_EQUAL (second (internalSequence.symbols()).id(), b.id());
        BOOST_CHECK_EQUAL (third (internalSequence.symbols()).id(), a.id());

        auto external = expand (tag, internal);
        BOOST_CHECK_EQUAL (first (external.components()), 1.5f);
        auto externalSequence = second (external.components());
        BOOST_CHECK_EQUAL (first (externalSequence.symbols()), 'a');
        BOOST_CHECK_EQUAL (second (externalSequence.symbols()), 'b');
        BOOST_CHECK_EQUAL (third (externalSequence.symbols()), 'a');
    }
}

BOOST_AUTO_TEST_SUITE_END()
