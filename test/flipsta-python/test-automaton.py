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

class State:
    '''
    State whose identity depends on it being the same object.
    (Even though it's got a name.)
    '''
    def __init__ (self, name):
        self.name = name

    def __str__ (self):
        return str (self.name)

state2 = State ('two')

def make_automaton():

    automaton = flipsta.Automaton()

    assert (not automaton.has_state ("start"))
    assert (not automaton.has_state (3))

    automaton.add_state ("start")
    assert (automaton.has_state ("start"))
    assert (not automaton.has_state (3))

    assert (list (automaton.states()) == ["start"])

    automaton.add_state (1)
    automaton.add_state (state2)
    automaton.add_state (3)

    assert (list (automaton.states()) == ["start", 1, state2, 3])

    try:
        automaton.add_state (3)
        assert (False)
    except ValueError:
        pass

    automaton.add_arc ("start", 3, Cost (1.5))
    # Quick test that is used in the documentation.
    (arc,) = automaton.arcs_on (True, "start")
    assert (arc.state (False) == "start")
    assert (arc.state (True) == 3)
    assert (arc.label() == Cost (1.5))

    (arc,) = automaton.arcs_on (False, 3)
    assert (arc.state (False) == "start")
    assert (arc.state (True) == 3)
    assert (arc.label() == Cost (1.5))

    automaton.add_arc ("start", 1, Cost (2.))
    automaton.add_arc (1, state2, Cost (0.))
    automaton.add_arc (state2, 3, Cost (.5))

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

    arcs = list (automaton.arcs_on (False, 3))
    assert (len (arcs) == 2)
    assert (arcs [0].state (False) == state2)
    assert (arcs [0].state (True) == 3)
    assert (arcs [0].label() == Cost (0.5))
    assert (arcs [1].state (False) == "start")
    assert (arcs [1].state (True) == 3)
    assert (arcs [1].label() == Cost (1.5))

    arcs = list (automaton.arcs_on (True, "start"))
    assert (len (arcs) == 2)
    assert (arcs[0].state (False) == "start")
    assert (arcs[0].state (True) == 1)
    assert (arcs[0].label() == Cost (2.))
    assert (arcs[1].state (False) == "start")
    assert (arcs[1].state (True) == 3)
    assert (arcs[1].label() == Cost (1.5))

    try:
        automaton.set_terminal_label (True, 5, Cost (1.))
        assert (False);
    except KeyError:
        pass

    automaton.set_terminal_label (True, "start", Cost (1.))
    automaton.set_terminal_label (False, 3, Cost (2.))
    automaton.set_terminal_label (True, 1, Zero)

    assert (list (automaton.terminal_states (True)) == [("start", Cost (1.))])
    assert (list (automaton.terminal_states (False)) == [(3, Cost (2.))])

    assert (automaton.terminal_label (True, "start") == Cost (1.))
    assert (automaton.terminal_label (True, 1) == Zero)

    assert (automaton.terminal_label (False, "start") == Zero)
    assert (automaton.terminal_label (False, 3) != Zero)
    assert (automaton.terminal_label (False, 3) == Cost (2.))

    return automaton

def make_acyclic_automaton():

    automaton = make_automaton()

    automaton.add_arc (state2, 1, Cost (1.))
    return automaton

make_automaton()

# Test topological order.

topologically_ordered = make_automaton().topological_order()
assert (list (topologically_ordered) == ["start", 1, state2, 3])

topologically_ordered = make_automaton().topological_order (True)
assert (list (topologically_ordered) == ["start", 1, state2, 3])

topologically_ordered = make_automaton().topological_order (False)
assert (list (topologically_ordered) == [3, state2, 1, "start"])

# We don't actually want to call this every time.
# make_automaton().draw ("./temporary_automaton.dot", True)

try:
    topologically_ordered = make_acyclic_automaton().topological_order()
    # Exception should have been thrown.
    assert (False)
except RuntimeError:
    pass

# Test acyclic shortest distance.
distances = list (make_automaton().shortest_distance_acyclic_from ("start"))
assert (len (distances) == 4)
assert (distances [0] == ("start", Cost (0)))
assert (distances [1] == (1, Cost (2.)))
assert (distances [2] == (state2, Cost (2.)))
assert (distances [3] == (3, Cost (1.5)))

# The same, but with the initial states as an iterable.
distances = list (
    make_automaton().shortest_distance_acyclic ([("start", Cost (1))]))
assert (len (distances) == 4)
assert (distances [0] == ("start", Cost (1)))
assert (distances [1] == (1, Cost (3.)))
assert (distances [2] == (state2, Cost (3.)))
assert (distances [3] == (3, Cost (2.5)))

distances = list (make_automaton().shortest_distance_acyclic (
    [("start", Cost (.5)), (state2, Cost (1.))]))

assert (distances == [("start", Cost (.5)), (1, Cost (2.5)),
    (state2, Cost (1.)), (3, Cost (1.5))])

# Give initial_states as a (lazy) iterable instead of a list.
distances = list (make_automaton().shortest_distance_acyclic (
    ((state, Cost (value)) for state, value in [("start", .5), (state2, 1.)])))

assert (distances == [("start", Cost (.5)), (1, Cost (2.5)),
    (state2, Cost (1.)), (3, Cost (1.5))])
