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
