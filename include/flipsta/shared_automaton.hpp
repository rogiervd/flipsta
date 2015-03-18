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

#ifndef FLIPSTA_SHARED_AUTOMATON_HPP_INCLUDED
#define FLIPSTA_SHARED_AUTOMATON_HPP_INCLUDED

#include <map>
#include <unordered_map>

#include <boost/functional/hash.hpp>

#include "range/core.hpp"
#include "range/for_each_macro.hpp"
#include "range/std/container.hpp"
#include "range/tuple.hpp"
#include "range/hash_range.hpp"

#include "math/magma.hpp"

#include "core.hpp"
#include "queue.hpp"
#include "core/hash_helper.hpp"

#include "detail/shared_automaton_state_memo.hpp"

namespace flipsta {

/** \brief
An immutable state in an acyclic automaton that is always determinised and
minimised.

Because SharedState objects are immutable, they can be shared, and their hash
value can be cached.
Sharing the objects makes it possible to keep a collection of automata jointly
minimised, as explained below.
By caching the hash values, each new state only has to compute one level of hash
values, so that the hash value of a whole automaton is always fast to compute.

It has a final weight, the weight assigned to the path ending at this state.
Arcs out of the state are represented with a sequence of zero or more keys
(often symbols) with a weight and a destination state attached to them.

The state is determinised with respect to the key by definition, since each
key can have only one weight and one destination state.

That the state is also minimised must be shown in two steps.

The weights must be normalised, that is, the sum of all the weights (the final
weight and all the weights attached to keys) must be semiring-one.

There is a store of states that keeps track of all states.
Whenever a SharedState is created, it must checked whether a SharedState with
the same values exists already.
If such a SharedState exists, the shared_ptr to it is used.
Otherwise, it is created and kept in the store to be used next time a
SharedState with the same value is required.

The state is minimised since all states are normalised and then looked up in the
store.
Any two states with the same suffix (as it is called) will therefore be the same
object.
*/
template <class Key, class Weight> class SharedState
: detail::SoleBase <SharedState <Key, Weight>>
{
    friend class detail::SoleBase <SharedState>;
    friend struct detail::SharedAutomatonOperations <Key, Weight>;

    typedef detail::SoleBase <SharedState <Key, Weight>> SoleBase;
    typedef typename SoleBase::Store Store;
    typedef SharedAutomatonMemo <Key, Weight> Memo;

public:
    typedef SharedAutomaton <Key, Weight> Automaton;

private:
    typedef std::map <Key, Automaton> Arcs;

    Weight const finalWeight_;
    Arcs const arcs_;

    std::size_t hash_;

    // The hash is cached, because it becomes slow to compute for a whole
    // automaton.
    std::size_t computeHash() const {
        std::size_t seed = 0;
        boost::hash_combine (seed, finalWeight_);
        RANGE_FOR_EACH (arc, arcs_) {
            boost::hash_combine (seed, arc.first);
            boost::hash_combine (seed, arc.second);
        }
        return seed;
    }

    void assertNormalised() const {
        Weight sum = finalWeight_;
        RANGE_FOR_EACH (arc, arcs_)
            sum = sum + arc.second.startWeight();
        assert (math::approximately_equal (sum, math::one <Weight>()));
    }

    Memo * memo() const { return static_cast <Memo *> (SoleBase::store()); }

public:
    SharedState (SharedState const & that)
    : SoleBase (that), finalWeight_ (that.finalWeight_), arcs_ (that.arcs_),
        hash_ (that.hash_) {}

    SharedState (SharedState && that)
    : SoleBase (std::move (that)), finalWeight_ (std::move (that.finalWeight_)),
        arcs_ (std::move (that.arcs_)), hash_ (that.hash_) {}

    /** \brief
    Initialise as a final state with no arcs to other states.

    Since the state must be normalised, the final weight is math::one.

    \post \c arcs().empty().
    \post <c>finalWeight() == math::zero \<Weight>()</c>
    */
    explicit SharedState()
    : SoleBase (nullptr), finalWeight_ (math::one <Weight>()), arcs_(),
        hash_ (computeHash())
    { assertNormalised(); }

    /** \brief
    Initialise explicitly, with a final weight and outgoing arcs.
    */
    SharedState (Memo * memo, Weight const & finalWeight, Arcs const & arcs)
    : SoleBase (memo), finalWeight_ (finalWeight), arcs_ (arcs),
        hash_ (computeHash())
    { assertNormalised(); }

    /** \brief
    Initialise explicitly, with a final weight and outgoing arcs.

    The arcs are moved.
    */
    SharedState (Memo * memo, Weight const & finalWeight, Arcs && arcs)
    : SoleBase (memo), finalWeight_ (finalWeight), arcs_ (std::move (arcs)),
        hash_ (computeHash())
    { assertNormalised(); }

    ~SharedState() {
        // Remove this state from the memo: it will not be passed to union()
        // anymore now.
        if (memo())
            memo()->removeStatePointer (this);
    }

    /**
    \return The final weight, i.e. the weight assigned to paths that end at this
    state.
    */
    Weight const & finalWeight() const { return finalWeight_; }

    /**
    \return The outgoing arcs, as a sequence of pairs of key and automata.
    */
    Arcs const & arcs() const { return arcs_; }

    /** \brief
    Return a \c shared_ptr to the singleton final state.

    It is useful for this to be a singleton object so that it can be found
    without any reference to a SoleStore.
    */
    static std::shared_ptr <SharedState const> finalState() {
        static std::shared_ptr <SharedState const> state
            = std::make_shared <SharedState>();
        return state;
    }

    friend std::size_t hash_value (SharedState const & state)
    { return state.hash_; }

    bool operator== (SharedState const & that) const {
        return this == &that
            || (this->finalWeight_ == that.finalWeight_
                && this->arcs_ == that.arcs_);
    }
};

/** \brief
An acyclic automaton that is always kept jointly determinised and minimised.

See the documentation of \ref SharedState for the proof that the states are
always determinised and minimised.

The important aspect for SharedAutomaton is that SharedState is always
normalised, that is, the semiring-sum of its weights is semiring-one.
This means the weights of the automaton are pushed to the front.
Therefore, this class contains not only a pointer to a SharedState, but also a
start weight.

It is possible for the start weight to be semiring-zero.
In that case, any path through this automaton would have a weight of zero.
The state is therefore set to \c nullptr in that case.
Otherwise, the state must be non-null.

\todo Allow access through the standard automaton protocol.
*/
template <class Key, class Weight> class SharedAutomaton {
public:
    typedef SharedState <Key, Weight> State;

private:
    Weight startWeight_;
    std::shared_ptr <State const> state_;

public:
    // Make sure that the copy and move constructors are defined.
    SharedAutomaton (SharedAutomaton const & that)
    : startWeight_ (that.startWeight_), state_ (that.state_) {}

    SharedAutomaton (SharedAutomaton && that)
    : startWeight_ (std::move (that.startWeight_)),
        state_ (std::move (that.state_)) {}

    SharedAutomaton & operator= (SharedAutomaton const & that) {
        this->startWeight_ = that.startWeight_;
        this->state_ = that.state_;
        return *this;
    }

    SharedAutomaton & operator= (SharedAutomaton && that) {
        this->startWeight_ = std::move (that.startWeight_);
        this->state_ = std::move (that.state_);
        return *this;
    }

    /** \brief
    Construct with an explicit start weight and state.

    Either the start weight must be zero, or the state must be non-null.
    */
    SharedAutomaton (
        Weight const & startWeight, std::shared_ptr <State const> const & state)
    : startWeight_ (startWeight),
        state_ (startWeight == math::zero <Weight>() ? nullptr : state)
    {
        if (startWeight_ == math::zero <Weight>()) {
            state_ = nullptr;
        } else {
            assert (state_ != nullptr);
        }
    }

    /**
    \return The start weight.
    */
    Weight const & startWeight() const { return startWeight_; }

    /** \brief
    Premultiply the start weight with \a w.

    This is a mutable operation.
    */
    void premultiply (Weight const & w)
    { this->startWeight_ = math::times (w, this->startWeight_); }

    /** \brief
    Divide the start weight by \a w from the left.

    This is a mutable operation.
    */
    void predivide (Weight const & w)
    { this->startWeight_ = math::divide <math::left> (this->startWeight_, w); }

    /** \brief
    Return A std::shared_ptr to the state.

    The shared_ptr is empty iff \c startWeight() is zero.
    */
    std::shared_ptr <State const> state() const { return state_; }

    /** \brief
    Return \c true iff the automaton is a null automaton, which allows no
    non-zero paths.
    */
    bool isNull() const { return startWeight() == math::zero <Weight>(); }

    bool operator == (SharedAutomaton const & that) const {
        return this->startWeight_ == that.startWeight_
            && *this->state_ == *that.state_;
    }
};

template <class Key, class Weight> inline
    std::size_t hash_value (SharedAutomaton <Key, Weight> const & automaton)
{
    std::size_t seed = 0;
    boost::hash_combine (seed, automaton.startWeight());
    if (automaton.state())
        boost::hash_combine (seed, *automaton.state());
    return seed;
}

namespace detail {

    /**
    Provides functions (as static methods) that apply to SharedAutomaton
    objects.
    In essence, this is a templated namespace.
    */
    template <class Key, class Weight> struct SharedAutomatonOperations {

        typedef SharedState <Key, Weight> State;
        typedef SharedAutomaton <Key, Weight> Automaton;
        typedef std::shared_ptr <State const> StatePtr;

        typedef std::map <Key, Automaton> Arcs;

        typedef SharedAutomatonMemo <Key, Weight> Memo;

        /** \brief
        Normalise a start weight and arcs.

        This computes the semiring-sum over the start weight and all the arc
        weights.

        \return
            The weight that has been normalised by, the normalised start weight,
            and the normalised arcs.
            If the arcs are already normalised, return
            <c>math::one \<Weight>()</c>, \a start and the original arcs.
        */
        static range::tuple <Weight, Weight, Arcs> normaliseArcs (
            Weight const & start, Arcs && arcs)
        {
            Weight sum = start;
            RANGE_FOR_EACH (arc, arcs)
                sum = sum + arc.second.startWeight();
            if (math::equal (sum, math::one <Weight>())) {
                // Do not change anything.
                return range::make_tuple (
                    math::one <Weight>(), start, std::move (arcs));
            } else {
                RANGE_FOR_EACH (arc, arcs)
                    arc.second.predivide (sum);
                return range::make_tuple (sum,
                    math::divide <math::left> (start, sum), std::move (arcs));
            }
        }

        static Automaton makeAutomaton (Memo * memo,
            Weight const & finalWeight, Arcs && arcs)
        {
            range::tuple <Weight, Weight, Arcs> result
                = normaliseArcs (finalWeight, std::move (arcs));
            StatePtr newState = memo->get (
                State (memo, range::second (result),
                    std::move (range::third (result))));
            return Automaton (range::first (result), newState);
        }

        /** \brief
        Compute a union of two automata where one automaton has a start weight
        of one, and its arcs are a temporary.

        This case can be optimised since the arcs of the other automaton can be
        added to the arcs of the one.

        The arcs do not have to be normalised.
        */
        static Automaton addAutomaton (Memo * memo,
            Weight const & leftFinalWeight, Arcs && arcs,
            Weight const & rightPreWeight, Weight const & rightFinalWeight,
            Arcs const & rightArcs)
        {
            RANGE_FOR_EACH (rightArc, rightArcs) {
                auto leftArc = arcs.find (rightArc.first);
                if (leftArc == arcs.end()) {
                    arcs.insert (std::make_pair (rightArc.first, Automaton (
                        rightPreWeight * rightArc.second.startWeight(),
                        rightArc.second.state())));
                } else {
                    // Left and right arc have the same key.
                    Automaton right (
                        rightPreWeight * rightArc.second.startWeight(),
                        rightArc.second.state());
                    leftArc->second =
                        union_ (std::move (leftArc->second), std::move (right));
                }
            }

            return makeAutomaton (memo,
                leftFinalWeight + rightPreWeight * rightFinalWeight,
                std::move (arcs));
        }

        /** \brief
        Compute the union between essentially two automata, but without the
        requirements that the states are normalised.

        The preconditions require that neither state is entirely vacuous.

        \pre leftPreWeight != math::zero <Weight>().
        \pre rightPreWeight != math::zero <Weight>().
        \pre leftFinalWeight != math::zero <Weight>() || !leftArcs.empty().
        \pre rightFinalWeight != math::zero <Weight>() || !rightArcs.empty().
        */
        static Automaton computeUnion (Memo * memo,
            Weight const & leftPreWeight, Weight const & leftFinalWeight,
            Arcs const & leftArcs,
            Weight const & rightPreWeight, Weight const & rightFinalWeight,
            Arcs const & rightArcs)
        {
            assert (leftPreWeight != math::zero <Weight>());
            assert (rightPreWeight != math::zero <Weight>());
            assert (leftFinalWeight != math::zero <Weight>()
                || !leftArcs.empty());
            assert (rightFinalWeight != math::zero <Weight>()
                || !rightArcs.empty());

            // Optimisation for when one of the start weights equals one.
            // This should always be true for idempotent weights.
            if (leftPreWeight == math::one <Weight>()) {
                Arcs leftArcsCopy = leftArcs;
                return addAutomaton (memo,
                    leftFinalWeight, std::move (leftArcsCopy),
                    rightPreWeight, rightFinalWeight, rightArcs);
            }
            if (rightPreWeight == math::one <Weight>()) {
                Arcs rightArcsCopy = rightArcs;
                return addAutomaton (memo,
                    rightFinalWeight, std::move (rightArcsCopy),
                    leftPreWeight, leftFinalWeight, leftArcs);
            }

            // Merge leftArcs and rightArcs.
            Arcs newArcs;
            auto leftCurrent = range::view (leftArcs);
            auto rightCurrent = range::view (rightArcs);
            while (!range::empty (leftCurrent) && !range::empty (rightCurrent))
            {
                auto const & leftArc = range::first (leftCurrent);
                auto const & rightArc = range::first (rightCurrent);

                if (leftArc.first < rightArc.first) {
                    // Left arc has the lowest key, so insert it.
                    newArcs.insert (std::make_pair (leftArc.first,
                        Automaton (leftPreWeight * leftArc.second.startWeight(),
                            leftArc.second.state())));
                    leftCurrent = range::drop (leftCurrent);
                } else if (rightArc.first < leftArc.first) {
                    // Right arc has the lowest key, so insert it.
                    newArcs.insert (std::make_pair (rightArc.first,
                        Automaton (
                            rightPreWeight * rightArc.second.startWeight(),
                            rightArc.second.state())));
                    rightCurrent = range::drop (rightCurrent);
                } else {
                    // Left and right arc are equal.
                    Automaton left (
                        leftPreWeight * leftArc.second.startWeight(),
                        leftArc.second.state());
                    Automaton right (
                        rightPreWeight * rightArc.second.startWeight(),
                        rightArc.second.state());
                    newArcs.insert (std::make_pair (leftArc.first,
                        union_ (std::move (left), std::move (right))));

                    leftCurrent = range::drop (leftCurrent);
                    rightCurrent = range::drop (rightCurrent);
                }
            }

            RANGE_FOR_EACH (leftArc, leftCurrent) {
                newArcs.insert (std::make_pair (leftArc.first,
                    Automaton (leftPreWeight * leftArc.second.startWeight(),
                        leftArc.second.state())));
            }
            RANGE_FOR_EACH (rightArc, rightCurrent) {
                newArcs.insert (std::make_pair (rightArc.first,
                    Automaton (rightPreWeight * rightArc.second.startWeight(),
                        rightArc.second.state())));
            }

            Weight newFinalWeight =
                leftPreWeight * leftFinalWeight
                + rightPreWeight * rightFinalWeight;

            return makeAutomaton (memo, newFinalWeight, std::move (newArcs));
        }

        /**
        \todo The current implementation takes time in the order of the number
        of paths in the left automaton.
        This should be the number of distinct states.
        For the use case of the automaton semiring, this does not make a
        difference, but for other cases it might.

        A better implementation would either:
        1.  memoise the result of this; or
        2.  inside of each call to this, have a closure with the right automaton
            and a memo call itself recursively.
            This will be faster than (1) by a constant, and it may save space
            in practice.

        But maybe we first need to think of a clear use case to make this
        judgement call.
        */
        static Automaton concatenate (
            Automaton const & left, Automaton const & right)
        {
            // ++ sharedAutomatonOperationCounts().concatenateCalls;

            typedef SharedAutomaton <Key, Weight> Automaton;

            if (left.startWeight() == math::zero <Weight>()
                    || right.startWeight() == math::zero <Weight>())
                return Automaton (math::zero <Weight>(), nullptr);

            // If the left sequence is empty: a weighted version of "right".
            Weight leftEmptyStartWeight =
                left.state()->finalWeight() * right.startWeight();

            if (left.state()->arcs().empty()) {
                // ++ sharedAutomatonOperationCounts().concatenateLeftEmpty;
                return Automaton (left.startWeight() * leftEmptyStartWeight,
                    right.state());
            }

            if (right.state()->arcs().empty()) {
                // ++ sharedAutomatonOperationCounts().concatenateRightEmpty;
            }

            // If the left sequence is not empty: start with each of left's
            // keys.
            Arcs newArcs;
            RANGE_FOR_EACH (arc, left.state()->arcs()) {
                Key key = arc.first;
                Automaton const & automaton = arc.second;
                Automaton newArc = concatenate (automaton, right);
                newArcs.insert (std::make_pair (key, newArc));
            }

            Memo * memo = left.state()->memo();
            if (leftEmptyStartWeight != math::zero <Weight>()) {
                Automaton result = addAutomaton (memo,
                    math::zero <Weight>(), std::move (newArcs),
                    leftEmptyStartWeight,
                    right.state()->finalWeight(), right.state()->arcs());

                result.premultiply (left.startWeight());
                return std::move (result);
            } else {
                Automaton result = makeAutomaton (memo,
                    math::zero <Weight>(), std::move (newArcs));

                result.premultiply (left.startWeight());
                return std::move (result);
            }
        }

        /** \brief
        Compute the union of two SharedAutomaton objects, trying to re-use
        results computed by storing them in a memo.

        The memo stores results with normalised start weights, and consistent
        order of the arguments.

        This does not deal with trivial cases, because those should not end up
        in the memo.
        Such trivial cases are when the left or right automata are null (in this
        case the other automaton should be returned) and when the states of both
        automata are the same.
        When the trivial cases are removed, at least one of the arguments is a
        non-final state, which means that it has a pointer to the memo, which
        can then be used.

        \pre <c>!left.isNull()</c>
        \pre <c>!right.isNull()</c>
        \pre <c>left != right.</c>
        */
        static Automaton memoisedUnion (
            Automaton const & left, Automaton const & right)
        {
            // Normalise the weights.
            Weight leftStartWeight = left.startWeight();
            Weight rightStartWeight = right.startWeight();
            Weight normalisation = leftStartWeight + rightStartWeight;
            leftStartWeight = math::divide <math::left> (
                leftStartWeight, normalisation);
            rightStartWeight = math::divide <math::left> (
                rightStartWeight, normalisation);

            // left can be the final state, or right, but not both, because the
            // case where the two states are the same has already been dealt
            // with.
            Memo * memo = left.state()->memo();
            if (memo) {
                if (right.state()->memo()) {
                    // The left and right state must have a pointer to the same
                    // memo.
                    assert (memo == right.state()->memo());
                }
            } else {
                memo = right.state()->memo();
                assert (memo != nullptr);
            }

            typename Memo::UnionArguments arguments (
                leftStartWeight, left.state(), rightStartWeight, right.state());

            Automaton result = memo->retrieve (arguments);

            if (result.isNull()) {
                // Actually compute the automaton.
                result = computeUnion (memo,
                    leftStartWeight,
                    left.state()->finalWeight(), left.state()->arcs(),
                    rightStartWeight,
                    right.state()->finalWeight(), right.state()->arcs());

                memo->remember (arguments, result);
            }
            result.premultiply (normalisation);
            return std::move (result);
        }

        static Automaton union_ (
            Automaton const & left, Automaton const & right)
        {
            // Deal with trivial cases.

            if (left.isNull()) {
                return right;
            }
            if (right.isNull()) {
                return left;
            }

            // Deal with the case where the left and right state are the same.
            // This is mostly an optimisation, but it is necessary if both are
            // the final state, since then neither has a pointer to the memo.
            if (left.state() == right.state()) {
                return Automaton (
                    left.startWeight() + right.startWeight(), left.state());
            }

            Automaton result = memoisedUnion (left, right);

            return std::move (result);
        }

        static void writeAttAutomaton (
            std::ostream & stream, Automaton const & automaton)
        {
            typedef Dense <std::size_t> DenseType;
            std::unordered_map <StatePtr, DenseType, boost::hash <StatePtr>>
                stateIndices;
            flipsta::LifoQueue <StatePtr> todo;
            if (!automaton.isNull()) {
                // There is only one start state so that makes this simple.
                stateIndices.insert (std::make_pair (
                    automaton.state(), DenseType (0)));
                std::size_t lastStateIndex = 0;
                todo.push (automaton.state());
                while (!todo.empty()) {
                    StatePtr source = todo.pop();
                    std::size_t sourceIndex = stateIndices.at (source);

                    Weight finalWeight = source->finalWeight();
                    if (finalWeight != math::zero <Weight>()) {
                        // This is a final state.
                        stream << sourceIndex;
                        if (finalWeight != math::one <Weight>())
                            stream << ' ' << finalWeight;
                        stream << '\n';
                    }

                    RANGE_FOR_EACH (arc, source->arcs()) {
                        Key symbol = arc.first;
                        Weight weight = arc.second.startWeight();
                        StatePtr destination = arc.second.state();

                        std::size_t destinationIndex;
                        auto destinationIndexPosition
                            = stateIndices.find (destination);
                        if (destinationIndexPosition != stateIndices.end())
                            destinationIndex = destinationIndexPosition->second;
                        else {
                            todo.push (destination);
                            ++ lastStateIndex;
                            destinationIndex = lastStateIndex;
                            stateIndices.insert (std::make_pair (
                                destination, DenseType (destinationIndex)));
                        }

                        stream << sourceIndex << ' ' << destinationIndex
                            << ' ' << symbol << ' ' << symbol;
                        if (weight != math::one <Weight>())
                            stream << ' ' << weight;
                        stream << '\n';
                    }
                }
            }
        }

        template <class CallBack> static void enumerate (
            std::vector <Key> const & previousKeys,
            Weight const & previousWeight,
            Automaton const & automaton, CallBack && callBack)
        {
            Weight currentWeight = previousWeight * automaton.startWeight();
            if (currentWeight != math::zero <Weight>()) {
                auto const & state = *automaton.state();
                if (state.finalWeight() != math::zero <Weight>())
                    callBack (previousKeys,
                        currentWeight * state.finalWeight());
                RANGE_FOR_EACH (arc, state.arcs()) {
                    std::vector <Key> currentKeys = previousKeys;
                    currentKeys.push_back (arc.first);
                    enumerate (currentKeys, currentWeight, arc.second,
                        callBack);
                }
            }
        }

        static void print (std::ostream & stream,
            std::string const & indentation, Automaton const & automaton)
        {
            if (automaton.startWeight() == math::zero <Weight>()) {
                stream << "zero\n";
                return;
            }
            stream << automaton.startWeight();
            auto state = automaton.state();
            stream << " to " << state
                // << ' ' << hash_value (*state)
                << '\n';
            if (state->finalWeight() != math::zero <Weight>())
                stream << indentation << "final: " << state->finalWeight()
                    << '\n';
            RANGE_FOR_EACH (arc, state->arcs()) {
                stream << indentation << arc.first << ": ";
                print (stream, indentation + "  ", arc.second);
            }
        }

    };

} // namespace detail

// Accessor functions.
// These should probably all be more general.

template <class Key, class Weight> inline
    SharedAutomaton <Key, Weight> concatenate (
        SharedAutomaton <Key, Weight> const & left,
        SharedAutomaton <Key, Weight> const & right)
{
    return detail::SharedAutomatonOperations <Key, Weight>::concatenate (
        left, right);
}

template <class Key, class Weight> inline
    SharedAutomaton <Key, Weight> union_ (
        SharedAutomaton <Key, Weight> const & left,
        SharedAutomaton <Key, Weight> const & right)
{
    return detail::SharedAutomatonOperations <Key, Weight>::union_ (
        left, right);
}

template <class Key, class Weight> inline
    void writeAttAutomaton (
        std::ostream & stream, SharedAutomaton <Key, Weight> const & automaton)
{
    return detail::SharedAutomatonOperations <Key, Weight>::writeAttAutomaton (
        stream, automaton);
}

template <class Key, class Weight, class CallBack> inline
    void enumerate (
        SharedAutomaton <Key, Weight> const & automaton, CallBack && callBack)
{
    detail::SharedAutomatonOperations <Key, Weight>::enumerate (
        std::vector <Key>(), math::one <Weight>(), automaton, callBack);
}

template <class Key, class Weight> inline
    void print (std::ostream & stream,
        SharedAutomaton <Key, Weight> const & automaton)
{
    detail::SharedAutomatonOperations <Key, Weight>::print (
        stream, "", automaton);
}

} // namespace flipsta

#endif // FLIPSTA_SHARED_AUTOMATON_HPP_INCLUDED
