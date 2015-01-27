***************
Flipsta library
***************

The Flipsta library deals with finite-state automata.
These are concise representations of, say, many word sequences, with probabilities attached to them.
Many algorithms in text and speech processing can be expressed in terms of a handful of automaton operations.

The name "Flipsta" evokes rapid state changes.
Also, it cleverly hides the acronym for "finite-state automaton" in plain view:
**F**\ lip\ **s**\ t\ **a**.

.. toctree::
    :maxdepth: 2

.. rubric:: Design

There are a number of libraries that deal with finite-state automata.
The Flipsta library is more flexible.
It derives its flexibility from an extreme view on the *labels* on the transitions between states of automata.
(These labels are the thing we usually care about.)
Other libraries fix the number of symbol sequences to 1 (to get finite-state *acceptors*) or 2 (to get finite-state *transducers*), and zero or one weight.
Through these are fine choices for many applications, many require an *n*-tape automaton with *n*>2.
Whole papers are written about how to do this.
Indeed, the OpenFst library allows users to work around the 2-tape restriction by moving extra symbols into the weights.
(They then become awkward to manipulate.)

The Flipsta library circumvents this by allowing only one *label*, which must be in a semiring, but by allowing it to be composite.
Though C++11 variadic templates (and elaborate machinery underneath that manipulates the types), any combination and number of symbols and more traditional (or more outlandish) semirings can be used as labels.
Three-tape automata are thus just one line of code away.

.. rubric:: Other libraries

Other libraries that deal with finite-state automata are:

*   `OpenFst <http://www.openfst.org/>`_, which views all automata as transducers, i.e. 2-tape automata, from Google.
*   `AT&T FSM Library <http://www2.research.att.com/~fsmtools/fsm/>`_, a predecessor of OpenFst.
*   `The RWTH FSA Toolkit <http://www-i6.informatik.rwth-aachen.de/~kanthak/fsa.html>`_, from RWTH Aachen University.
