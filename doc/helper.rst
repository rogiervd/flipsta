.. _helper:

.. highlight:: cpp

**************
Helper classes
**************

There are various useful classes.

Keeping track of states
=======================

.. doxygenclass:: flipsta::Map
    :members:

.. doxygenclass:: flipsta::LifoQueue
    :members:

.. doxygenclass:: flipsta::Dense

Exception types
===============

The Flipsta library uses `Boost.Exception`_ for exception reporting.
The following shows how to throw exceptions::

    #include "flipsta/error.hpp"

    // ...

    // Throw error, attaching information about the state .
    throw flipsta::Error() << flipsta::errorInfoState <State> (5);

This is how they can be caught::

    #include <boost/exception/get_error_info.hpp>
    #include "flipsta/error.hpp"

    // ...

    try {
        // ... Operation that may throw an exception.
    } catch (boost::exception & error) // or catch a more specific type.
    {
        std::cerr << "An error has occurred.\n";
        // See whether information about the state is attached.
        State const * state = boost::get_error_info <
            flipsta::TagErrorInfoStateType <State>::type> (error);
        // If there is, report it.
        if (state)
            std::cerr << "The error concerned state " << *state << '\n';
    }

.. doxygenstruct:: flipsta::Error
.. doxygenstruct:: flipsta::StateNotFound
.. doxygenstruct:: flipsta::StateExists
.. doxygenstruct:: flipsta::AutomatonNotAcyclic
.. doxygenstruct:: flipsta::TagErrorInfoState
.. doxygenstruct:: flipsta::TagErrorInfoStateType


.. _Boost.Exception: http://www.boost.org/doc/libs/release/libs/exception/doc/boost-exception.html
