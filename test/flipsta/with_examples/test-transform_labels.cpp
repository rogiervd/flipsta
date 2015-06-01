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

#define BOOST_TEST_MODULE test_flipsta_transformLabels
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/transform_labels.hpp"

#include "range/core.hpp"
#include "range/walk_size.hpp"

#include "example_automata.hpp"

using range::empty;
using range::size;
using range::first;
using range::second;

using flipsta::forward;
using flipsta::backward;

/// Simple function to project labels onto their nth component.
template <std::size_t N> struct Project {
    template <class Composite>
        auto operator() (Composite const & composite) const
    -> typename std::decay <
        decltype (range::at_c <N> (composite.components()))>::type
    { return range::at_c <N> (composite.components()); }
};

/// Simple function to square labels.
/// (I cannot think of a reason for this in practice.)
struct Square {
    template <class Label> auto operator() (Label const & label) const
    -> typename std::decay <decltype (label * label)>::type
    { return label * label; }
};

/// Bump character labels to the next one.
/// This is only to test that external labels can be transformed.
struct BumpChar {
    math::empty_sequence <char>
        operator() (math::empty_sequence <char> const & e) const
    { return e; }

    math::single_sequence <char>
        operator() (math::single_sequence <char> const & s) const
    { return math::single_sequence <char> (char (s.symbol() + 1)); }

    math::sequence <char>
        operator() (math::sequence <char> const & s) const
    {
        if (s.is_annihilator())
            return math::sequence_annihilator <char>();
        assert (size (s.symbols()) <= 1);
        if (empty (s.symbols()))
            return math::sequence <char>();
        else
            return math::single_sequence <char> (
                char (first (s.symbols()) + 1));
    }
};

BOOST_AUTO_TEST_SUITE(test_suite_topological_order)

BOOST_AUTO_TEST_CASE (testProjectLabels) {
    // This example has a lexicographical label; we extract the cost.
    auto example = acyclicSequenceExample();

    typedef math::cost <float> Cost;

    auto transformed = flipsta::transformLabels (
        example, Project <0>(),
        range::first (example.descriptor().components()));

    static_assert (std::is_same <
        flipsta::LabelType <decltype (transformed)>::type, Cost>::value, "");

    BOOST_CHECK_EQUAL (range::walk_size (flipsta::states (transformed)), 6);

    BOOST_CHECK (flipsta::hasState (transformed, 'c'));

    RANGE_FOR_EACH (arc, flipsta::arcsOn (transformed, forward, 'f')) {
        if (arc.state (forward) == 'b') {
            BOOST_CHECK_EQUAL (arc.label().value(), -1);
        } else {
            BOOST_CHECK_EQUAL (arc.state (forward), 'e');
            BOOST_CHECK_EQUAL (arc.label().value(), 1);
        }
    }

    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, forward, 'd'), Cost (0));
    // 'e' is a final state, not a start state.
    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, forward, 'e'),
        math::zero <Cost>());

    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, backward, 'e'), Cost (1));

    auto finalStates = flipsta::terminalStates (transformed, backward);
    BOOST_CHECK_EQUAL (range::walk_size (finalStates), 1);
    BOOST_CHECK_EQUAL (first (first (finalStates)), 'e');
    BOOST_CHECK_EQUAL (second (first (finalStates)), Cost (1));
}

BOOST_AUTO_TEST_CASE (testSquareCost) {
    // This example has a lexicographical label; we extract the cost.
    auto example = acyclicExample();

    typedef math::cost <float> Cost;

    auto transformed = flipsta::transformLabels (
        example, Square(), example.descriptor());

    static_assert (std::is_same <
        flipsta::LabelType <decltype (transformed)>::type, Cost>::value, "");

    BOOST_CHECK_EQUAL (range::walk_size (flipsta::states (transformed)), 6);

    BOOST_CHECK (flipsta::hasState (transformed, 'c'));

    RANGE_FOR_EACH (arc, flipsta::arcsOn (transformed, backward, 'b')) {
        if (arc.state (backward) == 'f') {
            BOOST_CHECK_EQUAL (arc.label().value(), -2);
        } else {
            BOOST_CHECK_EQUAL (arc.state (backward), 'a');
            BOOST_CHECK_EQUAL (arc.label().value(), 8);
        }
    }

    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, forward, 'd'), Cost (0));
    // 'e' is a final state, not a start state.
    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, forward, 'e'),
        math::zero <Cost>());

    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, backward, 'e'), Cost (2));

    auto finalStates = flipsta::terminalStates (transformed, backward);
    BOOST_CHECK_EQUAL (range::walk_size (finalStates), 1);
    BOOST_CHECK_EQUAL (first (first (finalStates)), 'e');
    BOOST_CHECK_EQUAL (second (first (finalStates)), Cost (2));
}

BOOST_AUTO_TEST_CASE (testSquareSequence) {
    // This example has a sequence label, so that it is possible to test types.
    auto example = prefixExample();

    typedef math::empty_sequence <char> EmptySequence;
    typedef math::single_sequence <char> SingleSequence;
    typedef math::sequence <char> Sequence;

    auto transformed = flipsta::transformLabels (
        example, Square(), example.descriptor());

    typedef decltype (transformed) TransformedAutomaton;


    // A squared single sequence contains two elements and is a Sequence.
    static_assert (std::is_same <
        flipsta::LabelType <decltype (transformed)>::type, Sequence>::value,
        "");
    // The empty sequence remains empty (as it should).
    static_assert (std::is_same <
        TransformedAutomaton::TerminalLabel, EmptySequence>::value, "");

    BOOST_CHECK_EQUAL (range::walk_size (flipsta::states (transformed)), 8);

    BOOST_CHECK (flipsta::hasState (transformed, 10));
    BOOST_CHECK (!flipsta::hasState (transformed, 9));

    RANGE_FOR_EACH (arc, flipsta::arcsOn (transformed, forward, 4)) {
        BOOST_CHECK_EQUAL (size (arc.label().symbols()), 2);
        if (arc.state (forward) == 6) {
            BOOST_CHECK_EQUAL (first (arc.label().symbols()), 'c');
            BOOST_CHECK_EQUAL (second (arc.label().symbols()), 'c');
        } else {
            BOOST_CHECK_EQUAL (arc.state (forward), 3);

            BOOST_CHECK_EQUAL (first (arc.label().symbols()), 'b');
            BOOST_CHECK_EQUAL (second (arc.label().symbols()), 'b');
        }
    }

    // The terminal label must be able to represent zero and one, and is
    // therefore returned as type Sequence.
    static_assert (std::is_same <
        decltype (flipsta::terminalLabel (transformed, forward, 1)),
        Sequence>::value, "");
    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, forward, 1), EmptySequence());
    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, forward, 2),
        math::zero <Sequence>());

    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, backward, 10), EmptySequence());

    auto finalStates = flipsta::terminalStates (transformed, backward);
    BOOST_CHECK_EQUAL (range::walk_size (finalStates), 1);
    BOOST_CHECK_EQUAL (first (first (finalStates)), 10);
    BOOST_CHECK_EQUAL (second (first (finalStates)), EmptySequence());
}

BOOST_AUTO_TEST_CASE (testTransformExpandedLabels) {
    // This example has a sequence label, so that it is possible to test types.
    auto example = prefixExample();

    typedef math::empty_sequence <char> EmptySequence;
    typedef math::single_sequence <char> SingleSequence;
    typedef math::sequence <char> Sequence;

    // BumpChar only takes sequence semirings with "char", so this tests
    // transformExpandedLabels properly.
    auto transformed = flipsta::transformExpandedLabels (
        example, BumpChar(), example.descriptor());

    typedef decltype (transformed) TransformedAutomaton;

    static_assert (std::is_same <
        flipsta::LabelType <decltype (transformed)>::type, SingleSequence
        >::value, "");
    // The empty sequence remains empty (as it should).
    static_assert (std::is_same <
        TransformedAutomaton::TerminalLabel, EmptySequence>::value, "");

    BOOST_CHECK_EQUAL (range::walk_size (flipsta::states (transformed)), 8);

    BOOST_CHECK (flipsta::hasState (transformed, 10));
    BOOST_CHECK (!flipsta::hasState (transformed, 9));

    RANGE_FOR_EACH (arc, flipsta::arcsOn (transformed, forward, 4)) {
        BOOST_CHECK_EQUAL (size (arc.label().symbols()), 1);
        if (arc.state (forward) == 6) {
            // Bumped one from 'c'.
            BOOST_CHECK_EQUAL (arc.label().symbol(), 'd');
        } else {
            BOOST_CHECK_EQUAL (arc.state (forward), 3);

            // Bumped one from 'b'.
            BOOST_CHECK_EQUAL (arc.label().symbol(), 'c');
        }
    }

    // The terminal label must be able to represent zero and one, and is
    // therefore returned as type Sequence.
    static_assert (std::is_same <
        decltype (flipsta::terminalLabel (transformed, forward, 1)),
        Sequence>::value, "");
    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, forward, 1), EmptySequence());
    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, forward, 2),
        math::zero <Sequence>());

    BOOST_CHECK_EQUAL (
        flipsta::terminalLabel (transformed, backward, 10), EmptySequence());

    auto finalStates = flipsta::terminalStates (transformed, backward);
    BOOST_CHECK_EQUAL (range::walk_size (finalStates), 1);
    BOOST_CHECK_EQUAL (first (first (finalStates)), 10);
    BOOST_CHECK_EQUAL (second (first (finalStates)), EmptySequence());
}

BOOST_AUTO_TEST_SUITE_END()
