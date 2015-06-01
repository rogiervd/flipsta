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
