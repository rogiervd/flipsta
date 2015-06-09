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

'''
Check properties of the C++ State and Semiring classes, which wrap Python
objects.
'''

import flipsta

from check_objects import *
from cost import Cost

class State:
    def __init__ (self, name):
        self.name = name

    def __str__ (self):
        return '%s (0x%x)' % (self.name, id (self))

# Two different objects, even though they have the same values.
state1 = State ('final')
state2 = State ('final')

assert (state1 != state2)

equal_count = check_states (['start', 5, 7.5, 5, state1, state2])
# Eight pairs are the same: each object with itself, and the two fives two
# ways around.
assert (equal_count == 8)

equal_count = check_semiring ([Cost (0), flipsta.One], False)
assert (equal_count == 4)

equal_count = check_semiring ([Cost (float ('inf')), flipsta.Zero], True)
assert (equal_count == 4)

equal_count = check_semiring ([Cost (3.5), Cost (3.5)], True)
assert (equal_count == 4)

equal_count = check_semiring ([Cost (0), Cost (float ('inf')),
    Cost (-3.5), Cost (7.25), Cost (-3.5)], True)
