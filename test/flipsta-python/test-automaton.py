# Copyright 2015 Rogier van Dalen.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import flipsta
from flipsta import Zero, One
from cost import Cost

def make_automaton():

    automaton = flipsta.Automaton()

    assert (not automaton.has_state (0))
    assert (not automaton.has_state (3))

    automaton.add_state (0)
    assert (automaton.has_state (0))
    assert (not automaton.has_state (3))

    assert (list (automaton.states()) == [0])

    automaton.add_state (1)
    automaton.add_state (2)
    automaton.add_state (3)

    assert (list (automaton.states()) == [0, 1, 2, 3])

    try:
        automaton.add_state (3)
        assert (False)
    except ValueError:
        pass

    automaton.add_arc (0, 3, Cost (1.5))
    # Quick test that is used in the documentation.
    (arc,) = automaton.arcs_on (True, 0)
    assert (arc.state (False) == 0)
    assert (arc.state (True) == 3)
    assert (arc.label() == Cost (1.5))

    automaton.add_arc (0, 1, Cost (2.))
    automaton.add_arc (1, 2, Cost (0.))
    automaton.add_arc (2, 3, Cost (.5))

    # def printArc (arc):
    #     print arc.state (False), arc.state (True), arc.label().value

    # for state in automaton.states():
    #     print '***', state
    #     print '  in:'
    #     for arc in automaton.arcs_on (False, state):
    #         print '    ',
    #         printArc (arc)
    #     print '  out:'
    #     for arc in automaton.arcs_on (True, state):
    #         print '    ',
    #         printArc (arc)

    arcs = list (automaton.arcs_on (3, False))
    assert (len (arcs) == 2)
    assert (arcs [0].state (False) == 0)
    assert (arcs [0].state (True) == 1)
    assert (arcs [0].label() == Cost (2.))
    assert (arcs [1].state (False) == 0)
    assert (arcs [1].state (True) == 3)
    assert (arcs [1].label() == Cost (1.5))

    arcs = list (automaton.arcs_on (True, 0))
    assert (len (arcs) == 2)
    assert (arcs[0].state (False) == 0)
    assert (arcs[0].state (True) == 1)
    assert (arcs[0].label() == Cost (2.))
    assert (arcs[1].state (False) == 0)
    assert (arcs[1].state (True) == 3)
    assert (arcs[1].label() == Cost (1.5))

    try:
        automaton.set_terminal_label (True, 5, Cost (1.))
        assert (False);
    except KeyError:
        pass

    automaton.set_terminal_label (True, 0, Cost (1.))
    automaton.set_terminal_label (False, 3, Cost (2.))
    automaton.set_terminal_label (True, 1, Zero)

    assert (list (automaton.terminal_states (True)) == [(0, Cost (1.))])
    assert (list (automaton.terminal_states (False)) == [(3, Cost (2.))])

    assert (automaton.terminal_label (True, 0) == Cost (1.))
    assert (automaton.terminal_label (True, 1) == Zero)

    assert (automaton.terminal_label (False, 0) == Zero)
    assert (automaton.terminal_label (False, 3) != Zero)
    assert (automaton.terminal_label (False, 3) == Cost (2.))

    return automaton

def make_acyclic_automaton():

    automaton = make_automaton()

    automaton.add_arc (2, 1, Cost (1.))
    return automaton

make_automaton()

# Test topological order.

topologically_ordered = make_automaton().topological_order()
assert (list (topologically_ordered) == [0, 1, 2, 3])

topologically_ordered = make_automaton().topological_order (True)
assert (list (topologically_ordered) == [0, 1, 2, 3])

topologically_ordered = make_automaton().topological_order (False)
assert (list (topologically_ordered) == [3, 2, 1, 0])

# We don't actually want to call this every time.
# make_automaton().draw ("./temporary_automaton.dot", True)

try:
    topologically_ordered = make_acyclic_automaton().topological_order()
    # Exception should have been thrown.
    assert (False)
except RuntimeError:
    pass

# Test acyclic shortest distance.

distances = list (make_automaton().shortest_distance_acyclic_from (0))
assert (len (distances) == 4)
assert (distances [0] == (0, Cost (0)))
assert (distances [1] == (1, Cost (2.)))
assert (distances [2] == (2, Cost (2.)))
assert (distances [3] == (3, Cost (1.5)))

# The same, but with the initial states as an iterable.
distances = list (make_automaton().shortest_distance_acyclic ([(0, Cost (1))]))
assert (len (distances) == 4)
assert (distances [0] == (0, Cost (1)))
assert (distances [1] == (1, Cost (3.)))
assert (distances [2] == (2, Cost (3.)))
assert (distances [3] == (3, Cost (2.5)))

distances = list (make_automaton().shortest_distance_acyclic (
    [(0, Cost (.5)), (2, Cost (1.))]))

assert (distances == [
    (0, Cost (.5)), (1, Cost (2.5)), (2, Cost (1.)), (3, Cost (1.5))])

# Give initial_states as a (lazy) iterable instead of a list.
distances = list (make_automaton().shortest_distance_acyclic (
    ((state, Cost (value)) for state, value in [(0, .5), (2, 1.)])))

assert (distances == [
    (0, Cost (.5)), (1, Cost (2.5)), (2, Cost (1.)), (3, Cost (1.5))])
