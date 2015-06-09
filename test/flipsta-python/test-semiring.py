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
Test Zero and One, the singleton objects, and the Cost semiring implemented in
Python.
'''

from flipsta import Zero, One, ZeroType, OneType

assert (Zero is Zero)
assert (One is One)
assert (str (Zero) == "<Zero>")
assert (str (One) == "<One>")

assert (not Zero)
assert (One)

assert (ZeroType.instance == Zero)
assert (OneType.instance == One)

assert (Zero == Zero)
assert (One != Zero)
assert (Zero != One)
assert (One == One)

assert (not Zero != Zero)
assert (not One == Zero)
assert (not Zero == One)
assert (not One != One)

assert (Zero + Zero is Zero)
assert (Zero + One is One)
assert (One + Zero is One)

try:
    # Without knowledge about the semiring that we're going to use, this can't
    # work.
    One + One
    assert (False)
except TypeError:
    pass

assert (Zero * Zero is Zero)
assert (Zero * One is Zero)
assert (One * Zero is Zero)
assert (One * One is One)

#### Test the example semiring "Cost".
from cost import Cost

costInf = Cost (float ('inf'))

assert (Cost (1) == Cost (1))
assert (Cost (1) != Cost (2))

# Test the interaction with Zero and One.
examples = [-2.5, -1, 0, +0.5, 3, float ('+inf')]
for cost in (Cost (value) for value in examples):
    assert (cost == cost * One == cost)
    assert (cost == One * cost == cost)

    assert (cost == cost + Zero == cost)
    assert (cost == Zero + cost == cost)

    assert (Zero == cost * Zero == Zero)
    assert (Zero == Zero * cost == Zero)

assert (Cost (float ('inf')) == Zero)
assert (Zero == Cost (float ('inf')))
assert (Cost (0) == One)
assert (One == Cost (0))

assert (hash (Cost (float ('inf'))) == hash (Zero))
assert (hash (Cost (0)) == hash (One))

# Test that hash() is consistent with equality.
for value1 in examples:
    cost1 = Cost (value1)
    for value2 in examples:
        cost2 = Cost (value2)
        if value1 == value2:
            assert (cost1 == cost2)
            assert (not cost1 != cost2)
            assert (hash (cost1) == hash (cost2))
        else:
            assert (cost1 != cost2)
            assert (not cost1 == cost2)
            # This should almost always succeed if the implementation is
            # correct.
            # If it does not, changing the example values is a fine solution.
            assert (hash (cost1) != hash (cost2))

cost0Copy = Cost (0)
cost1Copy = Cost (1)

assert (Cost (0) == cost0Copy)
assert (Cost (0) == One)
assert (Cost (1) == cost1Copy)
assert (costInf == Zero)

# Add.
assert (Cost (0) + Cost (1) == Cost (0))
assert (Cost (2) + Cost (1) == Cost (1))

# Multiply.
assert (Cost (0) * Cost (1) == Cost (1))
assert (Cost (2) * Cost (1) == Cost (3))
