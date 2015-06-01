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
from viterbi_semiring import ViterbiSemiring

def make_automaton():
    automaton = flipsta.Automaton()

    automaton.add_state (1)
    automaton.add_state (2)
    automaton.add_state (3)
    automaton.add_state (4)
    automaton.add_state (5)
    automaton.add_state (6)

    automaton.add_arc (1, 2, ViterbiSemiring (5/8., ['a']))
    automaton.add_arc (2, 3, ViterbiSemiring (1/2., ['b']))
    automaton.add_arc (3, 6, ViterbiSemiring (1., ['c']))

    automaton.add_arc (1, 4, ViterbiSemiring (3./8, ['e']))
    automaton.add_arc (4, 5, ViterbiSemiring (1., ['f']))
    automaton.add_arc (5, 6, ViterbiSemiring (1., ['c']))

    automaton.add_arc (2, 5, ViterbiSemiring (1./8, ['b']))
    automaton.add_arc (2, 5, ViterbiSemiring (3./8, ['d']))

    automaton.set_terminal_label (True, 1, One)
    automaton.set_terminal_label (False, 6, One)

    return automaton

automaton = make_automaton()
for (state, label) in automaton.shortest_distance_acyclic_from (1):
    print state, label
assert (state == 6)
assert (label == ViterbiSemiring (3./8, ['e', 'f', 'c']))

# Backward: the result should be the same.
for (state, label) in automaton.shortest_distance_acyclic_from (6, False):
    print state, label
assert (state == 1)
assert (label == ViterbiSemiring (3./8, ['e', 'f', 'c']))
