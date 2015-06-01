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
