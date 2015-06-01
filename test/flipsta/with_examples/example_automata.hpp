/*
Copyright 2015 Rogier van Dalen.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/** \file
Example automata.
If you would like to see these graphically, run test-draw-examples from the
unit tests and pass in output .dot file names.
Then run Graphviz dot to turn the .dot files into a drawing.
*/

#ifndef FLIPSTA_TEST_EXAMPLE_AUTOMATA_HPP_INCLUDED
#define FLIPSTA_TEST_EXAMPLE_AUTOMATA_HPP_INCLUDED

#include <memory>
#include <string>

#include "math/cost.hpp"
#include "math/lexicographical.hpp"
#include "math/sequence.hpp"

#include "math/alphabet.hpp"

#include "flipsta/automaton.hpp"

/**
Example from Cormen et al. (2009), that is not yet topologically sorted.
The six states (r, s, t, x, y, z) are called (d, c, a, f, b, e).
*/
flipsta::Automaton <char, math::cost <float>> acyclicExample();

/**
The same example as acyclicExample, but then with symbols attached.
Also, the link between t and z ('a' and 'e') has a cost of 2.5, to disambiguate.
*/
flipsta::Automaton <char,
    math::lexicographical <math::over <
        math::cost <float>, math::single_sequence <char>>>>
    acyclicSequenceExample();

/**
Example automaton that has paths with a common prefix of one character.
*/
flipsta::Automaton <int, math::single_sequence <char, math::left>>
    prefixExample();

/**
Example automaton that has paths with a common suffix of two characters.
*/
flipsta::Automaton <int, math::single_sequence <char, math::right>>
    suffixExample();

flipsta::Automaton <int, math::optional_sequence <std::string>>
    hypothesisExample (std::shared_ptr <math::alphabet <std::string>>);

flipsta::Automaton <char, math::single_sequence <std::string>>
    referenceExample (std::shared_ptr <math::alphabet <std::string>>);

#endif // FLIPSTA_TEST_EXAMPLE_AUTOMATA_HPP_INCLUDED
