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

#ifndef FLIPSTA_CORE_DENSE_HPP_INCLUDED
#define FLIPSTA_CORE_DENSE_HPP_INCLUDED

#include <limits>

#include <boost/functional/hash_fwd.hpp>

namespace flipsta {

/**
\brief Wrap an integer type promising that the space of values will be dense.

This means that all values will be non-negative, and the values that are used
will be dense and close to zero.

This allows parts of Flipsta to use, essentially, array indexing instead of
looking keys up in hash maps.

This class has implicit conversions to \a Type, comparison operators, and it
works with boost::hash.
*/
template <class Type> class Dense {
    static_assert (std::numeric_limits <Type>::is_specialized,
        "Dense can only be used with arithmetic types.");
    static_assert (std::numeric_limits <Type>::is_integer,
        "Dense can only be used with integer types.");

    Type value_;
public:
    Dense (Type value) : value_ (value)
    { assert (value_ >= 0); }

    Type value() const { return value_; }

    operator Type() const { return value_; }

    bool operator == (Dense const & that) const
    { return this->value_ == that.value_; }

    bool operator != (Dense const & that) const
    { return this->value_ != that.value_; }

    bool operator < (Dense const & that) const
    { return this->value_ < that.value_; }
    bool operator <= (Dense const & that) const
    { return this->value_ <= that.value_; }

    bool operator > (Dense const & that) const
    { return this->value_ > that.value_; }
    bool operator >= (Dense const & that) const
    { return this->value_ >= that.value_; }

};

// Compute hash for Boost.Hash.
template <class Type> inline std::size_t hash_value (Dense <Type> const & d) {
    boost::hash <Type> hasher;
    return hasher (d.value());
}

} // namespace flipsta

#endif // FLIPSTA_CORE_DENSE_HPP_INCLUDED
