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

#define BOOST_TEST_MODULE test_core_sole
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/detail/sole.hpp"

#include <vector>
#include <string>
#include <iostream>
#include <ostream>

#include <boost/functional/hash.hpp>

#include "range/for_each_macro.hpp"
#include "range/std/container.hpp"

using flipsta::detail::SoleStore;
using flipsta::detail::SoleBase;

using range::view;
using range::empty;
using range::chop_in_place;

/**
Class that keeps an int, but compares equal if (i % 5) is equal.
*/
struct IntModulo5 {
    int i;

    IntModulo5 (int i) : i (i) {}

    bool operator == (IntModulo5 const & other) const
    { return (this->i % 5) == (other.i % 5); }
};


std::size_t hash_value (IntModulo5 const & i)
{ return std::size_t (i.i % 5); }

/**
State that can have other states following it, and has a symbol associated with
it.
The hash value depends on the symbol, but not on whether it is final.
*/
class SimpleState : SoleBase <SimpleState> {
    typedef SoleBase <SimpleState> SoleType;
    friend class SoleBase <SimpleState>;

    char symbol_;
    bool final_;
    std::vector <std::shared_ptr <SimpleState const>> successors_;

    void addSuccessor_ (SimpleState const & s) {
        successors_.push_back (store()->get (s));
    }

public:
    SimpleState (Store * store, char symbol, bool final = false)
    : SoleType (store), symbol_ (symbol), final_ (final) {}

    char symbol() const { return symbol_; }

    bool final() const { return final_; }

    void addSuccessor (SimpleState const & s) const {
        // By making this mutable, we can test that states are actually shared.
        // Not normally recommended.
        const_cast <SimpleState *> (this)->addSuccessor_ (s);
    }

    void printSequences (std::ostream & os, std::string soFar) const {
        std::string current = soFar + symbol();
        if (final_)
            std::cout << current << '\n';
        RANGE_FOR_EACH (successor, successors_)
            successor->printSequences (os, current);
    }

    void appendSequences (
        std::vector <std::string> & sequences, std::string soFar) const
    {
        std::string current = soFar + symbol();
        if (final_)
            sequences.push_back (current);
        RANGE_FOR_EACH (successor, successors_)
            successor->appendSequences (sequences, current);
    }
};

bool operator== (SimpleState const & left, SimpleState const & right) {
    return left.symbol() == right.symbol() && left.final() == right.final();
}

std::size_t hash_value (SimpleState const & s) {
    boost::hash <char> hasher;
    return hasher (s.symbol());
}

BOOST_AUTO_TEST_SUITE(test_suite_sore)

BOOST_AUTO_TEST_CASE (test_sole_simple) {
    SoleStore <int> ints;

    {
        auto sole1 = ints.get (1);
        BOOST_CHECK_EQUAL (*sole1, 1);
        BOOST_CHECK (ints.get (1) == sole1);

        // minusOne is a special object, which is explicitly added and removed.
        auto minusOne = std::make_shared <int> (-1);
        ints.set (minusOne);

        // This is not allowed: 1 is already in the store.
        // auto one = std::make_shared <int> (1);
        // ints.set (one);

        auto sole7 = ints.get (7);
        BOOST_CHECK_EQUAL (*sole7, 7);
        BOOST_CHECK (ints.get (7) == sole7);

        int i10 = 10;
        auto sole10 = ints.get (i10);
        std::weak_ptr <int const> w (sole10);
        BOOST_CHECK (!w.expired());
        sole10.reset();
        // The SoleStore does not hold any shared_ptr itself.
        // The object must therefore have been destructed.
        BOOST_CHECK (w.expired());

        ints.remove (minusOne);
    }
}

// Multiple objects, some of which compare equal.
BOOST_AUTO_TEST_CASE (test_flipsta_multiple) {
    IntModulo5 a (4);
    IntModulo5 a2 (4);
    IntModulo5 b (9);
    IntModulo5 c (3);
    IntModulo5 d (7);

    {
        SoleStore <IntModulo5> store;

        std::shared_ptr <IntModulo5 const> aPtr = store.get (a);
        BOOST_CHECK_EQUAL (aPtr->i, 4);
        std::shared_ptr <IntModulo5 const> a2Ptr = store.get (a2);
        BOOST_CHECK_EQUAL (a2Ptr->i, 4);

        BOOST_CHECK (aPtr == a2Ptr);

        std::shared_ptr <IntModulo5 const> bPtr = store.get (b);
        // Instead of b, a pointer to a copy of a must have been returned,
        // because they compare equal.
        BOOST_CHECK (bPtr == aPtr);
        BOOST_CHECK_EQUAL (bPtr->i, 4);

        std::shared_ptr <IntModulo5 const> cPtr = store.get (c);
        BOOST_CHECK_EQUAL (cPtr->i, 3);

        std::shared_ptr <IntModulo5 const> dPtr = store.get (d);
        BOOST_CHECK_EQUAL (dPtr->i, 7);

        std::weak_ptr <IntModulo5 const> cWeakPtr = cPtr;
        cPtr.reset();
        // The copy of c is not alive anymore.
        BOOST_CHECK (cWeakPtr.lock() == std::shared_ptr <IntModulo5 const>());

        // Add it again.
        cPtr = store.get (c);
        BOOST_CHECK_EQUAL (cPtr->i, 3);
    }
}

// Derive from SoleBase.
BOOST_AUTO_TEST_CASE (test_sole_derived) {
    SoleStore <SimpleState> states;

    auto singletonQ = std::make_shared <SimpleState> (nullptr, 'q');

    states.set (singletonQ);

    {
        auto soleA = states.get (SimpleState (&states, 'a'));
        BOOST_CHECK_EQUAL (soleA->symbol(), 'a');
        BOOST_CHECK (states.get (SimpleState (&states, 'a')) == soleA);
        BOOST_CHECK (states.get (SimpleState (&states, 'a', true)) != soleA);

        SimpleState qState (&states, 'q');
        auto soleQ = states.get (qState);
        BOOST_CHECK_EQUAL (singletonQ, soleQ);

        auto bFinal = states.get (SimpleState (&states, 'b', true));
        auto bNonFinal = states.get (SimpleState (&states, 'b', false));
        BOOST_CHECK_EQUAL (bFinal->symbol(), 'b');
        BOOST_CHECK (
            states.get (SimpleState (&states, 'b')) == bNonFinal);
        BOOST_CHECK (states.get (SimpleState (&states, 'b', true)) == bFinal);

        soleA->addSuccessor (SimpleState (&states, 'b'));
        soleA->addSuccessor (SimpleState (&states, 'c'));
        soleA->addSuccessor (SimpleState (&states, 'b', true));

        // Change B and C in hindsight. (Not normally recommended.)
        bFinal->addSuccessor (SimpleState (&states, 'c'));
        bNonFinal->addSuccessor (SimpleState (&states, 'd', true));
        states.get (SimpleState (&states, 'c'))->addSuccessor (
            SimpleState (&states, 'd', true));

        // Printing the sequences is useful to see what is happening.
        bFinal->printSequences (std::cout, "From bFinal: ");
        soleA->printSequences (std::cout, "From soleA: ");

        {
            std::vector <std::string> sequences;
            bFinal->appendSequences (sequences, "");

            auto s = view (sequences);
            BOOST_CHECK_EQUAL (chop_in_place (s), "b");
            BOOST_CHECK_EQUAL (chop_in_place (s), "bcd");
            BOOST_CHECK (empty (s));
        }
        {
            std::vector <std::string> sequences;
            soleA->appendSequences (sequences, "");

            auto s = view (sequences);
            BOOST_CHECK_EQUAL (chop_in_place (s), "abd");
            BOOST_CHECK_EQUAL (chop_in_place (s), "acd");
            BOOST_CHECK_EQUAL (chop_in_place (s), "ab");
            BOOST_CHECK_EQUAL (chop_in_place (s), "abcd");
            BOOST_CHECK (empty (s));
        }
    }
    states.remove (singletonQ);
}

BOOST_AUTO_TEST_SUITE_END()
