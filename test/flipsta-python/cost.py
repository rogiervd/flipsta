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
