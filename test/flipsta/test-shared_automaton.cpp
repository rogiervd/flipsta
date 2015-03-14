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

#define BOOST_TEST_MODULE test_flipsta_shared_automaton
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/shared_automaton.hpp"

#include <iostream>

#include "math/cost.hpp"

BOOST_AUTO_TEST_SUITE(test_suite_flipsta_shared_automaton)

typedef char Key;
typedef math::cost <float> Weight;

typedef flipsta::SharedState <Key, Weight> State;
typedef flipsta::SharedAutomaton <Key, Weight> Automaton;
typedef flipsta::SharedAutomatonMemo <Key, Weight> Memo;

struct PrintValues {
    template <class Type1, class Type2>
        void operator() (Type1 const & o1, Type2 const & o2) const
    {
        RANGE_FOR_EACH (c, o1)
            std::cout << c;
        std::cout << ' ' << o2 << std::endl;
    }
};

std::vector <Automaton> exampleAutomata (Memo & memo) {
    std::vector <Automaton> result;

    Automaton nullAutomaton (math::zero <Weight>(), nullptr);
    // flipsta::enumerate (nullAutomaton, PrintValues());
    result.push_back (nullAutomaton);

    std::shared_ptr <State const> final = State::finalState();

    Automaton finalAutomaton (math::one <Weight>(), final);
    // flipsta::enumerate (finalAutomaton, PrintValues());
    result.push_back (finalAutomaton);

    Automaton finalAutomaton2 (Weight (2), final);
    // flipsta::enumerate (finalAutomaton2, PrintValues());
    result.push_back (finalAutomaton2);


    std::shared_ptr <State const> state1;
    {
        Automaton arc (Weight (0), final);
        std::map <Key, Automaton> arcs;
        arcs.insert (std::make_pair ('a', arc));
        State state (&memo, math::zero <Weight>(), arcs);
        state1 = memo.get (state);
    }

    Automaton automaton1 (Weight (.5), state1);
    // flipsta::enumerate (automaton1, PrintValues());
    result.push_back (automaton1);

    std::shared_ptr <State const> state2;
    {
        std::map <Key, Automaton> arcs;
        arcs.insert (std::make_pair ('b', Automaton (Weight (0), final)));
        arcs.insert (std::make_pair ('c', Automaton (Weight (1), state1)));
        State state (&memo, Weight (5), arcs);
        state2 = memo.get (state);
    }

    Automaton automaton2 (Weight (2), state2);
    // flipsta::enumerate (automaton2, PrintValues());
    result.push_back (automaton2);

    std::shared_ptr <State const> state3;
    {
        std::map <Key, Automaton> arcs;
        arcs.insert (std::make_pair ('z', Automaton (Weight (0), final)));
        State state (&memo, math::zero <Weight>(), arcs);
        state3 = memo.get (state);
    }

    Automaton automaton3 (Weight (1.5 + 3.75), state3);
    // flipsta::enumerate (automaton3, PrintValues());
    result.push_back (automaton3);

    std::shared_ptr <State const> state4;
    {
        std::map <Key, Automaton> arcs;
        arcs.insert (std::make_pair ('d', Automaton (Weight (.125), state2)));
        arcs.insert (std::make_pair ('e', Automaton (Weight (0), state3)));
        arcs.insert (std::make_pair ('f', Automaton (Weight (8), state1)));
        State state (&memo, math::zero <Weight>(), arcs);
        state4 = memo.get (state);
    }

    Automaton automaton4 (Weight (-3), state4);
    // flipsta::enumerate (automaton4, PrintValues());
    result.push_back (automaton4);

    std::shared_ptr <State const> state5;
    {
        std::map <Key, Automaton> arcs;
        arcs.insert (std::make_pair ('c', Automaton (Weight (0), state4)));
        State state (&memo, Weight (4.5), arcs);
        state5 = memo.get (state);
    }

    Automaton automaton5 (Weight (3), state5);
    // flipsta::enumerate (automaton5, PrintValues());
    result.push_back (automaton5);

    std::shared_ptr <State const> state6;
    {
        std::map <Key, Automaton> arcs;
        arcs.insert (std::make_pair ('c', Automaton (Weight (1), state4)));
        State state (&memo, Weight (0), arcs);
        state6 = memo.get (state);
    }

    Automaton automaton6 (Weight (4.75), state6);
    // flipsta::enumerate (automaton6, PrintValues());
    result.push_back (automaton6);

    std::shared_ptr <State const> state7;
    {
        std::map <Key, Automaton> arcs;
        arcs.insert (std::make_pair ('b', Automaton (Weight (0), state4)));
        State state (&memo, math::zero <Weight>(), arcs);
        state7 = memo.get (state);
    }

    Automaton automaton7 (Weight (5), state7);
    // flipsta::enumerate (automaton7, PrintValues());
    result.push_back (automaton7);

    return result;
}

typedef std::map <std::vector <Key>, Weight> Mapping;

class SumWeights {
    Mapping weights_;
public:
    SumWeights() {}

    void operator() (std::vector <Key> const & sequence, Weight const & weight)
    {
        auto iterator = weights_.find (sequence);
        if (iterator != weights_.end())
            iterator->second = iterator->second + weight;
        else
            weights_.insert (std::make_pair (sequence, weight));
    }

    Mapping const & weights() const { return weights_; }

    void print (std::ostream & os) const {
        RANGE_FOR_EACH (m, weights_) {
            RANGE_FOR_EACH (c, m.first)
                os << c;
            os << ' ' << m.second << '\n';
        }
    }
};

void testConcatenate (
    Automaton const & leftAutomaton, Mapping const & leftMapping,
    Automaton const & rightAutomaton, Mapping const & rightMapping)
{
    SumWeights reference;
    RANGE_FOR_EACH (left, leftMapping) {
        RANGE_FOR_EACH (right, rightMapping) {
            std::vector <Key> sequence = left.first;
            sequence.insert (sequence.end(),
                right.first.begin(), right.first.end());
            reference (sequence, left.second * right.second);
        }
    }

    Automaton concatenated = flipsta::concatenate (
        leftAutomaton, rightAutomaton);

    SumWeights result;
    flipsta::enumerate (concatenated, result);

    // std::cout << "Testing\n";

    BOOST_CHECK (result.weights() == reference.weights());
    if (result.weights() != reference.weights()) {
        std::cout << "Automaton:";
        Automaton concatenatedAgain = flipsta::concatenate (
            leftAutomaton, rightAutomaton);
        flipsta::print (std::cout, concatenatedAgain);
        std::cout << "This enumerates:\n";
        result.print (std::cout);
        std::cout << "Should have been:\n";
        reference.print (std::cout);
        std::cout << std::endl;
    }
}

void testUnion (
    Automaton const & leftAutomaton, Mapping const & leftMapping,
    Automaton const & rightAutomaton, Mapping const & rightMapping)
{
    SumWeights reference;
    RANGE_FOR_EACH (left, leftMapping)
        reference (left.first, left.second);
    RANGE_FOR_EACH (right, rightMapping)
        reference (right.first, right.second);

    Automaton u = flipsta::union_ (leftAutomaton, rightAutomaton);

    SumWeights result;
    flipsta::enumerate (u, result);

    // std::cout << "Testing union\n";

    BOOST_CHECK (result.weights() == reference.weights());
    if (result.weights() != reference.weights()) {
        std::cout << "Automaton:\n";
        Automaton uAgain = flipsta::union_ (leftAutomaton, rightAutomaton);
        print (std::cout, uAgain);
        std::cout << "This enumerates:\n";
        result.print (std::cout);
        std::cout << "Should have been:\n";
        reference.print (std::cout);
        std::cout << std::endl;
    }
}

void test_SharedAutomaton (Memo & memo) {
    auto automata = exampleAutomata (memo);
    std::vector <std::pair <Automaton, Mapping>> examples;
    RANGE_FOR_EACH (exampleAutomaton, automata) {
        SumWeights sumWeights;
        flipsta::enumerate (exampleAutomaton, sumWeights);
        examples.push_back (std::make_pair (
            exampleAutomaton, sumWeights.weights()));
    }

    std::shared_ptr <State const> final = State::finalState();
    Automaton finalAutomaton (math::one <Weight>(), final);
    Mapping finalMapping;
    finalMapping.insert (std::make_pair (
        std::vector <Key>(), math::one <Weight>()));

    RANGE_FOR_EACH (left, examples) {
        testConcatenate (finalAutomaton, finalMapping, left.first, left.second);

        RANGE_FOR_EACH (right, examples) {
            testConcatenate (
                left.first, left.second, right.first, right.second);
            testUnion (left.first, left.second, right.first, right.second);
        }
    }
}

BOOST_AUTO_TEST_CASE (test_flipsta_SharedAutomaton) {
    Memo memo;
    {
        test_SharedAutomaton (memo);
    }
}

BOOST_AUTO_TEST_SUITE_END()
