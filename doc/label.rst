.. _label:

******
Labels
******

.. highlight:: cpp

Every transition on a finite-state automaton has a *label*.
The label must be in a `semiring`_ .
A semiring is a magma under the operation ⊗ (:cpp:type:`math::times`) and a magma under the operation ⊕ (:cpp:type:`math::plus`), with a few requirements that make the interaction between these similar to, say, natural numbers.
The operation ⊗ is used to combine the labels of two arcs following each other; the operation ⊕ is used to combine the labels of two paths ending in the same state.
The semiring will often be a combination of a symbol sequence and a cost or probability.
For example, a speech recogniser essentially computes the path with the highest probability through an automaton, and the word sequence that goes with it.

Note that some tools, and some papers, assume specific configurations of these.
This library, on the other hand, is more flexible and general, bus also more explicit.

Some examples of semirings are:

*   ``float``.

    Represents a real, with normal times for ⊗, and normal plus for ⊕.

*   :cpp:class:`math::cost` ``<float>``.

    Represents a real with ``+`` for ⊗, and ``min`` for ⊕.
    This can be used to find the minimum cost in a finite-state automaton.

*   :cpp:class:`math::max_semiring` < :cpp:class:`math::log_float` <float>>.

    Represents a real (stored as the floating-point representation of its logarithm), with normal times for ⊗, and ``max`` for ⊕.
    This can be used to find the maximum probability in an automaton.

*   :cpp:class:`math::optional_sequence` <char> (which is a left semiring or optionally a right semiring).

    This can be used to represent sequences of symbols from an alphabet.
    ⊗ is implemented as concatenation of two sequences.
    :cpp:class:`math::optional_sequence` is a sequence of length 0 or 1, (applying :cpp:type:`math::times` results in a more general but less efficient object of type :cpp:class:`math::sequence`).
    ⊕ picks the longest common prefix (for the left sequence semiring) or suffix (for the right sequence semiring) of two sequences.
    This definition of ⊕ may seem strange at first blush.
    It turns out to make sense when performing some automaton operations (such as determinisation).
    However, to find a best path, it is useful to embed the sequence semiring in a lexicographical semiring.
    (The sequence's ⊕ operator is then bypassed.)

*   :cpp:class:`math::product` <math::over <int, float>>

    This semiring is the cartesian product of two semirings.
    (But you can use more, or fewer, component semirings.)
    ⊗ merely applies ⊗ to each of the components; and ⊕ similarly applies ⊕ to each of the components.

*   :cpp:class:`math::lexicographical` <math::over <
    :cpp:class:`math::max_semiring` < :cpp:class:`math::log_float` <float>>,
    :cpp:class:`math::optional_sequence` <char>>>

    This semiring combines a max semiring and a sequence semiring.
    ⊗ merely applies ⊗ to each of the components.
    The ⊕ operation, however, chooses the best of two values.
    Here, "best" is defined by picking the value with the "best" first component (in this), and if there is a tie, the "best" second component, et cetera.
    Hence the name "lexicographical" (coined by `Roark et al. (2011) <http://dl.acm.org/citation.cfm?id=2002738>`_).
    In this example, the first component is the only component that can meaningfully compared, and the second component is just a hanger-on.

    An automaton with this type of weight is a weighted finite state acceptor, that is, an automaton with one weight and one symbol tape.
    It is equivalent to some types of weighted finite state acceptor as defined by other libraries.

*   :cpp:class:`math::lexicographical` <math::over <
    :cpp:class:`math::cost` <float>,
    :cpp:class:`math::optional_sequence` <char>,
    :cpp:class:`math::optional_sequence` <char>>>

    This semiring combines a cost semiring and two sequence semirings.

    An automaton with this type of weight is a weighted finite state transducer, that is, an automaton with a weight and two symbol tapes.
    The `OpenFst <http://www.openfst.org/>`_ library is a popular library that views all automata as transducers, i.e. two-tape automata.
    (It even represents acceptors as two-tape automata with the exact same symbols on the two tapes.)

The composite labels :cpp:class:`math::product` and :cpp:class:`math::lexicographical` can be used with any number of components, and even recursively.
This makes this library quite a bit more flexible than its competitors, and opens up many new possibilities.

Descriptors
===========

For efficiency, it is often useful to represent labels differently internally than externally.
For example, sequences of symbols are represented internally by dense symbols, and an :cpp:class:`math::alphabet` keeps track of the mapping.
Conversion to compressed labels and back to expanded labels is done by the *descriptor*.
An automaton holds contains one descriptor for all the labels on its arcs.
For composite labels, composite descriptors are used, which forward to descriptors specific to the components.

.. doxygenstruct:: flipsta::label::DefaultDescriptorFor

.. doxygenstruct:: flipsta::label::CompressedLabelType


.. doxygenvariable:: flipsta::label::compress

.. doxygenvariable:: flipsta::label::expand

Descriptor types
----------------

Descriptor object define ``compress()`` and ``expand()`` methods that return a function object that converts between external and internal representations.
They can often be default-constructed, but it is sometimes possible to pass in parameters.
For example, :cpp:class:`flipsta::label::AlphabetDescriptor` produces its own alphabet if it is default-constructed.
Alternatively, a std::shared_ptr to an alphabet can be passed in; this alphabet can be shared with other descriptors.

The following descriptor types are predefined:

.. doxygenclass:: flipsta::label::NoDescriptor
    :members:

.. doxygenclass:: flipsta::label::AlphabetDescriptor
    :members:

.. doxygenclass:: flipsta::label::CompositeDescriptor
    :members:


Terminal labels
===============

Finite state automata have labels for transitions, but they can also have labels on states.
These states are *terminal states*, i.e. either start states or end states.
If a state has no terminal label, it is implicitly given a label with a semiring zero.
The type of label of states that do have a terminal label is often restricted.
For example, symbol sequences in terminal labels should usually be empty.
The default type (but not necessarily value) for terminal labels is therefore the type of semiring zero of the label type.
For symbol sequences, this type is :cpp:class:`math::empty_sequence`.
These labels can be converted using the descriptor, like the labels on the transitions.

Compile-time helpers
====================

.. doxygenstruct:: flipsta::label::GeneraliseToZero
.. doxygenstruct:: flipsta::label::GeneraliseSemiring
.. doxygenstruct:: flipsta::label::GetDefaultTerminalLabel

.. _semiring: http://en.wikipedia.org/wiki/Semiring
