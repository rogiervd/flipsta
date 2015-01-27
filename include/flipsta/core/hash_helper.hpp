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
    // For reasons unknown to me, Clang 3.5 does not seem to pick up that
    // definition.
#if (BOOST_VERSION < 105100) || defined (__clang__)
    template <class Pointee>
        inline std::size_t hash_value (std::shared_ptr <Pointee> const & p)
    {
        boost::hash <Pointee *> hasher;
        return hasher (p.get());
    }
#endif

} // namespace boost

#endif // FLIPSTA_CORE_HASH_HELPER_HPP_INCLUDED
