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

    label.rst
    automaton.rst
    examining.rst

    defining.rst

.. rubric:: Design

Automata are directed graphs.
The nodes represent states, and the edges transitions, which will be called "arcs" for short.
On the arcs are *labels*, which can be symbol sequences, numerical weights, a product of various weights, or other things.
They must, however, be in a *semiring*.

An example automaton, which could represent the output of a speech recogniser, is:

..  image:: example.png
    :scale: 40 %
    :align: center

It has state A, B, C, and D.
The circle of state A is in bold, which indicates a start state.
State B has a double line, which indicates a final state.
This automaton probably represents the sequences

* *a a c* with probability ⅛.
* *a c c* with probability ⅜.
* *b a c* with probability ⅛.
* *b c c* with probability ⅜.

The overall probability has been assumed to be the product of the weights, but this could have been different.
The interpretation of these operation depends on the *semiring* that they are in.
In brief, a `semiring`_ defines an operation ⊗ which is applied to merge consecutive labels while traversing the automaton.
Here, ⊗ is defined as ×, normal times, so it just multiplies the weights.
(A semiring also defines an operation ⊕ that is applied to merge competing paths, but that is not used in this example.)

Incidentally, it is not just the weights that must be in a semiring, but also the symbol sequences.
Each of the symbols *a*, *b*, and *c* is a one-symbol sequence, and the operation ⊗ is defined as concatenating two sequences.
(The operation ⊕ is less easy to define.)
Thus, (*b*, ½) ⊗ (*a*, ¼) = (*b a*, ⅛), and (*b a*, ⅛) ⊗ (*c*, 1) = (*b a c*, ⅛), which is the label for one complete path.

The flexibility of the Flipsta library, compared to other libraries that deal with finite-state automata, derives in large part from its extreme view on the labels on the transitions between states of automata.
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

.. _semiring: http://en.wikipedia.org/wiki/Semiring
