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

#ifndef FLIPSTA_AUTOMATON_SEMIRING_HPP_INCLUDED
#define FLIPSTA_AUTOMATON_SEMIRING_HPP_INCLUDED

#include <boost/functional/hash_fwd.hpp>

#include "math/magma.hpp"

#include "flipsta/shared_automaton.hpp"
#include "flipsta/core/hash_helper.hpp"

namespace flipsta {

namespace label {
    template <class Key, class Weight> class AutomatonSemiringTag;
} // namespace label

/** \brief
Semiring that contains an acyclic automaton that is always determinised and
minimised.

Internally, its value is represented by a SharedAutomaton.
Thus, the states of the automaton are shared as much as possible.
The label tag keeps a store of shared states alive.
Using this store, states can be shared between all automaton semirings inside
another automaton, or even between automata.

\todo Check that it is impossible/silly to implement operations:
-   divide.
*/
template <class Key, class Weight> class AutomatonSemiring;

template <class Key, class Weight> struct AutomatonSemiringMagmaTag;

template <class Key, class Weight> class AutomatonSemiring {
public:
    typedef SharedState <Key, Weight> State;
    typedef SharedAutomaton <Key, Weight> Automaton;
    typedef SharedAutomatonMemo <Key, Weight> Memo;
    typedef label::AutomatonSemiringTag <Key, Weight> LabelTag;

    Automaton automaton_;

private:
    static std::shared_ptr <State const> unityState (
        Memo * memo, Key const & key)
    {
        std::map <Key, Automaton> arcs;
        arcs.insert (std::make_pair (key,
            Automaton (math::one <Weight>(), State::finalState())));
        return memo->get (State (
            memo, math::zero <Weight>(), std::move (arcs)));
    }

public:
    explicit AutomatonSemiring (Automaton const & automaton)
    : automaton_ (automaton) {}

    explicit AutomatonSemiring (Automaton && automaton)
    : automaton_ (std::move (automaton)) {}

    /** \brief
    Construct as zero.
    */
    AutomatonSemiring()
    : automaton_(math::zero <Weight>(), nullptr) {}

    /** \brief
    Construct as a final automaton with weight \a weight.
    */
    explicit AutomatonSemiring (Weight const & weight)
    : automaton_(weight, State::finalState()) {}

    /** \brief
    Construct as an automaton that assigns a sequence with one symbol, \a key,
    \a weight.
    */
    explicit AutomatonSemiring (
        LabelTag const & tag, Weight const & weight, Key const & key)
    : automaton_(weight, unityState (tag.memo().get(), key))
    { assert (!automaton_.isNull()); }

    /** \brief
    Return the underlying automaton.
    */
    Automaton const & automaton() const { return automaton_; }

    friend std::size_t hash_value (AutomatonSemiring const & s) {
        boost::hash <Automaton> hasher;
        return hasher (s.automaton());
    }
};

namespace detail {

    template <class Type> struct IsAutomatonSemiringMagmaTag
    : boost::mpl::false_ {};

    template <class Key, class Weight>
        struct IsAutomatonSemiringMagmaTag <
            AutomatonSemiringMagmaTag <Key, Weight>>
    : boost::mpl::true_ {};

} // namespace detail

MATH_MAGMA_GENERATE_OPERATORS (detail::IsAutomatonSemiringMagmaTag)

namespace label {

    /** \brief
    Tag for AutomatonSemiring labels.

    Currently this does not perform any conversion between internal or external
    labels.

    \todo Should this perform conversion?
    Is conversion to automata with external labels something that can be called
    an external representation?

    Its main purpose is to keep the memo (which keeps states minimised and
    optimises the union operation) alive as long as the labels need it.
    */
    template <class Key, class Weight> class AutomatonSemiringTag {
        typedef SharedAutomatonMemo <Key, Weight> Memo;
        std::shared_ptr <Memo> memo_;

        struct Identity {
            template <class Type> Type const & operator() (Type const & o) const
            { return o; }

            template <class Type> Type operator() (Type && o) const
            { return std::forward <Type> (o); }
        };

    public:
        explicit AutomatonSemiringTag()
        : memo_ (std::make_shared <Memo>()) {}

        explicit AutomatonSemiringTag (std::shared_ptr <Memo> const & memo)
        : memo_ (memo) {}

        Identity compress() const { return Identity(); }
        Identity expand() const { return Identity(); }

        std::shared_ptr <Memo> const & memo() const { return memo_; }

        bool operator== (AutomatonSemiringTag const & that) const
        { return this->memo_ == that.memo_; }
    };

    // Sequences.
    template <class Key, class Weight>
        struct DefaultDescriptorFor <AutomatonSemiring <Key, Weight>>
    { typedef AutomatonSemiringTag <Key, Weight> type; };

} // namespace label

} // namespace flipsta

namespace math {

template <class Key, class Weight>
    struct decayed_magma_tag <flipsta::AutomatonSemiring <Key, Weight>>
{ typedef flipsta::AutomatonSemiringMagmaTag <Key, Weight> type; };

namespace operation {

    /* Queries. */

    template <class Key, class Weight>
        struct equal <flipsta::AutomatonSemiringMagmaTag <Key, Weight>>
    {
        bool operator() (flipsta::AutomatonSemiring <Key, Weight> const & left,
            flipsta::AutomatonSemiring <Key, Weight> const & right) const
        { return left.automaton() == right.automaton(); }
    };

    template <class Key, class Weight>
        struct is_annihilator <flipsta::AutomatonSemiringMagmaTag <Key, Weight>,
            callable::times>
    {
        bool operator() (flipsta::AutomatonSemiring <Key, Weight> const & s)
            const
        { return s.automaton().isNull(); }
    };

    /* Produce. */

    template <class Key, class Weight>
        struct identity <flipsta::AutomatonSemiringMagmaTag <Key, Weight>,
            callable::times>
    {
        flipsta::AutomatonSemiring <Key, Weight> operator() () const {
            return flipsta::AutomatonSemiring <Key, Weight> (
                math::one <Weight>());
        }
    };

    template <class Key, class Weight>
        struct identity <flipsta::AutomatonSemiringMagmaTag <Key, Weight>,
            callable::plus>
    {
        flipsta::AutomatonSemiring <Key, Weight> operator() () const
        { return flipsta::AutomatonSemiring <Key, Weight> (); }
    };

    /* Operations. */

    // times: call concatenate.
    template <class Key, class Weight>
        struct times <flipsta::AutomatonSemiringMagmaTag <Key, Weight>>
    : associative
    {
        flipsta::AutomatonSemiring <Key, Weight> operator() (
            flipsta::AutomatonSemiring <Key, Weight> const & left,
            flipsta::AutomatonSemiring <Key, Weight> const & right) const
        {
            return flipsta::AutomatonSemiring <Key, Weight> (
                concatenate (left.automaton(), right.automaton()));
        }
    };

    // plus: call union_.
    template <class Key, class Weight>
        struct plus <flipsta::AutomatonSemiringMagmaTag <Key, Weight>>
    : associative, commutative
    {
        flipsta::AutomatonSemiring <Key, Weight> operator() (
            flipsta::AutomatonSemiring <Key, Weight> const & left,
            flipsta::AutomatonSemiring <Key, Weight> const & right) const
        {
            return flipsta::AutomatonSemiring <Key, Weight> (
                union_ (left.automaton(), right.automaton()));
        }
    };

    // Only a left semiring.
    template <class Key, class Weight> struct is_semiring <
        flipsta::AutomatonSemiringMagmaTag <Key, Weight>, left,
        callable::times, callable::plus>
    : rime::true_type {};

    template <class Key, class Weight>
        struct print <flipsta::AutomatonSemiringMagmaTag <Key, Weight>>
    {
    private:
        static void printAutomaton (std::ostream & os,
            flipsta::SharedAutomaton <Key, Weight> const & automaton)
        {
            if (automaton.startWeight() == math::zero <Weight>()) {
                os << "zero";
                return;
            }
            os << '(' << automaton.startWeight() << ", ";
            auto state = automaton.state();
            if (state->finalWeight() != math::zero <Weight>())
                os << "final: " << state->finalWeight() << ' ';
            os << '[';
            RANGE_FOR_EACH (arc, state->arcs()) {
                os << arc.first << ": ";
                printAutomaton (os, arc.second);
                os << ", ";
            }
            os << "])";
        }

    public:
        void operator() (std::ostream & os,
            flipsta::AutomatonSemiring <Key, Weight> const & s) const
        { printAutomaton (os, s.automaton()); }
    };

} // namespace operation

} // namespace math

#endif // FLIPSTA_AUTOMATON_SEMIRING_HPP_INCLUDED
