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
Basic handling of automata.
*/

#ifndef FLIPSTA_CORE_CALLABLE_HPP_INCLUDED
#define FLIPSTA_CORE_CALLABLE_HPP_INCLUDED

#include <type_traits>

#include <boost/utility/enable_if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/identity.hpp>

#include "utility/returns.hpp"

#include "utility/nested_callable.hpp"

namespace flipsta {

namespace callable {

    template <template <class ...> class Apply, class ... CompileTimeArguments>
        struct Generic
    {
        template <class ... Arguments> struct apply
        : Apply <CompileTimeArguments ..., Arguments ...> {};

        template <class ... Arguments> auto
            operator() (Arguments && ... arguments) const
        -> typename std::result_of <apply <Arguments ...> (Arguments ...)>::type
        {
            return apply <Arguments ...>() (
                std::forward <Arguments> (arguments) ...);
        }
    };

} // namespace callable

namespace operation {

    /**
    Inherit from this class to indicate that the operation is not implemented
    (for specific arguments).
    */
    struct Unimplemented {};

    /**
    Metafunction that indicates whether an operation is implemented.
    \return \c true iff \a Operation is not derived from unimplemented.
    */
    template <class Operation> struct IsImplemented
    : boost::mpl::not_ <std::is_base_of <Unimplemented, Operation>> {};

} // namespace operation

namespace callable {

    template <class FunctionExpression> struct IsImplemented;
    template <class Callable, class ... Arguments>
        struct IsImplemented <Callable (Arguments...)>
    : operation::IsImplemented <typename std::decay <Callable>::type::template
        apply <Arguments...>>
    {};

} // namespace callable

/**
\brief Compile-time constant that returns whether a nested call expression has
been implemented for a particular set of parameters.
\tparam Expression
    The nested call expression of which the return type is sought.
*/
template <class Expression> struct Has
: nested_callable::all <callable::IsImplemented <boost::mpl::_1>, Expression>
{};

/**
\brief Find the result of a nested call expression.
\tparam Expression
    The nested call expression of which the return type is sought.
*/
template <class Expression> struct ResultOf
: nested_callable::result_of <Expression> {};

/**
\brief Find the decayed result of a nested call expression.
\tparam Expression
    The nested call expression of which the return type is sought.
*/
template <class Expression> struct DecayedResultOf
: std::decay <typename ResultOf <Expression>::type> {};

} // namespace flipsta

#endif // FLIPSTA_CORE_CALLABLE_HPP_INCLUDED
