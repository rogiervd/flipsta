/*
Copyright 2014, 2015 Rogier van Dalen.

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
Read AT&T automaton file.
The format is documented at
http://www2.research.att.com/~fsmtools/fsm/man/fsm.5.html
*/

#include <string>
#include <functional>

#include "parse_ll/core.hpp"
#include "parse_ll/number/unsigned.hpp"
#include "parse_ll/number/float.hpp"

#include "parse_ll/debug/trace.hpp"
#include "parse_ll/debug/ostream_observer.hpp"

#include "parse_ll/support/file_range.hpp"
#include "parse_ll/support/text_location_range.hpp"

#include "flipsta/att/automaton.hpp"

#include "../read_text_file.hpp"

namespace flipsta { namespace att {

using namespace parse_ll;
using range::at_c;

namespace detail {

/*
\todo Revamp this implementation.
This abuses the current parsing framework.
Once that has seen a revamp, redo this.
Mostly, the processing of arcs and states should be done one level higher,
instead of with these mutating closures.

Also, of course, the weights should be read as strings and then passed to
something that converts them into actual weights, instead of assuming they are
floating-point values.
*/

namespace {

/**
Convert a range of char's into a std::string.
*/
struct convertToStdString {
    template <class Range> std::string operator() (Range range) const
    {
        std::string result;
        for (; !::range::empty (range); range = ::range::drop (range))
            result.push_back (::range::first (range));
        return result;
    }
};

struct Nothing {};

class AddArc {
    AutomatonWrapper & automaton_;
    SymbolTable const & inputSymbolTable_;
    SymbolTable const & outputSymbolTable_;
    mutable bool seenState;

public:
    AddArc (AutomatonWrapper & automaton,
        SymbolTable const & inputSymbolTable,
        SymbolTable const & outputSymbolTable)
    : automaton_ (automaton), inputSymbolTable_ (inputSymbolTable),
        outputSymbolTable_ (outputSymbolTable), seenState (false) {}

    OptionalSequence getSymbol (SymbolTable const & table,
        std::string const & symbolName) const
    {
        if (table.hasEmptySymbol() && symbolName == table.emptySymbol())
            return OptionalSequence();
        return OptionalSequence (symbolName);
    }

    template <class Tuple> Nothing operator() (Tuple const & tuple) const {
        auto source = at_c <0> (tuple);
        auto destination = at_c <1> (tuple);
        auto inputSymbol = at_c <2> (tuple);
        auto outputSymbol = at_c <3> (tuple);
        auto weightRepresentation = at_c <4> (tuple);

        boost::any weight;
        if (weightRepresentation)
            weight = automaton_.weight (weightRepresentation.get());
        else
            weight = automaton_.one();

        if (!automaton_.hasState (source))
            automaton_.addState (source);
        if (!seenState) {
            // The first state is automatically the start state.
            automaton_.setStartState (source);
            seenState = true;
        }
        if (!automaton_.hasState (destination))
            automaton_.addState (destination);

        OptionalSequence input = getSymbol (inputSymbolTable_, inputSymbol);
        OptionalSequence output = getSymbol (outputSymbolTable_, outputSymbol);

        automaton_.addArc (source, destination, input, output, weight);
        return Nothing();
    }
};

struct AddFinalState {
    AutomatonWrapper & automaton_;

    AddFinalState (AutomatonWrapper & automaton)
    : automaton_ (automaton) {}

    template <class Tuple> Nothing operator() (Tuple const & tuple) const {
        State state = at_c <0> (tuple);
        auto weightRepresentation = at_c <1> (tuple);

        boost::any weight;
        if (weightRepresentation)
            weight = automaton_.weight (weightRepresentation.get());
        else
            weight = automaton_.one();

        if (!automaton_.hasState (state))
            automaton_.addState (state);
        automaton_.setFinalState (state, weight);
        return Nothing();
    }
};

void readAutomatonFrom (
    TextFileRange && fileRange, AutomatonWrapper & automaton,
    SymbolTable const & inputSymbolTable,
    SymbolTable const & outputSymbolTable)
{
    PARSE_LL_DEFINE_NAMED_PARSER (ch, char_ - one_whitespace);

    PARSE_LL_DEFINE_NAMED_PARSER (symbolName,
        no_skip [(+ch) [convertToStdString()]]);
    PARSE_LL_DEFINE_NAMED_PARSER (stateName, unsigned_as <State>());
    PARSE_LL_DEFINE_NAMED_PARSER (weight, float_as <double>());

    // PARSE_LL_DEFINE_NAMED_PARSER (transition,
    //     (stateName >> stateName >> symbolName >> symbolName >> -weight)
    //     [AddArc (automaton)]);
    rule <TextFileRange, Nothing> transition =
        (stateName >> stateName >> symbolName >> symbolName >> -weight)
        [AddArc (automaton, inputSymbolTable, outputSymbolTable)];
    // PARSE_LL_DEFINE_NAMED_PARSER (finalState, (stateName >> -weight)
    //     [AddFinalState (automaton)]);
    rule <TextFileRange, Nothing> finalState = (stateName >> -weight)
        [AddFinalState (automaton)];

    auto line = transition | finalState;

    // This is actually quite dangerous: if you add the debugging, it will
    // duplicate arcs.
    // This is because this code abuses the parser framework.
    auto lines =
        // trace (ostream_observer()) [
            skip (horizontal_space) [*(line >> vertical_space) > end]
        // ]
        ;

    auto outcome = parse_ll::parse (lines, fileRange);

    if (!success (outcome))
        throw parse_ll::error();
    if (!range::empty (rest (outcome)))
        throw parse_ll::error() << error_at (rest (outcome));
}

} // namespace

void readAutomaton (
    std::string const & fileName, AutomatonWrapper & wrapper,
    SymbolTable const & inputSymbolTable,
    SymbolTable const & outputSymbolTable)
{
    return readTextFile (
        std::bind <void> (readAutomatonFrom,
            std::placeholders::_1, std::ref (wrapper),
            std::ref (inputSymbolTable), std::ref (outputSymbolTable)),
        fileName);
}

} // namespace detail

}} // namespace flipsta::att
