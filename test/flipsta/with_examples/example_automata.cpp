/*
Copyright 2015 Rogier van Dalen.

This file is part of the Flipsta library.

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** \file
Example automata.
If you would like to see these graphically, run test-draw-examples from the
unit tests and pass in output .dot file names.
Then run Graphviz dot to turn the .dot files into a drawing.
*/

#include "example_automata.hpp"

using flipsta::forward;
using flipsta::backward;

/**
Example from Cormen et al. (2009), that is not yet topologically sorted.
The six states (r, s, t, x, y, z) are called (d, c, a, f, b, e).
*/
flipsta::Automaton <char, math::cost <float>> acyclicExample() {
    typedef math::cost <float> Cost;
    flipsta::Automaton <char, Cost> automaton;

    automaton.addState ('a');
    automaton.addState ('b');
    automaton.addState ('c');
    automaton.addState ('d');
    automaton.addState ('e');
    automaton.addState ('f');

    char r = 'd';
    char s = 'c';
    char t = 'a';
    char x = 'f';
    char y = 'b';
    char z = 'e';

    // Arcs that the topological order hangs on.
    automaton.addArc (r, s, Cost (5));
    automaton.addArc (s, t, Cost (2));
    automaton.addArc (t, x, Cost (7));
    automaton.addArc (x, y, Cost (-1));
    automaton.addArc (y, z, Cost (-2));

    // Arcs that skip at least one level.
    automaton.addArc (r, t, Cost (3));
    automaton.addArc (s, x, Cost (6));
    automaton.addArc (t, y, Cost (4));
    automaton.addArc (t, z, Cost (2));
    automaton.addArc (x, z, Cost (1));

    return automaton;
}

/**
The same example as acyclicExample, but then with symbols attached.
Also, the link between t and z ('a' and 'e') has a cost of 2.5, to disambiguate.
*/
flipsta::Automaton <char,
    math::lexicographical <math::over <
        math::cost <float>, math::single_sequence <char>>>>
    acyclicSequenceExample()
{
    typedef math::cost <float> Cost;
    typedef math::single_sequence <char> Sequence;
    typedef math::lexicographical <math::over <Cost, Sequence>> Lexicographical;
    flipsta::Automaton <char, Lexicographical> automaton;

    automaton.addState ('a');
    automaton.addState ('b');
    automaton.addState ('c');
    automaton.addState ('d');
    automaton.addState ('e');
    automaton.addState ('f');

    char r = 'd';
    char s = 'c';
    char t = 'a';
    char x = 'f';
    char y = 'b';
    char z = 'e';

    // Arcs that the topological order hangs on.
    automaton.addArc (r, s, Lexicographical (Cost (5), Sequence ('h')));
    automaton.addArc (s, t, Lexicographical (Cost (2), Sequence ('j')));
    automaton.addArc (t, x, Lexicographical (Cost (7), Sequence ('l')));
    automaton.addArc (x, y, Lexicographical (Cost (-1), Sequence ('o')));
    automaton.addArc (y, z, Lexicographical (Cost (-2), Sequence ('p')));

    // Arcs that skip at least one level.
    automaton.addArc (r, t, Lexicographical (Cost (3), Sequence ('i')));
    automaton.addArc (s, x, Lexicographical (Cost (6), Sequence ('k')));
    automaton.addArc (t, y, Lexicographical (Cost (4), Sequence ('m')));
    automaton.addArc (t, z, Lexicographical (Cost (2.5), Sequence ('n')));
    automaton.addArc (x, z, Lexicographical (Cost (1), Sequence ('o')));

    return automaton;
}

template <class Sequence> flipsta::Automaton <int, Sequence> affixExample() {
    flipsta::Automaton <int, Sequence> automaton;

    automaton.addState (1);
    automaton.addState (2);
    automaton.addState (3);
    automaton.addState (4);
    // automaton.addState (5);
    automaton.addState (6);
    automaton.addState (7);
    automaton.addState (8);
    automaton.addState (10);

    automaton.addArc (1, 2, Sequence ('a'));
    automaton.addArc (2, 3, Sequence ('b'));
    automaton.addArc (3, 7, Sequence ('y'));

    automaton.addArc (1, 4, Sequence ('a'));
    automaton.addArc (4, 3, Sequence ('b'));
    automaton.addArc (3, 7, Sequence ('y'));

    automaton.addArc (4, 6, Sequence ('c'));
    automaton.addArc (6, 8, Sequence ('y'));

    automaton.addArc (7, 10, Sequence ('z'));
    automaton.addArc (8, 10, Sequence ('z'));

    return automaton;
}

flipsta::Automaton <int, math::single_sequence <char, math::left>>
    prefixExample()
{ return affixExample <math::single_sequence <char, math::left>>(); }

flipsta::Automaton <int, math::single_sequence <char, math::right>>
    suffixExample()
{ return affixExample <math::single_sequence <char, math::right>>(); }

typedef flipsta::Automaton <int, math::optional_sequence <std::string>>
    IntAutomaton;

IntAutomaton hypothesisExample (
    std::shared_ptr <math::alphabet <std::string>> alphabet)
{
    IntAutomaton automaton (alphabet);

    math::empty_sequence <std::string> one;

    automaton.addState (0);
    automaton.addState (1);
    automaton.addState (2);
    automaton.addState (3);
    automaton.addState (4);

    automaton.setTerminalLabel (forward, 0, one);
    automaton.setTerminalLabel (backward, 4, one);

    automaton.addArc (0, 1, math::single_sequence <std::string> ("tall"));
    automaton.addArc (0, 1, math::single_sequence <std::string> ("cat"));

    automaton.addArc (1, 3, math::single_sequence <std::string> ("tall"));
    automaton.addArc (1, 2, one);
    automaton.addArc (2, 3, math::single_sequence <std::string> ("tree"));

    automaton.addArc (3, 4, math::single_sequence <std::string> ("tree"));

    return std::move (automaton);
}

typedef flipsta::Automaton <char, math::single_sequence <std::string>>
    CharAutomaton;

CharAutomaton referenceExample (
    std::shared_ptr <math::alphabet <std::string>> alphabet)
{
    CharAutomaton automaton (alphabet);

    math::empty_sequence <std::string> one;

    automaton.addState ('A');
    automaton.addState ('B');
    automaton.addState ('C');

    automaton.setTerminalLabel (forward, 'A', one);
    automaton.setTerminalLabel (backward, 'C', one);

    automaton.addArc ('A', 'B', math::single_sequence <std::string> ("tall"));
    automaton.addArc ('B', 'C', math::single_sequence <std::string> ("tree"));

    return std::move (automaton);
}
