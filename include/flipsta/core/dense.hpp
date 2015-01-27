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

#ifndef FLIPSTA_CORE_DENSE_HPP_INCLUDED
#define FLIPSTA_CORE_DENSE_HPP_INCLUDED

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
