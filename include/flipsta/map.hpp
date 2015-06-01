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

#ifndef FLIPSTA_MAP_HPP_INCLUDED
#define FLIPSTA_MAP_HPP_INCLUDED

#include <unordered_map>
#include <vector>

#include <boost/functional/hash.hpp>

#include "rime/core.hpp"
#include "rime/assert.hpp"

#include "range/core.hpp"
#include "range/for_each.hpp"

#include "core/dense.hpp"

namespace flipsta {

template <class Key, class Value,
    bool hasDefault = false, bool alwaysContain = false>
class Map;

namespace map_detail {

    template <class Value, bool hasDefault> struct WithDefault;

    template <class Value> struct WithDefault <Value, false> {
    protected:
        WithDefault() {}

        // This should never be used.
        // A static assertion is in the constructor of Map.
        WithDefault (Value const &);

        typedef Value & DefaultReference;

        DefaultReference defaultValue() const {
            assert (false);
            return * static_cast <Value *> (nullptr);
        }
    };

    template <class Value> struct WithDefault <Value, true> {
    private:
        Value defaultValue_;
    protected:
        // This should never be used.
        // A static assertion is in the constructor of Map.
        WithDefault();

        WithDefault (Value const & defaultValue)
        : defaultValue_ (defaultValue) {}

        typedef Value const & DefaultReference;

        DefaultReference defaultValue() const { return defaultValue_; }
    };

    // Closure that inserts the data into the map.
    template <class Map> struct InsertKeyValue {
        Map & map_;

        InsertKeyValue (Map & map) : map_ (map) {}

        template <class KeyValue>
            void operator() (KeyValue && key_value) const
        {
            rime::assert_ (range::size (key_value) == 2);
            map_.set (range::first (key_value), range::second (key_value));
        }
    };

} // namespace map_detail

/**
\brief Associative map from \a Key to \a Value.

This performs roughly the same function as standard unordered associative
containers, but with a different interface.

All operations are amortised constant time.

Normally this wraps a std::unordered_map.
However, if \a hasDefault and \a alwaysContain are both true, and \a Key is
\c Dense<>, then a std::vector is used.
This should be faster by a constant factor in cases where a key space is dense.

\tparam Key
    The type of the key.
    \c boost::hash must be implemented for this type.
\tparam Value
    The type of the value.
\tparam hasDefault
    If true, there is a default value, which will be assumed for keys that are
    not in the map.
    This behaviour is well-known from std::map, where default-construction is
    used.
    Here, on the other hand, the default value is specified and stored
    explicitly.
    If \a hasDefault == \c true, \c operator[] returns a const reference,
    because it might return a reference to the default value.
\tparam alwaysContain
    Hint that most keys that will be inserted will actually be dense, starting
    close to zero.
    If \a Key type is dense, this may then use a vector instead of a hash map.
    This gives a constant-factor speedup.
*/
template <class Key, class Value, bool hasDefault, bool alwaysContain>
    class Map : map_detail::WithDefault <Value, hasDefault>
{
    typedef std::unordered_map <Key, Value, boost::hash <Key>> Data;
    Data data_;
    typedef map_detail::WithDefault <Value, hasDefault> WithDefault;
    static_assert (!alwaysContain || hasDefault,
        "If alwaysContain is true, hasDefault must be true.");

public:
    /**
    \brief Initialise empty.

    This is only available for <c>hasDefault == false</c>.
    */
    Map() {
        static_assert (!hasDefault,
            "You must pass a default value when constructing this Map.");
    }

    /**
    \brief Initialise with a default value.

    This is only available for <c>hasDefault == true</c>.
    \param defaultValue
        The value that will be returned for keys that are not in the map.
    */
    Map (Value const & defaultValue) : WithDefault (defaultValue) {
        static_assert (hasDefault,
            "You cannot pass a default value when constructing this Map.");
    }

    /**
    \brief Initialise with the initial values in \a initialValues.

    This is only available for <c>hasDefault == false</c>.
    \param initialValues
        Initial content of the map, in the form of a range of key-value pairs.
        If any key occurs more than once, the last entry is retained.
    */
    template <class Range, class Enable = typename
        boost::enable_if <range::is_range <Range>>::type>
    Map (Range && initialValues)
    {
        static_assert (!hasDefault,
            "You must pass a default value when constructing this Map.");
        range::for_each (map_detail::InsertKeyValue <Map> (*this),
            std::forward <Range> (initialValues));
    }

    /**
    \brief Initialise with a default value, and with the initial values in
    \a initialValues.

    This is only available for <c>hasDefault == true</c>.
    \param defaultValue
        The value that will be returned for keys that are not in the map.
    \param initialValues
        Initial content of the map, in the form of a range of key-value pairs.
        If any key occurs more than once, the last entry is retained.
    */
    template <class Range, class Enable = typename
        boost::enable_if <range::is_range <Range>>::type>
    Map (Value const & defaultValue, Range && initialValues)
    : WithDefault (defaultValue) {
        static_assert (hasDefault,
            "You cannot pass a default value when constructing this Map.");
        range::for_each (map_detail::InsertKeyValue <Map> (*this),
            std::forward <Range> (initialValues));
    }

    /**
    \brief Return Whether the map contains \a key.

    If <c>alwaysContain == true</c>, the return type is \c rime::true_type.
    */
    /// \cond DONT_DOCUMENT
    template <bool alwaysContain2 = alwaysContain>
        typename std::enable_if <!alwaysContain2,
    /// \endcond
        bool
    /// \cond DONT_DOCUMENT
        >::type
    /// \endcond
            contains (Key const & key) const
    { return data_.count (key) != 0; }

    /// \cond DONT_DOCUMENT
    template <bool alwaysContain2 = alwaysContain>
        typename std::enable_if <alwaysContain2, rime::true_type>::type
            contains (Key const &) const
    { return rime::true_; }
    /// \endcond

    /**
    \brief Set the value associated to key \a key to \a value.

    If the key is not in the map, it is inserted.
    If the key is already in the map, the value is replaced by \a value.
    */
    void set (Key const & key, Value const &value) {
        // emplace is not implemented on GCC 4.6.
        auto result = data_.insert (std::make_pair (key, value));
        if (!result.second)
            result.first->second = value;
    }

    /**
    \brief Return a const-reference to the value corresponding to \a key.
    \pre If \c !hasDefault: \a key must be already in the map.
    */
    Value const & operator[] (Key const & key) const {
        auto position = data_.find (key);
        if (position == data_.end())
            return this->defaultValue();
        else
            return position->second;
    }

    /**
    \brief Return a reference to the value corresponding to \a key.

    If \c hasDefault, this is a const reference, because it may be to the
    default value.
    \pre If \c !hasDefault: \a key must be already in the map.
    */
    typename WithDefault::DefaultReference operator[] (Key const & key) {
        auto position = data_.find (key);
        if (position == data_.end())
            return this->defaultValue();
        else
            return position->second;
    }

    /**
    \brief Remove the value from the map.

    If <c>alwaysContain == true</c>, reset the value to the default.
    \pre \a key must be in the map.
    */
    void remove (Key const & key) { data_.erase (key); }
};

/// \cond DONT_DOCUMENT
// For Dense <Key> with a dense cover: optimise using a std::vector.
template <class Key, class Value>
    class Map <Dense <Key>, Value, true, true>
{
    typedef std::vector <Value> Data;
    Data data_;
    Value defaultValue_;

public:
    Map (Value const & defaultValue) : defaultValue_ (defaultValue) {}

    template <class Range, class Enable = typename
        boost::enable_if <range::is_range <Range>>::type>
    Map (Value const & defaultValue, Range && initialValues)
    : defaultValue_ (defaultValue) {
        range::for_each (map_detail::InsertKeyValue <Map> (*this),
            std::forward <Range> (initialValues));
    }

    rime::true_type contains (Dense <Key> const & key) const
    { return rime::true_; }

    void set (Dense <Key> const & key_, Value const & value) {
        std::size_t key (key_.value());
        if (data_.size() <= key)
            data_.resize (key + 1, defaultValue_);
        assert (key <= data_.size());
        data_ [key] = value;
    }

    Value const & operator[] (Dense <Key> const & key_) const {
        std::size_t key (key_.value());
        if (key < data_.size())
            return data_ [key];
        else
            return defaultValue_;
    }

    void remove (Dense <Key> const & key_) {
        std::size_t key (key_.value());
        if (key < data_.size())
            data_ [key] = defaultValue_;
        // Otherwise, it is already explicitly the default value.
    }
};
/// \endcond

} // namespace flipsta

#endif // FLIPSTA_MAP_HPP_INCLUDED
