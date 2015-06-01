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
