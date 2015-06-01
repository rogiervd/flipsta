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

#ifndef FLIPSTA_DETAIL_SHARED_AUTOMATON_STATE_MEMO_HPP_INCLUDED
#define FLIPSTA_DETAIL_SHARED_AUTOMATON_STATE_MEMO_HPP_INCLUDED

#include <memory>
#include <utility>

#include <boost/functional/hash.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/global_fun.hpp>

#include "range/tuple.hpp"

#include "./sole.hpp"

#include "flipsta/core/hash_helper.hpp"

namespace flipsta {

    // Forward definitions
    template <class Key, class Weight> class SharedState;
    template <class Key, class Weight> class SharedAutomaton;

    namespace detail {
        template <class Key, class Weight> struct SharedAutomatonOperations;
    } // namespace detail

    /** \brief
    Keep track of SharedState objects, and memoise the result of computing
    the union of two SharedAutomaton objects.

    This derives from SoleStore, so that a <c>SoleStore *</c> can be converted
    with <c>static_cast \<SharedAutomatonMemo *> (...)</c>.
    This SoleStore will contain the singleton final state.
    */
    template <class Key, class Weight> class SharedAutomatonMemo
    : public detail::SoleStore <SharedState <Key, Weight>>
    {
        typedef detail::SoleStore <SharedState <Key, Weight>> Store;
        typedef SharedState <Key, Weight> State;
        typedef SharedAutomaton <Key, Weight> Automaton;
        typedef std::shared_ptr <State const> StatePtr;
        typedef std::weak_ptr <State const> WeakStatePtr;

    public:
        class UnionArguments {
            Weight leftWeight_;
            State const * leftPointer_;

            Weight rightWeight_;
            State const * rightPointer_;

        public:
            UnionArguments (
                Weight const & leftWeight,
                std::shared_ptr <State const> const & leftState,
                Weight const & rightWeight,
                std::shared_ptr <State const> const & rightState)
            : leftWeight_ (leftWeight), leftPointer_ (leftState.get()),
                rightWeight_ (rightWeight), rightPointer_ (rightState.get())
            {
                /*
                The items inserted into the memo should be as normalised as
                possible, to increase reuse.
                But the result of a sequence of computations should always be
                the same for every run of the program.
                The union is an associative operation, so the left and right
                arguments can be swapped.
                The actual computation of the union is performed in the same
                order regardless of which pointer has a lower address.
                */
                if (rightPointer_ < leftPointer_) {
                    using std::swap;
                    swap (leftWeight_, rightWeight_);
                    swap (leftPointer_, rightPointer_);
                }
            }

            friend std::size_t hash_value (UnionArguments const & a) {
                std::size_t seed = 0;
                boost::hash_combine (seed, a.leftWeight_);
                boost::hash_combine (seed, a.leftPointer_);
                boost::hash_combine (seed, a.rightWeight_);
                boost::hash_combine (seed, a.rightPointer_);
                return seed;
            }

            bool operator == (UnionArguments const & that) const {
                return this->leftWeight_ == that.leftWeight_
                    && this->leftPointer_ == that.leftPointer_
                    && this->rightWeight_ == that.rightWeight_
                    && this->rightPointer_ == that.rightPointer_;
            }

            State const * leftPointer() const { return leftPointer_; }
            State const * rightPointer() const { return rightPointer_; }
        };

    private:
        /** \brief
        The result as it is saved.

        This would be an Automaton, but if the left or right argument is the
        state itself, then that would create a cycle, so a weak_ptr should be
        saved.
        */
        typedef std::pair <Weight,
            rime::variant <StatePtr, WeakStatePtr>> StoredResult;

        // Stored mapping from arguments to result.
        typedef std::pair <UnionArguments, StoredResult> Mapping;

        static State const * leftRawPointer (Mapping const & i)
        { return i.first.leftPointer(); }

        static State const * rightRawPointer (Mapping const & i)
        { return i.first.rightPointer(); }

        /**
        The actual memo, which has three indices:
        \li hashed by UnionArguments, for looking up stored results;
        \li hashed by the left state pointer, to delete the stored result when
            the argument gets destructed (keeping it would be useless).
        \li hashed by the right state pointer.
        */
        typedef boost::multi_index_container <
            Mapping,
            boost::multi_index::indexed_by <
                boost::multi_index::hashed_unique <
                    boost::multi_index::member <
                        Mapping, UnionArguments, &Mapping::first>>,
                boost::multi_index::hashed_non_unique <
                    boost::multi_index::global_fun <
                        Mapping const &, State const *,
                        SharedAutomatonMemo::leftRawPointer>>,
                boost::multi_index::hashed_non_unique <
                    boost::multi_index::global_fun <
                        Mapping const &, State const *,
                        SharedAutomatonMemo::rightRawPointer>>
            >> Memo;

        Memo memo_;

    public:
        SharedAutomatonMemo() {
            // Insert the singleton final state.
            Store::set (State::finalState());
        }

        ~SharedAutomatonMemo() {
            // All states must have been removed from the memo before it is
            // destructed.
            assert (memo_.empty());

            // Remove the singleton final state.
            Store::remove (State::finalState());
        }

        /** \brief
        Retrieve the result of a remembered function call.
        */
        Automaton retrieve (UnionArguments const & arguments) const {
            auto position = memo_.find (arguments);

            if (position == memo_.end())
                return Automaton (math::zero <Weight>(), nullptr);

            if (position->second.second.template contains <StatePtr>())
                return Automaton (position->second.first,
                    rime::get <StatePtr> (position->second.second));
            else
                return Automaton (position->second.first,
                    rime::get <WeakStatePtr> (position->second.second).lock());
        }

        /** \brief
        Remember the result of the function call to be retrieved next time
        using retrieve().

        \pre \c arguments must not yet be in the memo.
        */
        void remember (UnionArguments const & arguments,
            Automaton const & result)
        {
            // Check whether the function has returned the left or the right
            // state argument itself.
            if (arguments.leftPointer() == result.state().get()
                || arguments.rightPointer() == result.state().get())
            {
                // One of the arguments is in there.
                // To prevent circular references, we want to save a weak_ptr.
                auto iteratorAndSuccess = memo_.insert (Mapping (arguments,
                    StoredResult (result.startWeight(),
                        WeakStatePtr (result.state()))));

                assert (iteratorAndSuccess.second);
                (void) iteratorAndSuccess;
            } else {
                // Store the shared_ptr.
                auto iteratorAndSuccess = memo_.insert (Mapping (arguments,
                    StoredResult (result.startWeight(), result.state())));
                assert (iteratorAndSuccess.second);
                (void) iteratorAndSuccess;
            }
        }

        /** \brief
        Notify the memo that a SharedState object is being destructed.

        Any memoised result where either the left or the right argument is a
        SharedState should therefore be removed from the memo.
        */
        void removeStatePointer (State const * statePointer) {
            // Removing a state pointer is a dangerous action.
            // When an entry from the memo is removed, the result may be the
            // last reference to that state.
            // This triggers the destruction of that state, which will
            // recursively call this method.
            // That situation must be handled carefully.

            // Collect all resulting state pointers that are removed in one
            // place.
            std::vector <StatePtr> garbage;

            // Put the results with statePointer as the left argument in
            // "garbage".
            auto & index1 = memo_.template get<1>();
            {
                auto r = index1.equal_range (statePointer);
                for (; r.first != r.second; ++ r.first) {
                    // If it is a shared_ptr, copy it: we do not want to mutate
                    // the memo while using the iterators.
                    auto pointer = r.first->second.second;
                    if (pointer.template contains <StatePtr>())
                        garbage.push_back (rime::get <StatePtr> (pointer));
                }
            }
            index1.erase (statePointer);

            // Put the results with statePointer as the right argument in
            // "garbage".
            auto & index2 = memo_.template get<2>();
            {
                auto r = index2.equal_range (statePointer);
                for (; r.first != r.second; ++ r.first) {
                    auto pointer = r.first->second.second;
                    if (pointer.template contains <StatePtr>())
                        garbage.push_back (rime::get <StatePtr> (pointer));
                }
            }

            // Erase the entries, which will now not trigger any use_count to
            // drop to 0.
            index2.erase (statePointer);

            // Now "garbage" is destructed, and ~SharedState may be called
            // recursively.
        }
    };

} // namespace flipsta

#endif // FLIPSTA_DETAIL_SHARED_AUTOMATON_STATE_MEMO_HPP_INCLUDED
