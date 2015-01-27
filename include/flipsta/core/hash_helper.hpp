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
Basic handling of automata.
*/

#ifndef FLIPSTA_CORE_HASH_HELPER_HPP_INCLUDED
#define FLIPSTA_CORE_HASH_HELPER_HPP_INCLUDED

#include <memory>

#include <boost/functional/hash_fwd.hpp>

namespace boost {

    template <class Pointee>
        inline std::size_t hash_value (std::shared_ptr <Pointee> const & p)
    {
        boost::hash <Pointee *> hasher;
        return hasher (p.get());
    }

} // namespace boost

#endif // FLIPSTA_CORE_HASH_HELPER_HPP_INCLUDED
