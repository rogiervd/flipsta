.. _examining:

******************
Examining automata
******************

The following outlines ways of examining automata.

Most operations for examining automata are generic and rely on the intrinsic operations.
However, some operations can be optimised for certain types of automata, and then have different performance characteristics.

Drawing an automaton
====================

The Flipsta library cannot actually draw automata; but it can produce ``.dot`` files that are straightforward to convert into, say, PDF or SVG, using `Graphviz <http://www.graphviz.org/>`_ ``dot``.

.. doxygenfunction:: flipsta::draw

.. _traversal:

Traversing an automaton
=======================

For various algorithms it is important to traverse automata in a specific order.
For example, finding the shortest distance in an acyclic automaton traverses the automaton in topological order.
The standard algorithm for finding the topological order uses :ref:`depth-first traversal <depth_first_traversal>`.

The standard return value for a function that traverses an automaton is a, possibly lazy, range of states.
(Using callback functions would be possible, but this would be more cumbersome to use.)

.. _depth_first_traversal:

Depth-first traversal
---------------------

A basic way to traverse an automaton is depth-first search.

.. doxygenfunction:: flipsta::traverse

.. doxygenstruct:: flipsta::TraversedState
    :members:

.. doxygenenum:: flipsta::TraversalEvent
