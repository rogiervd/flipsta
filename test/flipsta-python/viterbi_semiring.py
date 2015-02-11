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
An example semiring that makes a shortest-distance algorithm the Viterbi
algorithm.
'''

from flipsta import Zero, One

class ViterbiSemiring:
    '''
    An example semiring that keeps a probability and a symbol sequence
    associated with it.
    + chooses the maximum-probability of two; * multiplies the probabilities and
    concatenates the symbols.
    '''
    def __init__ (self, probability, symbols = []):
        assert (probability >= 0)
        self.probability = probability
        self.symbols = symbols

    def __eq__ (self, other):
        if other is Zero:
            return self.probability == 0.
        elif other is One:
            return self.probability == 1. and self.symbols == []
        else:
            return self.probability == other.probability and \
                self.symbols == other.symbols

    def __add__ (self, other):
        if other is Zero:
            return self
        elif other is One:
            if self.probability < 1:
                return other
            else:
                return ViterbiSemiring (self.probability, self.symbols)
        else:
            if self.probability < other.probability:
                return other
            else:
                return self

    def __radd__ (self, other):
        return self + other

    def __mul__ (self, other):
        if other is Zero:
            return Zero
        elif other is One:
            return self
        else:
            return ViterbiSemiring (self.probability * other.probability,
                self.symbols + other.symbols)

    def __rmul__ (self, other):
        if other is Zero:
            return Zero
        elif other is One:
            return self
        else:
            return NotImplemented

    def __str__ (self):
        return '(%s, [%s])' % (self.probability, ', '.join (self.symbols))

# Quick test of the Viterbi semiring.

if __name__ == '__main__':
    assert (ViterbiSemiring (0) == Zero)
    assert (ViterbiSemiring (1) == One)
    assert (Zero == ViterbiSemiring (0))
    assert (One == ViterbiSemiring (1))

    assert (ViterbiSemiring (.5, ['a']) + Zero == ViterbiSemiring (.5, ['a']))
    assert (Zero + ViterbiSemiring (.5, ['a']) == ViterbiSemiring (.5, ['a']))
    assert (ViterbiSemiring (.5) * Zero == Zero)
    assert (Zero * ViterbiSemiring (.5) == Zero)

    assert (ViterbiSemiring (.5) * One == ViterbiSemiring (.5))
    assert (One * ViterbiSemiring (.5) == ViterbiSemiring (.5))
    assert (ViterbiSemiring (.5, ['b']) * One == ViterbiSemiring (.5, ['b']))
    assert (One * ViterbiSemiring (.5, ['b']) == ViterbiSemiring (.5, ['b']))

    assert (ViterbiSemiring (.5, ['a']) * ViterbiSemiring (.25, ['b'])
        == ViterbiSemiring (.125, ['a', 'b']))
