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

#include "utility/unique_ptr.hpp"

#include "example_automata.hpp"

using flipsta::forward;
using flipsta::backward;

/**
Example from Cormen et al. (2009), that is not yet topologically sorted.
The six states (r, s, t, x, y, z) are called (d, c, a, f, b, e).
*/
std::unique_ptr <flipsta::Automaton <char, math::cost <float>>> acyclicExample()
{
    typedef math::cost <float> Cost;
    auto automaton = utility::make_unique <flipsta::Automaton <char, Cost>>();

    automaton->addState ('a');
    automaton->addState ('b');
    automaton->addState ('c');
    automaton->addState ('d');
    automaton->addState ('e');
    automaton->addState ('f');

    automaton->setTerminalLabel (forward, 'd', math::one <Cost>());
    automaton->setTerminalLabel (backward, 'e', Cost (1));

    char r = 'd';
    char s = 'c';
    char t = 'a';
    char x = 'f';
    char y = 'b';
    char z = 'e';

    // Arcs that the topological order hangs on.
    automaton->addArc (r, s, Cost (5));
    automaton->addArc (s, t, Cost (2));
    automaton->addArc (t, x, Cost (7));
    automaton->addArc (x, y, Cost (-1));
    automaton->addArc (y, z, Cost (-2));

    // Arcs that skip at least one level.
    automaton->addArc (r, t, Cost (3));
    automaton->addArc (s, x, Cost (6));
    automaton->addArc (t, y, Cost (4));
    automaton->addArc (t, z, Cost (2));
    automaton->addArc (x, z, Cost (1));

    return automaton;
}

/**
The same example as acyclicExample, but then with symbols attached.
Also, the link between t and z ('a' and 'e') has a cost of 2.5, to disambiguate.
*/
std::unique_ptr <flipsta::Automaton <char,
    math::lexicographical <math::over <
        math::cost <float>, math::single_sequence <char>>>>>
    acyclicSequenceExample()
{
    typedef math::cost <float> Cost;
    typedef math::single_sequence <char> Sequence;
    typedef math::lexicographical <math::over <Cost, Sequence>> Lexicographical;
    auto automaton
        = utility::make_unique <flipsta::Automaton <char, Lexicographical>>();

    automaton->addState ('a');
    automaton->addState ('b');
    automaton->addState ('c');
    automaton->addState ('d');
    automaton->addState ('e');
    automaton->addState ('f');

    automaton->setTerminalLabel (forward, 'd', math::one <Lexicographical>());
    automaton->setTerminalLabel (backward, 'e',
        math::make_lexicographical (Cost (1), math::one <Sequence>()));

    char r = 'd';
    char s = 'c';
    char t = 'a';
    char x = 'f';
    char y = 'b';
    char z = 'e';

    // Arcs that the topological order hangs on.
    automaton->addArc (r, s, Lexicographical (Cost (5), Sequence ('h')));
    automaton->addArc (s, t, Lexicographical (Cost (2), Sequence ('j')));
    automaton->addArc (t, x, Lexicographical (Cost (7), Sequence ('l')));
    automaton->addArc (x, y, Lexicographical (Cost (-1), Sequence ('o')));
    automaton->addArc (y, z, Lexicographical (Cost (-2), Sequence ('p')));

    // Arcs that skip at least one level.
    automaton->addArc (r, t, Lexicographical (Cost (3), Sequence ('i')));
    automaton->addArc (s, x, Lexicographical (Cost (6), Sequence ('k')));
    automaton->addArc (t, y, Lexicographical (Cost (4), Sequence ('m')));
    automaton->addArc (t, z, Lexicographical (Cost (2.5), Sequence ('n')));
    automaton->addArc (x, z, Lexicographical (Cost (1), Sequence ('o')));

    return automaton;
}

template <class Sequence>
    std::unique_ptr <flipsta::Automaton <int, Sequence>> affixExample()
{
    auto automaton
        = utility::make_unique <flipsta::Automaton <int, Sequence>>();

    automaton->addState (1);
    automaton->addState (2);
    automaton->addState (3);
    automaton->addState (4);
    // automaton->addState (5);
    automaton->addState (6);
    automaton->addState (7);
    automaton->addState (8);
    automaton->addState (10);

    automaton->setTerminalLabel (forward, 1, math::one <Sequence>());
    automaton->setTerminalLabel (backward, 10, math::one <Sequence>());

    automaton->addArc (1, 2, Sequence ('a'));
    automaton->addArc (2, 3, Sequence ('b'));
    automaton->addArc (3, 7, Sequence ('y'));

    automaton->addArc (1, 4, Sequence ('a'));
    automaton->addArc (4, 3, Sequence ('b'));
    automaton->addArc (3, 7, Sequence ('y'));

    automaton->addArc (4, 6, Sequence ('c'));
    automaton->addArc (6, 8, Sequence ('y'));

    automaton->addArc (7, 10, Sequence ('z'));
    automaton->addArc (8, 10, Sequence ('z'));

    return automaton;
}

std::unique_ptr <
        flipsta::Automaton <int, math::single_sequence <char, math::left>>>
    prefixExample()
{ return affixExample <math::single_sequence <char, math::left>>(); }

std::unique_ptr <
        flipsta::Automaton <int, math::single_sequence <char, math::right>>>
    suffixExample()
{ return affixExample <math::single_sequence <char, math::right>>(); }

typedef flipsta::Automaton <int, math::optional_sequence <std::string>>
    IntAutomaton;

std::unique_ptr <IntAutomaton> hypothesisExample (
    std::shared_ptr <math::alphabet <std::string>> alphabet)
{
    auto automaton = utility::make_unique <IntAutomaton> (alphabet);

    math::empty_sequence <std::string> one;

    automaton->addState (0);
    automaton->addState (1);
    automaton->addState (2);
    automaton->addState (3);
    automaton->addState (4);

    automaton->setTerminalLabel (forward, 0, one);
    automaton->setTerminalLabel (backward, 4, one);

    automaton->addArc (0, 1, math::single_sequence <std::string> ("tall"));
    automaton->addArc (0, 1, math::single_sequence <std::string> ("cat"));

    automaton->addArc (1, 3, math::single_sequence <std::string> ("tall"));
    automaton->addArc (1, 2, one);
    automaton->addArc (2, 3, math::single_sequence <std::string> ("tree"));

    automaton->addArc (3, 4, math::single_sequence <std::string> ("tree"));

    return std::move (automaton);
}

typedef flipsta::Automaton <char, math::single_sequence <std::string>>
    CharAutomaton;

std::unique_ptr <CharAutomaton> referenceExample (
    std::shared_ptr <math::alphabet <std::string>> alphabet)
{
    auto automaton = utility::make_unique <CharAutomaton> (alphabet);

    math::empty_sequence <std::string> one;

    automaton->addState ('A');
    automaton->addState ('B');
    automaton->addState ('C');

    automaton->setTerminalLabel (forward, 'A', one);
    automaton->setTerminalLabel (backward, 'C', one);

    automaton->addArc ('A', 'B', math::single_sequence <std::string> ("tall"));
    automaton->addArc ('B', 'C', math::single_sequence <std::string> ("tree"));

    return std::move (automaton);
}
