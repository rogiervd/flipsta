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

    exposeSemiring();
    exposeAutomaton();
    exposeExceptions();
}
