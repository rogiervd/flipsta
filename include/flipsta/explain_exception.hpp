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

#ifndef FLIPSTA_EXPLAIN_EXCEPTION_HPP_INCLUDED
#define FLIPSTA_EXPLAIN_EXCEPTION_HPP_INCLUDED

#include <boost/exception/exception.hpp>

namespace flipsta {

/** \brief
Give as much explanation about the exception as possible.

This is most useful for exceptions from reading files: exceptions generated
while manipulating automata often have information attached to them with types
that depend on the automaton that is being manipulated.
*/
void explainException (std::ostream &, boost::exception const &);

} // namespace flipsta

#endif // FLIPSTA_EXPLAIN_EXCEPTION_HPP_INCLUDED
