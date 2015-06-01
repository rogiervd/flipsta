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
