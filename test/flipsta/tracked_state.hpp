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

#ifndef FLIPSTA_TEST_TRACKED_STATE_HPP_INCLUDED
#define FLIPSTA_TEST_TRACKED_STATE_HPP_INCLUDED

#include "utility/test/tracked.hpp"

/**
A state type that behaves like an integer but also tracks copying, moving, et
cetera.
*/
class TrackedState {
    utility::tracked <int> value_;
public:
    TrackedState (utility::tracked_registry & registry, int value)
    : value_ (registry, value)
    { assert (value_.content() >= 0); }

    int value() const { return value_.content(); }

    operator int() const { return value_.content(); }
};

// Compute hash for Boost.Hash.
inline std::size_t hash_value (TrackedState const & s) {
    boost::hash <int> hasher;
    return hasher (s.value());
}

#endif // FLIPSTA_TEST_TRACKED_STATE_HPP_INCLUDED
