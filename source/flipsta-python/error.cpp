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

#include <Python.h>
#include <boost/python/exception_translator.hpp>

#include "flipsta/error.hpp"

#include "module.hpp"

namespace flipsta { namespace python {

    struct TranslateException {

        void operator() (flipsta::Error const & e) const {
            // Non-descriptive error.
            PyErr_SetString (PyExc_RuntimeError,
                "Error concerning finite-state automata");
        }

        void operator() (flipsta::StateNotFound const & e) const {
            PyErr_SetString (PyExc_KeyError, "State not found");
        }

        void operator() (flipsta::StateExists const & e) const {
            PyErr_SetString (PyExc_ValueError, "State exists already");
        }

        void operator() (flipsta::AutomatonNotAcyclic const & e) const {
            PyErr_SetString (PyExc_RuntimeError, "Automaton not acyclic");
        }

    };

}} // namespace flipsta::python

void exposeExceptions() {
    using namespace boost::python;
    using namespace flipsta::python;

    /*
    \todo Ideally the exception hierarchy would be copied in Python.
    However, this seems hardish.
    \todo It would be good to attach information, say, states, to Python
    exceptions.
    Putting them in the exception text would definitely be useful.
    */

    register_exception_translator <flipsta::Error> (
        TranslateException());
    register_exception_translator <flipsta::StateNotFound> (
        TranslateException());
    register_exception_translator <flipsta::StateExists> (
        TranslateException());
    register_exception_translator <flipsta::AutomatonNotAcyclic> (
        TranslateException());
}
