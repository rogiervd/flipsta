/*
Copyright 2014, 2015 Rogier van Dalen.

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

/** \brief
Give as much explanation about the exception as possible.

This mostly deals
*/
void explainException (std::ostream &, boost::exception const &);

} // namespace flipsta

#endif // FLIPSTA_ERROR_HPP_INCLUDED
