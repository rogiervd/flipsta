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
