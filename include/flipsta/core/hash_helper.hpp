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

/** \file
Provide hash_value for types that Boost does not provide it for.
*/

#ifndef FLIPSTA_CORE_HASH_HELPER_HPP_INCLUDED
#define FLIPSTA_CORE_HASH_HELPER_HPP_INCLUDED

#include <memory>

#include <boost/version.hpp>
#include <boost/functional/hash_fwd.hpp>

namespace boost {

    // Support for std::shared_ptr was added in Boost 1.51.0.
#if (BOOST_VERSION < 105100) || defined (__clang__)
    template <class Pointee> struct hash <std::shared_ptr <Pointee>> {
        std::size_t operator() (std::shared_ptr <Pointee> const & p) const {
            boost::hash <Pointee *> hasher;
            return hasher (p.get());
        }
    };
#endif

} // namespace boost

#endif // FLIPSTA_CORE_HASH_HELPER_HPP_INCLUDED
