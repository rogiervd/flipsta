/*
Copyright 2015 Rogier van Dalen.

This file is part of Rogier van Dalen's Range library for C++.

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

#ifndef FLIPSTA_PYTHON_ARC_HPP_INCLUDED
#define FLIPSTA_PYTHON_ARC_HPP_INCLUDED

#include <type_traits>

#include <boost/python/class.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/copy_const_reference.hpp>

#include "utility/returns.hpp"

#include "range/transform.hpp"
#include "range/python/return_view.hpp"
#include "range/python/tuple.hpp"

#include "flipsta/automaton.hpp"

namespace flipsta { namespace python {

    template <class Arc> inline
        auto stateOnArc (Arc const & arc, bool start)
    -> decltype (arc.state (flipsta::forward))
    {
        if (start)
            return arc.state (flipsta::forward);
        else
            return arc.state (flipsta::backward);
    }

}} // namespace flipsta::python

template <class Arc> inline void registerArc (char const * name) {
    using namespace boost::python;
    class_ <Arc> (name, no_init)
        .def ("state", &flipsta::python::stateOnArc <Arc>,
            return_value_policy <copy_const_reference>())
        .def ("label", &Arc::label,
            return_value_policy <copy_const_reference>())
        ;
}

#endif // FLIPSTA_PYTHON_ARC_HPP_INCLUDED
