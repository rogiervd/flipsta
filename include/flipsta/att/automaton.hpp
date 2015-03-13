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

#ifndef FLIPSTA_ATT_AUTOMATON_HPP_INCLUDED
#define FLIPSTA_ATT_AUTOMATON_HPP_INCLUDED

#include <string>
#include <memory>
#include <functional>

#include <boost/any.hpp>

#include "utility/make_unique.hpp"

#include "math/alphabet.hpp"
#include "math/arithmetic_magma.hpp"
#include "math/cost.hpp"
#include "math/sequence.hpp"
#include "math/product.hpp"

#include "flipsta/automaton.hpp"

#include "symbol_table.hpp"

namespace flipsta { namespace att {

namespace detail {

    typedef math::alphabet <std::string> Alphabet;
    typedef typename Alphabet::dense_symbol_type Symbol;
    typedef math::optional_sequence <std::string> OptionalSequence;
    typedef math::empty_sequence <std::string> EmptySymbol;
    typedef math::cost <double> Weight;

    typedef math::product <math::over <
        OptionalSequence, OptionalSequence, Weight>> Label;

    typedef std::size_t State;

    typedef math::product <math::over <
        EmptySymbol, EmptySymbol, Weight>> TerminalLabel;

    typedef flipsta::Automaton <State, Label, TerminalLabel> Automaton;

    class AutomatonWrapper {
    public:
        virtual boost::any one() const = 0;
        virtual boost::any weight (double) const = 0;
        virtual bool hasState (State) const = 0;
        virtual void addState (State) = 0;
        virtual void addArc (State, State, OptionalSequence, OptionalSequence,
            boost::any const &) = 0;
        virtual void setStartState (State) = 0;
        virtual void setFinalState (State, boost::any const &) = 0;
    };

    template <class Weight, class Automaton> class WrappedAutomaton
    : public AutomatonWrapper
    {
        Automaton & automaton_;
    public:
        WrappedAutomaton (Automaton & automaton) : automaton_ (automaton) {}

        boost::any one() const { return math::one <Weight>(); }
        boost::any weight (double d) const  { return Weight (d); }
        bool hasState (State s) const { return automaton_.hasState (s); }
        void addState (State s) { automaton_.addState (s); }

        void addArc (State source, State destination,
            OptionalSequence input, OptionalSequence output,
            boost::any const & weight)
        {
            automaton_.addArc (source, destination,
                Label (input, output, boost::any_cast <Weight> (weight)));
        }

        void setStartState (State s)
        { automaton_.setTerminalLabel (forward, s, math::one <Label>()); }

        void setFinalState (State s, boost::any const & weight) {
            automaton_.setTerminalLabel (backward, s,
                Label (EmptySymbol(), EmptySymbol(),
                    boost::any_cast <Weight> (weight)));
        }
    };

    void readAutomaton (
        std::string const & fileName, AutomatonWrapper & wrapper,
        SymbolTable const & inputSymbolTable,
        SymbolTable const & outputSymbolTable);

} // namespace detail


/** \brief
Read an automaton from a file in AT&T format.

The weights are currently assumed to be StdWeights, i.e. math::cost \<double>.

The symbol tables can be the same, if the input and output alphabets are the
same.

\todo The symbol mapping merely needs to indicate what the empty symbol is, and
an alphabet needs to be passed to the automaton's label tag.
Allow for these two cases:
\li The symbols in the file are strings, but the symbol table should be built as
    the symbols are encountered.
    This should be initialised with an empty symbol (\<eps>, typically).
\li The symbols in the file are dense integers, and should be saved as such.
    In that case, there should be no alphabet tag at all, and "0" should map to
    the empty sequence.

\todo Maybe it should be possible to read automata while guaranteeing that no
symbols are actually empty?

\todo Allow for different semirings.
*/
inline std::unique_ptr <detail::Automaton>
    readAutomaton (std::string const & file_name,
        SymbolTable const & inputSymbolTable,
        SymbolTable const & outputSymbolTable)
{
    typedef detail::Automaton Automaton;
    typedef typename Automaton::Tag Tag;

    auto result = utility::make_unique <Automaton> (
        Tag (inputSymbolTable.alphabet(), outputSymbolTable.alphabet(),
            label::NoTag()));
    detail::WrappedAutomaton <detail::Weight, Automaton> wrapper (*result);
    detail::readAutomaton (
        file_name, wrapper, inputSymbolTable, outputSymbolTable);
    return std::move (result);
}

}} // namespace flipsta::att

#endif // FLIPSTA_ATT_AUTOMATON_HPP_INCLUDED
