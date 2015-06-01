/*
Copyright 2014, 2015 Rogier van Dalen.

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
Read an AT&T symbol file.
The format is documented at
http://www2.research.att.com/~fsmtools/fsm/man/fsm.5.html
*/

#include "flipsta/att/symbol_table.hpp"

#include <boost/exception/all.hpp>

#include <boost/format.hpp>
#include <boost/bimap.hpp>

#include "utility/make_unique.hpp"

#include "range/tuple.hpp"
#include "range/std/tuple.hpp"

#include "range/for_each_macro.hpp"

#include "parse_ll/support/file_range.hpp"
#include "parse_ll/support/text_location_range.hpp"

#include "parse_ll/core.hpp"
#include "parse_ll/number/unsigned.hpp"

#include "../read_text_file.hpp"

typedef typename math::alphabet <std::string>::dense_type Index;

namespace flipsta { namespace att {

namespace {
std::unique_ptr <SymbolTable> readSymbolTableFrom (TextFileRange && file_range);
} // namespace

std::unique_ptr <SymbolTable> readSymbolTable (std::string fileName) {
    return readTextFile (&readSymbolTableFrom, fileName);
}

namespace {

using range::tuple;
using std::string;
using std::vector;
using range::drop;
using range::first;
using range::second;
using range::empty;

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

std::unique_ptr <SymbolTable> readSymbolTableFrom (TextFileRange && file_range)
{
    std::unique_ptr <SymbolTable> result
        = utility::make_unique <SymbolTable>();
    typedef typename math::alphabet <std::string>::dense_type Index;

    using namespace parse_ll;

    PARSE_LL_DEFINE_NAMED_PARSER (Char, char_ - one_whitespace);
    PARSE_LL_DEFINE_NAMED_PARSER (SymbolName,
        no_skip [(+Char) [convertToStdString()]]);
    PARSE_LL_DEFINE_NAMED_PARSER (SymbolMapping,
        (SymbolName >> unsigned_as <Index>() >> vertical_space));
    PARSE_LL_DEFINE_NAMED_PARSER (SymbolMappings,
        // trace (ostream_observer()) [
            skip (horizontal_space) [*(SymbolMapping) > end]
        // ]
        );

    auto outcome = parse_ll::parse (SymbolMappings, file_range);

    if (!success (outcome))
        throw parse_ll::error() << parse_ll::error_at (rest (outcome));;

    // The output must be in memory explicitly...!
    auto && o = output (outcome);

    typedef boost::bimap <std::string, Index> mappings_type;
    typedef typename mappings_type::value_type mapping_type;

    mappings_type mappings;
    RANGE_FOR_EACH (mapping, range::first (o)) {
        // Check for duplicate index.
        auto same_index = mappings.right.find (second (mapping));
        if (same_index != mappings.right.end())
            throw parse_ll::error()
                << parse_ll::error_description (boost::str (boost::format (
                    "Duplicate index: %i (for %s and %s)") %
                    second (mapping) % same_index->second % first (mapping)));

        // Check for duplicate symbol name.
        auto same_name = mappings.left.find (first (mapping));
        if (same_name != mappings.left.end())
            throw parse_ll::error()
                << parse_ll::error_description (boost::str (boost::format (
                    "Duplicate name: %s (with %i and %i)") %
                    first (mapping) % same_name->second % second (mapping)));

        mappings.insert (mapping_type (first (mapping), second (mapping)));
    }

    auto mappingsView = range::make_iterator_range (mappings.right);

    if (empty (mappingsView))
        return result;

    // Check that we start with 0 or 1.
    Index lowestIndex = first (mappingsView).first;
    if (lowestIndex != 0 && lowestIndex != 1) {

        throw parse_ll::error()
            << parse_ll::error_description ((boost::format (
                "The lowest index in symbol table must be 0 or 1, "
                "not %i (for %s).")
                % lowestIndex % first (mappingsView).second).str());
    }

    if (first (mappingsView).first == 0) {
        result->setEmptySymbol (first (mappingsView).second);
        mappingsView = drop (mappingsView);
    }

    // Check that values are adjacent.
    Index previousIndex = 0;
    RANGE_FOR_EACH (mapping, mappingsView) {
        if (previousIndex + 1 != mapping.first) {
            throw parse_ll::error()
                << parse_ll::error_description ((
                    boost::format ("The values in the symbol table must be "
                        "dense, i.e. all 1 apart, which %i and %i (for %s) "
                        "are not.")
                    % previousIndex % mapping.first % mapping.second).str());
        }
        previousIndex = mapping.first;
    }

    RANGE_FOR_EACH (mapping, mappingsView) {
        auto index = result->alphabet()->add_symbol (mapping.second);
        // The symbol table starts numbering non-empty symbols from 1.
        assert (index.id() == mapping.first - 1);
        (void) index;
    }

    return std::move (result);
}

} // namespace
}} // namespace flipsta::att
