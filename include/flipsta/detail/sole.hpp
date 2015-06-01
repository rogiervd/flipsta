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

#ifndef FLIPSTA_DETAIL_SOLE_HPP_INCLUDED
#define FLIPSTA_DETAIL_SOLE_HPP_INCLUDED

#include <cassert>
#include <memory>
#include <type_traits>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include <boost/functional/hash.hpp>

namespace flipsta { namespace detail {

    template <class Value> class SoleStore;

    struct SoleBaseTag {};

    /** \brief
    Base class for objects to be stored in \ref SoleStore that require access
    to other objects in the store.

    This makes sure that the same store is used (and saves some memory).

    Sole needs access to the conversion from and to Value.
    The destructor is not virtual, so it is not safe to cast \a Value to
    \a SoleBase in general.
    The best way of ensuring this is to privately derive from this class, and
    to make it a friend.
    */
    template <class Value> class SoleBase : public SoleBaseTag {
    protected:
        typedef SoleStore <Value> Store;

    private:
        Store * store_;
        bool inStore_;

        void setInStore() { inStore_ = true; }

        friend class SoleStore <Value>;

        static std::shared_ptr <Value> construct (
            Store & store, Value const & value)
        {
            // We must be inserting into the correct store.
            assert (value.store_ == &store);
            auto sole = std::make_shared <Value> (value);
            sole->setInStore();
            return std::move (sole);
        }

        static std::shared_ptr <Value> construct (
            Store & store, Value && value)
        {
            // We must be inserting into the correct store.
            assert (value.store_ == &store);
            auto sole = std::make_shared <Value> (std::move (value));
            sole->setInStore();
            return std::move (sole);
        }

    protected:
        /** \brief
        Initialise with a pointer to the SoleStore object to use.
        */
        SoleBase (Store * store) : store_ (store), inStore_ (false) {}

        /** \brief
        Return a pointer to the store that is used.
        */
        Store * store() const { return store_; }

    public:
        ~SoleBase() {
            if (inStore_)
                store_->removePointer (static_cast <Value const *> (this));
        }
    };

    /** \brief
    Class used internally by SoleStore if Value does not derive from SoleBase.
    */
    template <class Value> class SoleValue {
        Value value_;

        typedef SoleStore <Value> Store;
        Store & store_;

    public:
        SoleValue (Store & store, Value const & value)
        : value_ (std::move (value)), store_ (store) {}

        SoleValue (Store & store, Value && value)
        : value_ (value), store_ (store) {}

        ~SoleValue() { store_.removePointer (&value_); }

        static std::shared_ptr <Value const> construct (
            Store & store, Value const & value)
        {
            auto sole = std::make_shared <SoleValue> (store, value);
            // Produce an aliasing pointer to the value.
            return std::shared_ptr <Value const> (
                std::move (sole), &sole->value_);
        }

        static std::shared_ptr <Value const> construct (
            Store & store, Value && value)
        {
            auto sole = std::make_shared <SoleValue> (store, std::move (value));
            // Produce an aliasing pointer to the value.
            return std::shared_ptr <Value const> (
                std::move (sole), &sole->value_);
        }
    };

    /** \brief
    This contains a weak_ptr which should remain able to be converted into a
    shared_ptr at all times: as soon as the object is destructed, this reference
    is first destructed.

    Its hash is computed as the hash of the object that it points to.
    */
    template <class Value> class WeakPtr {
        std::weak_ptr <Value const> object_;
        // This remains even when the object goes out of scope.
        Value const * rawPointer_;

    public:
        WeakPtr (std::shared_ptr <Value const> const & object)
        : object_ (object), rawPointer_ (object.get()) {}

        /** \brief
        Get a shared_ptr owning the object.

        This should work until the object gets deleted.
        */
        std::shared_ptr <Value const> get() const { return object_.lock(); }

        /** \brief
        Return a reference to the object.

        This should work until the object gets deleted.
        */
        Value const & value() const {
            // Check that the object exists.
            assert (!object_.expired());
            return *rawPointer_;
        }

        Value const * rawPointer() const { return rawPointer_; }
    };

    template <class Value> std::size_t hash_value (WeakPtr <Value> const & p) {
        boost::hash <Value> hasher;
        return hasher (p.value());
    }

    template <class Value> bool
        operator== (WeakPtr <Value> const & left, WeakPtr <Value> const & right)
    { return left.rawPointer() == right.rawPointer(); }

    struct EqualToWeakPtr {
        template <class Value>
            bool operator() (Value const & left, WeakPtr <Value> const & right)
            const
        { return left == right.value(); }

        template <class Value>
            bool operator() (WeakPtr <Value> const & left, Value const & right)
            const
        { return left.value() == right; }
    };

    /** \brief
    Keep track of unique objects for each value.

    When a new value is added, it is checked whether it is already in the store,
    in which case a std::shared_ptr to the previously stored value is returned.
    When the last std::shared_ptr to the value is destructed, the value is
    destructed and removed from the store.

    All std::shared_ptr objects to values in the store must therefore be
    destructed before the store itself is.

    It is possible to add a particular std::shared_ptr to the store if the value
    is not in the store yet.
    This can be useful for adding singleton pointers.
    Such a pointer must be explicitly removed from the store when it is
    destructed.

    Values must have support for Boost.Hash, and equality must be defined.

    Sometimes objects need to hold pointers to other objects in the same store
    In that case, the objects require access to the store themselves, and then
    they can be derived from SoleBase so they can use the pointer to the store
    which is kept anyway.
    Note that objects should not keep a std::shared_ptr to themselves, directly
    or indirectly; as usual, a weak_ptr should be used to break the cycle.

    This is not guaranteed to be thread-safe.

    \todo A number of optimisations would be possible.
    Areas to think of are:
    This class makes use of std::shared_ptr and its facilities.
    A custom pointer could save memory.
    For example, the weak_ptr is not really necessary if you believe that the
    program logic is sound.
    SoleBase could combine its boolean and the pointer to the store in one word.
    Hashes may be computed more often than necessary.
    */
    template <class Value> class SoleStore {
        typedef WeakPtr <Value> Pointer;

        /*
        Keep objects, indexed by:
        1. The actual value that is pointed to.
        2. The position of the sole value, so the object can remove itself.
        */
        typedef boost::multi_index_container <
            Pointer,
            boost::multi_index::indexed_by <
                boost::multi_index::hashed_unique <
                    boost::multi_index::identity <Pointer>>,
                boost::multi_index::hashed_unique <
                    boost::multi_index::const_mem_fun <
                        Pointer, Value const *, &Pointer::rawPointer>>
            >> Objects;

        Objects objects_;

        /**
        The actual type of object that will be allocated.
        */
        typedef typename std::conditional <
                std::is_base_of <SoleBaseTag, Value>::value,
                SoleBase <Value>, SoleValue <Value>>::type
            SoleType;

    public:
        SoleStore() {}

        ~SoleStore() {
            // All objects must have gone from memory or they'll try and remove
            // themselves when they are deleted.
            assert (objects_.empty());
        }

        /** \brief
        Retrieve the sole object with value \a value.

        If an object with this value is already in the store, return a pointer
        to that object.
        If not, return a pointer to a newly allocated object.
        */
        std::shared_ptr <Value const> get (Value const & value) {
            auto existing = objects_.find (value,
                boost::hash <Value>(), EqualToWeakPtr());
            if (existing != objects_.end())
                return existing->get();

            // The value is not in the store yet; insert it.
            auto sole = SoleType::construct (*this, value);
            Pointer newElement (sole);
            // Note that this insertion involves a mere copy of newElement into
            // another place in memory.
            // No objects will therefore be deleted, and no objects will
            // therefore attempt to remove themselves from the store.
            objects_.insert (std::move (newElement));

            return sole;
        }

        // Rvalue reference.
        std::shared_ptr <Value const> get (Value && value) {
            auto existing = objects_.find (value,
                boost::hash <Value>(), EqualToWeakPtr());
            if (existing != objects_.end())
                return existing->get();

            // The value is not in the store yet; insert it.
            auto sole = SoleType::construct (*this, std::move (value));
            Pointer newElement (sole);
            objects_.insert (std::move (newElement));

            return sole;
        }

        /** \brief
        Force the pointer for the object in store to be a specific pointer.

        This can be useful for singleton values.
        These usually must be removed from the store explicitly.

        \pre No object with this value is already in the store.
        */
        void set (std::shared_ptr <Value const> pointer) {
            // The value must not be in the store yet.
            assert (objects_.find (*pointer,
                boost::hash <Value>(), EqualToWeakPtr()) == objects_.end());

            objects_.insert (std::move (pointer));
        }

        /** \brief
        Remove the object at the pointer from the store.
        */
        void remove (std::shared_ptr <Value const> pointer) {
            this->removePointer (pointer.get());
        }

    private:
        friend class SoleBase <Value>;
        friend class SoleValue <Value>;

        /** \brief
        Remove the object from the store.

        This is called by the destructor of SoleBase or SoleValue.
        This may seem dangerous for two reasons:
        1.  A weak_ptr to an object will be destructed while the shared_ptr and
            the object itself is being destructed.
            However, this is a way of implementing std::enable_shared_from_this
            so this must be supported.
        2.  What if this is called in the middle of another operation on
            objects_?
            It will not.
            This is simply because this class performs mutating operations on
            objects_ in only two places.
            The one here only affects the shared_ptr control block.
            The one in get() does not affect anything.
        So this is actually safe.
        */
        void removePointer (Value const * pointer) {
           auto numberDeleted = objects_.template get <1>().erase (pointer);
           assert (numberDeleted == 1);
           (void) numberDeleted;
        }

    };

}} // namespace flipsta::detail

#endif // FLIPSTA_DETAIL_SOLE_HPP_INCLUDED
