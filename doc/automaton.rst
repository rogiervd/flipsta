.. _automaton:

********
Automata
********

There are many different automaton classes, and many of them can be composed.
This composition is often lazy, which helps performance.
Any automaton can be used using a number of operations.
(Not all operations are defined for all types of automata.)

For the complete API documentation, see `the Doxygen documentation <../../doxygen/html/index.html>`_.

The operations often show symmetry, since automata can be traversed in the direction of the arcs, or the opposite direction.
The direction is indicated with either of two classes:

.. doxygenstruct:: flipsta::Forward

.. doxygenstruct:: flipsta::Backward

.. doxygenvariable:: flipsta::forward
.. doxygenvariable:: flipsta::backward

Intrinsic operations
====================

The operations access an automaton immutably, i.e. without changing it.
They usually require that the automaton remains in memory and remains unchanged.

.. doxygenvariable:: flipsta::tag
.. doxygenvariable:: flipsta::states
.. doxygenvariable:: flipsta::hasState
.. doxygenvariable:: flipsta::terminalStates
.. doxygenvariable:: flipsta::terminalLabel
.. doxygenvariable:: flipsta::arcsOn
.. doxygenvariable:: flipsta::times

These operations deal with the internal representations of labels:

.. doxygenvariable:: flipsta::terminalStatesCompressed
.. doxygenvariable:: flipsta::terminalLabelCompressed
.. doxygenvariable:: flipsta::arcsOnCompressed

These compile-time helpers are defined:

.. doxygenstruct:: flipsta::AutomatonTag
.. doxygenstruct:: flipsta::IsAutomaton
.. doxygenstruct:: flipsta::StateType
.. doxygenstruct:: flipsta::LabelType
.. doxygenstruct:: flipsta::TagType
.. doxygenstruct:: flipsta::CompressedLabelType

A mutable automaton: ``Automaton``
==================================

:cpp:class:`flipsta::Automaton` is a mutable automaton type.

.. doxygenclass:: flipsta::Automaton
    :members:

An explicit arc type: ``ExplicitArc``
=====================================

:cpp:class:`flipsta::ExplicitArc` is the arc type used by :cpp:class:`flipsta::Automaton`.
It also shows the interface that any arc type must adhere to.

.. doxygenclass:: flipsta::ExplicitArc
    :members:
