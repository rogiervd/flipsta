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

#ifndef FLIPSTA_DRAW_HPP_INCLUDED
#define FLIPSTA_DRAW_HPP_INCLUDED

#include <ostream>
#include <sstream>
#include <string>

#include "range/std/container.hpp"
#include "range/for_each_macro.hpp"

#include "core.hpp"

namespace flipsta {

/// \cond DONT_DOCUMENT
namespace draw_detail {

    template <class Type> inline std::string makePrintable (Type const & object)
    {
        std::ostringstream stream;
        stream << object;
        std::string result;
        RANGE_FOR_EACH (character, stream.str()) {
            if (character == '"')
                result += "\\\"";
            else
                result += character;
        }
        return result;
    }

} // namespace draw_detail
/// \endcond

/** \brief
Produce a \c .dot file for Graphviz dot to convert into a graphical
representation.

The names of states are put inside the nodes; the start and end labels, if any,
go next to it.
This uses the "xlabel" attribute and requires dot version 2.28 to render.
Earlier versions leave out the labels.

The textual representation of states and labels is acquired by streaming them
into an ostream.

Assuming the Graphviz dot is installed, then after writing the textual
representation to ``automaton.dot``, saying the following on the command line
will convert it into a PDF file:
\code{.sh}
    dot -Tpdf automaton.dot -o automaton.pdf
\endcode

\param file The \c ostream to output the textual representation to.
\param automaton The automaton to be drawn.
\param horizontal Lay levels out horizontally instead of vertically.
*/
template <class Automaton>
    inline void draw (std::ostream & file, Automaton const & automaton,
        bool horizontal = false)
{
    using draw_detail::makePrintable;

    file << "digraph flipstaGraph {\n"
        "/*  This file has been produced by the Flipsta library.\n"
        "    Generate a picture from this using Graphviz dot, e.g.\n"
        "        dot -Tpdf automaton.dot -o automaton.pdf\n"
        "*/\n\n";

    char const * indent = "  ";
    char const * arcIndent = "    ";

    if (horizontal)
        file << indent << "rankdir=\"LR\";\n";

    RANGE_FOR_EACH (state, states (automaton)) {
        file << indent;

        std::string stateAttributes;
        std::string outsideLabel;
        std::string finalLabelDescription;
        auto startLabel = terminalLabel (automaton, forward, state);

        auto zero = math::zero <decltype (startLabel)>();
        auto one = math::one <decltype (startLabel)>();

        if (startLabel != zero) {
            stateAttributes += ",style=bold";
            if (startLabel != one) {
                outsideLabel = makePrintable (startLabel);
            }
        }

        auto finalLabel = terminalLabel (automaton, backward, state);
        if (finalLabel != zero) {
            stateAttributes += ",peripheries=2";
            if (finalLabel != one) {
                if (!outsideLabel.empty())
                    outsideLabel += " / ";
                outsideLabel += makePrintable (startLabel);
            }
        }

        if (!outsideLabel.empty())
            stateAttributes += ",xlabel=\"" + outsideLabel + "\"";

        file << '"' << makePrintable (state) << '"'
            << " [shape=circle" << stateAttributes << "];\n";

        RANGE_FOR_EACH (arc, arcsOn (automaton, forward, state)) {
            file << arcIndent << '"' << makePrintable (state) << "\" -> \""
                << makePrintable (arc.state (forward))
                << "\" [label=\""
                << makePrintable (arc.label()) << "\"];\n";
        }
    }

    file << "}\n";
}

} // namespace flipsta

#endif // FLIPSTA_DRAW_HPP_INCLUDED
