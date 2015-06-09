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

#include <type_traits>
#include <fstream>

#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/errors.hpp>
#include <boost/python/register_ptr_to_python.hpp>

#include "utility/returns.hpp"

#include "range/transform.hpp"
#include "range/python/range.hpp"
#include "range/python/return_view.hpp"
#include "range/python/tuple.hpp"

#include "flipsta/automaton.hpp"

#include "flipsta/topological_order.hpp"
#include "flipsta/shortest_distance.hpp"
#include "flipsta/draw.hpp"

#include "module.hpp"
#include "state.hpp"
#include "semiring.hpp"
#include "arc.hpp"

/*
Many of the C++ operations need a thin wrapper to make them suitable for Python.
This is for any (combination) of these reasons:
-   The operation is not a function, so Boost.Python cannot find out the type.
-   The return type is a range with elements that must be converted to Python
    objects.
-   An input type is a range that needs to be converted from a Python iterator.
-   The operation specifies Forward or Backward, which in Python is a Bool.
    If the operation is not implemented in the C++ for one direction or the
    other, a ValueError is raised at run-time, but the binary should still
    compile.
*/

namespace flipsta { namespace python {

    template <class Automaton> inline auto states (Automaton const & automaton)
    RETURNS (flipsta::states (automaton));

    /// Wrapper for "setTerminalLabel" method, to deal with the direction.
    template <class Automaton> inline void setTerminalLabel (
        Automaton & automaton, bool forward,
        typename StateType <Automaton>::type const & state,
        typename LabelType <Automaton>::type const & label)
    {
        if (forward)
            automaton.setTerminalLabel (flipsta::forward, state, label);
        else
            automaton.setTerminalLabel (flipsta::backward, state, label);
    }

    /** \brief
    Callable class that always raises a Python ValueError exception.

    This is useful in allowing run-time values from Python where the C++ uses
    compile-time values that are disallowed.
    */
    template <class Result> struct RaiseValueError {
        template <class ... Arguments>
            Result operator() (Arguments const & ...) const
        {
            PyErr_SetString (PyExc_ValueError, "Invalid arguments for call");
            boost::python::throw_error_already_set();
            // Prevent warning about missing return statement.
            return *static_cast <
                typename std::add_pointer <Result>::type> (nullptr);
        }
    };

    /// Wrapper to deal with the return type of the \c terminalLabel function
    /// and to deal with the direction.
    template <class Automaton> inline typename LabelType <Automaton>::type
        terminalLabel (Automaton & automaton, bool forward,
            typename StateType <Automaton>::type const & state)
    {
        if (forward)
            return flipsta::terminalLabel (automaton, flipsta::forward, state);
        else
            return flipsta::terminalLabel (automaton, flipsta::backward, state);
    }

    /** \brief
    Wrapper to expose a callable adhering to the nested callable protocol.

    This indicates at compile-time whether an operation is implemented.
    In Python, there is no such thing as compile time, so if the operation turns
    out not to be implemented for certain parameters, raise a ValueError.

    This explicitly casts the return type to \a Result, or pretends to produce
    an object of type \a Result, if an exception is raised.
    */
    template <class Result, class Callable, class UnderlyingCallable = Callable>
        struct RaiseValueErrorIfUnimplemented
    {
        template <class ... Arguments> struct apply
        : std::conditional <
            flipsta::Has <UnderlyingCallable (Arguments ...)>::value,
            typename Callable::template apply <Arguments ...>,
            RaiseValueError <Result>>::type {};

        template <class ... Arguments>
            Result operator() (Arguments && ... arguments) const
        {
            return Result (apply <Arguments ...>() (
                std::forward <Arguments> (arguments) ...));
        }
    };

    /** \brief
    Function class that explicitly converts the argument to \a Target.
    */
    template <class Target> struct ConvertTo {
        template <class Type> Target operator() (Type && o) const
        { return Target (std::forward <Type> (o)); }
    };

    /** \brief
    Evaluate to an range::any_range type that contains objects of type
    \a Element and has just enough capabilities to expose the range to Python.
    */
    template <class Element> struct AnyRange {
        typedef range::any_range <Element,
            range::capability::unique_capabilities> type;
    };

    template <class Element, class Callable>
        struct ReturnAnyRangeImplementation
    {
        template <class ... Arguments> struct apply {
            typedef typename AnyRange <Element>::type Result;

            Result operator() (Arguments && ... arguments) const {
                return Result (range::transform (ConvertTo <Element>(),
                    Callable() (std::forward <Arguments> (arguments) ...)));
            }
        };

        template <class ... Arguments>
            auto operator() (Arguments && ... arguments) const
        RETURNS (apply <Arguments ...>() (
            std::forward <Arguments>  (arguments) ...));
    };

    /** \brief
    Wrap a callable to return an range::any_range with \a Element objects, but
    raise a ValueError instead if the callable is not implemented for the
    arguments given.
    */
    template <class Element, class Callable> struct ReturnAnyRange
    : RaiseValueErrorIfUnimplemented <typename AnyRange <Element>::type,
        ReturnAnyRangeImplementation <Element, Callable>, Callable> {};

    /**
    Wrap ac terminalStates to return an any_range and to deal with the
    direction.
    */
    template <class Automaton> inline
        typename AnyRange <boost::python::object>::type
            terminalStates (Automaton const & automaton, bool forward)
    {
        ReturnAnyRange <boost::python::object,
            flipsta::callable::TerminalStates> implementation;
        if (forward)
            return implementation (automaton, flipsta::forward);
        else
            return implementation (automaton, flipsta::backward);
    }

    /**
    Wrap \c arcsOn to return an any_range with \c ExplicitArc objects and to
    deal with the direction.
    */
    template <class Automaton> inline
        typename AnyRange <
            ExplicitArc <typename StateType <Automaton>::type,
                typename LabelType <Automaton>::type>
        >::type arcsOn (Automaton const & automaton, bool forward,
            typename StateType <Automaton>::type const & state)
    {
        typedef ExplicitArc <
            typename StateType <Automaton>::type,
            typename LabelType <Automaton>::type> Arc;
        ReturnAnyRange <Arc, flipsta::callable::ArcsOn> implementation;
        if (forward)
            return implementation (automaton, flipsta::forward, state);
        else
            return implementation (automaton, flipsta::backward, state);
    }

    /**
    Wrap \c topologicalOrder to return a range of states and to deal with the
    direction.
    */
    template <class Automaton>
        inline typename AnyRange <typename StateType <Automaton>::type>::type
        topologicalOrder (
            std::shared_ptr <Automaton> const & automaton, bool forward)
    {
        typedef typename AnyRange <typename StateType <Automaton>::type>::type
            Result;
        RaiseValueErrorIfUnimplemented <Result, callable::TopologicalOrder>
            implementation;
        if (forward)
            return implementation (automaton, flipsta::forward);
        else
            return implementation (automaton, flipsta::backward);
    }

    /**
    Wrap \c shortestDistanceAcyclicFrom to return a range of states and to deal
    with the direction.
    */
    template <class Automaton> inline
        typename AnyRange <boost::python::object>::type
            shortestDistanceAcyclicFrom (
                std::shared_ptr <Automaton> const & automaton,
                typename StateType <Automaton>::type const & state,
                bool forward)
    {
        ReturnAnyRange <boost::python::object,
            flipsta::callable::ShortestDistanceAcyclicFrom> implementation;
        if (forward)
            return implementation (automaton, state, flipsta::forward);
        else
            return implementation (automaton, state, flipsta::backward);
    }

    /**
    Wrap \c shortestDistanceAcyclic to return a range of states and to deal
    with the direction.

    This takes the range of initial states by value; this is necessary because
    Boost.Python does not yet deal with rvalue references.
    */
    template <class Automaton> inline
        typename AnyRange <boost::python::object>::type
            shortestDistanceAcyclic (
                std::shared_ptr <Automaton> const & automaton,
                range::python_range <range::python_range <
                    typename StateType <Automaton>::type,
                    typename LabelType <Automaton>::type>> initialStates,
                bool forward)
    {
        ReturnAnyRange <boost::python::object,
            flipsta::callable::ShortestDistanceAcyclic> implementation;
        if (forward)
            return implementation (
                automaton, std::move (initialStates), flipsta::forward);
        else
            return implementation (
                automaton, std::move (initialStates), flipsta::backward);
    }

    /** \brief
    Wrap \c draw to take a file name.

    \todo It would be even better if Python file objects could be turned
    into std::ostreams.
    */
    template <class Automaton> inline
        void draw (Automaton const & automaton, std::string fileName,
            bool horizontal)
    {
        std::ofstream os (fileName.c_str());
        flipsta::draw (os, automaton, horizontal);
    }

} // namespace python

/** \brief
Allow Boost.Python to deal with std::shared_ptr to Automaton objects.

This can be picked up because of argument-dependent lookup: Automaton is in this
namespace.
A general version of this function that can deal with all std::shared_ptr's
would be in namespace boost::python or namespace std, both unappetising choices.
It might also conflict with a later update of Boost.Python.
This overload will be picked over more general overloads, but that should do no
harm.
*/
template <class State, class Label>
inline Automaton <State, Label> * get_pointer (
    std::shared_ptr <Automaton <State, Label>> const & p)
{ return p.get(); }

} // namespace flipsta

namespace {
    /// Boost.Python return value policy that returns a view of the result
    /// while keeping the argument (e.g. the automaton) alive.
    template <std::size_t argumentIndex> struct ViewOfArgument
    : range::python::return_view <
        boost::python::with_custodian_and_ward_postcall <0, argumentIndex>> {};
}

void exposeAutomaton() {
    using boost::python::class_;
    using boost::python::arg;
    using namespace flipsta::python;
    using range::python_range;

    typedef flipsta::python::State State;
    typedef flipsta::python::Semiring Label;

    typedef flipsta::Automaton <State, Label> Automaton;

    registerArc <flipsta::ExplicitArc <State, Label>> ("Arc");

    // Return type of terminalStates and shortest distance.
    range::python::register_tuple <range::tuple <State, Label>>();

    // Argument type of shortestDistanceAcyclic.
    // The tuple.
    range::python::convert_object_to_range <python_range <State, Label>>();
    // The list of tuples.
    range::python::convert_object_to_range <
        python_range <python_range <State, Label>>>();

    // Make GCC 4.6 happy by first assigning this to a variable.
    auto statesFunction = &states <Automaton>;

    class_ <Automaton, std::shared_ptr <Automaton>> ("Automaton")
        .def ("add_state", &Automaton::addState)
        .def ("has_state", &Automaton::hasState)
        .def ("states", statesFunction, ViewOfArgument <1>())

        .def ("add_arc", &Automaton::addArc)
        .def ("arcs_on", &flipsta::python::arcsOn <Automaton>,
            ViewOfArgument <1>())

        .def ("set_terminal_label",
            &flipsta::python::setTerminalLabel <Automaton>)
        .def ("terminal_label", &flipsta::python::terminalLabel <Automaton>)
        .def ("terminal_states",
            &flipsta::python::terminalStates <Automaton>, ViewOfArgument <1>())

        .def ("topological_order",
            &flipsta::python::topologicalOrder <Automaton>,
            (arg ("forward") = true), ViewOfArgument <1>())
        .def ("shortest_distance_acyclic",
            &flipsta::python::shortestDistanceAcyclic <Automaton>,
            (arg ("initial_states"), arg ("forward") = true),
            ViewOfArgument <1>())
        .def ("shortest_distance_acyclic_from",
            &flipsta::python::shortestDistanceAcyclicFrom <Automaton>,
            (arg ("initial_state"), arg ("forward") = true),
            ViewOfArgument <1>())

        .def ("draw", &flipsta::python::draw <Automaton>,
            (arg ("file_name"), arg ("horizontal") = false))
        ;
}
