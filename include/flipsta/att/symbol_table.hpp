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

#ifndef FLIPSTA_ATT_SYMBOL_TABLE_HPP_INCLUDED
#define FLIPSTA_ATT_SYMBOL_TABLE_HPP_INCLUDED

#include <string>
#include <memory>

#include <boost/optional.hpp>

#include "math/alphabet.hpp"

namespace flipsta { namespace att {

/** \brief
Hold an AT&T symbol table, which is a one-to-one mapping from a dense set of
integers to a set of strings.

In the file format. there is a special integer (0) associated with an "empty"
symbol.
If that exists, the string for it is kept separately.

The dense symbols that the Flipsta library chooses are one off compared to the
file, since math::alphabet starts counting normal symbols from 0.
*/
class SymbolTable {
public:
    typedef math::alphabet <std::string> Alphabet;
private:
    std::shared_ptr <Alphabet> alphabet_;
    boost::optional <std::string> emptySymbol_;

public:
    SymbolTable() : alphabet_ (std::make_shared <Alphabet>()) {}

    std::shared_ptr <Alphabet> const & alphabet() const { return alphabet_; }

    bool hasEmptySymbol() const { return !!emptySymbol_; }
    std::string const & emptySymbol() const { return emptySymbol_.get(); }

    void setEmptySymbol (std::string symbol) {
        assert (!emptySymbol_);
        emptySymbol_ = symbol;
    }
};

/** \brief
Read a symbol table in AT&T format.

The resulting pointer is always non-null.

\throw parse_ll::error
    if a parse error occurs or there is an inconsistency in the table.
    \ref explainException should be able to print out a human-readable
    description.
*/
std::unique_ptr <SymbolTable> readSymbolTable (std::string file_name);

}} // namespace flipsta::att

#endif // FLIPSTA_ATT_SYMBOL_TABLE_HPP_INCLUDED
