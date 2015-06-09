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

#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/def.hpp>

#include "range/python/return_view.hpp"

#include "math/arithmetic_magma.hpp"

#include "flipsta/automaton.hpp"

#include "module.hpp"

BOOST_PYTHON_MODULE (flipsta) {
    using namespace boost::python;

    range::python::initialise_iterator();

    exposeState();
    exposeSemiring();
    exposeAutomaton();
    exposeExceptions();
}
