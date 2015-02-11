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

/* \file
Declare functions to register automaton classes that can be called from the
module definition.
*/

#ifndef FLIPSTA_PYTHON_MODULE_HPP
#define FLIPSTA_PYTHON_MODULE_HPP

void exposeSemiring();
void exposeAutomaton();
void exposeExceptions();

#endif // FLIPSTA_PYTHON_MODULE_HPP
