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

#ifndef FLIPSTA_DRAW_HPP_INCLUDED
#define FLIPSTA_DRAW_HPP_INCLUDED

#include <iosfwd>
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

    template <class First, class Second>
        inline std::string makePrintable (std::pair <First, Second> const & p)
    {
        return "(" + makePrintable (p.first) + ", "
            + makePrintable (p.second) + ")";
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
    static_assert (IsAutomaton <Automaton>::value,
        "Automaton must be an automaton.");
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
