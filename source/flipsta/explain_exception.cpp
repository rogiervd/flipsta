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

#include "flipsta/att/symbol_table.hpp"

#include <boost/exception/all.hpp>

#include "parse_ll/support/file_range.hpp"
#include "parse_ll/support/text_location_range.hpp"

#include "parse_ll/core/error.hpp"

typedef range::text_location_range <range::file_range> text_file_range;

namespace flipsta {

void explainException (std::ostream & os, boost::exception const & e) {
    if (std::string const * description
        = boost::get_error_info <parse_ll::error_description> (e))
    { os << "  " << *description << '\n'; }

    if (std::string const * fileName
        = boost::get_error_info <boost::errinfo_file_name> (e))
    { os << "  While reading " << *fileName << '\n'; }

    if (text_file_range const * position = boost::get_error_info <
        parse_ll::error_position <text_file_range>::type> (e))
    {
        os << "  At " << position->line() + 1
            << ", " << position->column() + 1 << "\n";
    }
}

} // namespace flipsta
