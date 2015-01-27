.. _examinig:

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
