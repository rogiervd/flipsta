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

#ifndef FLIPSTA_SOURCE_FLIPSTA_READ_TEXT_FILE_HPP_INCLUDED
#define FLIPSTA_SOURCE_FLIPSTA_READ_TEXT_FILE_HPP_INCLUDED

#include <type_traits>
#include <stdexcept>

#include <boost/exception/all.hpp>

namespace flipsta {

typedef range::text_location_range <range::file_range> TextFileRange;

/** \brief
Read a text file with a function that takes a TextFileRange.

If an exception is thrown, this does its best to attach the file name to the
exception.

This is useful to wrap around the function that does the actual parsing.
*/
template <class ParseFile>
    inline typename std::result_of <ParseFile (TextFileRange)>::type
    readTextFile (ParseFile && parseFile, std::string fileName)
{
    try {
        return parseFile (TextFileRange (fileName));
    } catch (boost::exception & e) {
        e << boost::errinfo_file_name (fileName);
        throw;
    } catch (std::exception &) {
        // Otherwise, catch here and convert to a boost exception
        try { boost::rethrow_exception (boost::current_exception()); }
        catch (boost::exception& e) {
            e << boost::errinfo_file_name (fileName);
            throw;
        }
    }
}

} // namespace flipsta

#endif // FLIPSTA_SOURCE_FLIPSTA_READ_TEXT_FILE_HPP_INCLUDED
