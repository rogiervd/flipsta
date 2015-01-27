/*
Copyright 2014, 2015 Rogier van Dalen.

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

/** \file
Error classes.
*/

#ifndef FLIPSTA_ERROR_HPP_INCLUDED
#define FLIPSTA_ERROR_HPP_INCLUDED

#include <stdexcept>

#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>

namespace flipsta {

/* Exceptions. */

/**
\brief Base exception class for Flipsta.
*/
struct Error : virtual std::exception, virtual boost::exception {};

/**
\brief Exception that indicates that a state was not found.
*/
struct StateNotFound : virtual Error {};

/**
\brief Exception that indicates that a state already exists.
*/
struct StateExists : virtual Error {};

/**
\brief Exception that indicates that an automaton is not acyclic.
*/
struct AutomatonNotAcyclic : virtual Error {};


/* boost::error_info tags. */

/// Tag for boost::error_info that indicates a state id.
template <class State> struct TagErrorInfoState;

template <class State> struct TagErrorInfoStateType
{ typedef boost::error_info <TagErrorInfoState <State>, State> type; };

/**
\brief Return a boost::error_info that holds a state identifier.

This can be attached to a boost::exception object.
*/
template <class State> inline
    typename TagErrorInfoStateType <State>::type
    errorInfoState (State const & state)
{ return typename TagErrorInfoStateType <State>::type (state); }

} // namespace flipsta

#endif // FLIPSTA_ERROR_HPP_INCLUDED
