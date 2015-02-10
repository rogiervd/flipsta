.. _shortest_distance:

****************************
Shortest-distance algorithms
****************************

Shortest-distance algorithms are the workhorse for computations with finite-state automata.
The "shortest distance" that these algorithms compute is, in general, the ⊕-sum of the labels of all paths from one state to another.
To make computing this sum feasible, the labels on the arcs of the automaton must have certain properties.
These properties man that the labels must be in a semiring.
That explains that requirement.

Which semiring the labels are in determines what the computation does exactly.
If the ⊕ operation chooses the minimum of the two values (like math::cost does), this computes the minimum-value path.
If this value stands for a distance, this is actually the shortest distance, which explains the name of the algorithm.
(In this case, the shortest distance also corresponds to a shortest path through the automaton.)
However, depending on the semiring the ⊕-sum might be the overall probability, a weighted error, statistics for re-estimating parameters of a model, or something else.

There are various algorithms for computing shortest distances, which have different requirements on the topology and semirings, and have different characteristics.
The most important distinction is whether they compute the sum over paths from one source state to all other states (single-source shortest-distance algorithms) or from all source states to all target states (all-pairs shortest-distance algorithms).
Not all of these are currently implemented.

Single-source shortest distance
===============================

If the automaton is acyclic, the following algorithm can be used:

.. doxygenvariable:: flipsta::shortestDistanceAcyclic
.. doxygenvariable:: flipsta::shortestDistanceAcyclicFrom
