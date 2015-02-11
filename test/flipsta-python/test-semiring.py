# Copyright 2015 Rogier van Dalen.

# This file is part of the Flipsta library.

# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
for value in [float ('-inf'), -2.5, -1, 0, +0.5, 3, float ('-inf')]:
    assert (Cost (value) == Cost (value) * One == Cost (value))
    assert (Cost (value) == One * Cost (value) == Cost (value))

    assert (Cost (value) == Cost (value) + Zero == Cost (value))
    assert (Cost (value) == Zero + Cost (value) == Cost (value))

    assert (Zero == Cost (value) * Zero == Zero)
    assert (Zero == Zero * Cost (value) == Zero)

assert (Cost (float ('inf')) == Zero)
assert (Zero == Cost (float ('inf')))
assert (Cost (0) == One)
assert (One == Cost (0))

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
