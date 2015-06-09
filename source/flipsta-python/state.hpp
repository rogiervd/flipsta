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

#ifndef FLIPSTA_PYTHON_STATE_HPP_INCLUDED
#define FLIPSTA_PYTHON_STATE_HPP_INCLUDED

#include <boost/python/object.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/str.hpp>

namespace flipsta { namespace python {

    using boost::python::object;

    class State {
        object underlying_;
    public:
        explicit State (object const & underlying)
        : underlying_ (underlying) {}

        object const & underlying() const { return underlying_; }
    };

    // Comparison.
    inline bool operator== (State const & left, State const & right)
    { return left.underlying() == right.underlying(); }
    inline bool operator!= (State const & left, State const & right)
    { return left.underlying() != right.underlying(); }

    // Hash value.
    inline std::size_t hash_value (State const & s)
    { return std::size_t (PyObject_Hash (s.underlying().ptr())); }

    // Print.
    inline std::ostream & operator<< (std::ostream & os, State const & s) {
        std::string str = boost::python::extract <std::string> (
            boost::python::str (s.underlying()));
        return os << str;
    }

}} // namespace python::flipsta

#endif // FLIPSTA_PYTHON_STATE_HPP_INCLUDED
