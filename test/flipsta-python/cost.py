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
An example semiring that keeps a cost.
+ chooses the minimum cost of two; * adds costs.
'''

from flipsta import Zero, One

class Cost:
    '''
    An example semiring which keeps a cost.
    '''
    def __init__ (self, value):
        self.value = float (value)

    def __eq__ (self, other):
        if other is Zero:
            return self.value == float ('inf')
        elif other is One:
            return self.value == 0
        else:
            return self.value == other.value

    def __ne__ (self, other):
        return not self == other

    def __add__ (self, other):
        if other is Zero:
            return self
        elif other is One:
            return Cost (min (self.value, 0))
        else:
            return Cost (min (self.value, other.value))

    def __radd__ (self, other):
        return self + other

    def __mul__ (self, other):
        if other is Zero:
            return Cost (float ('inf'))
        elif other is One:
            return self
        else:
            return Cost (self.value + other.value)

    # Multiplication is commutative (a*b == b*a) so forward to normal
    # multiplication.
    def __rmul__ (self, other):
        return self * other

    def __str__ (self):
        return str (self.value)
