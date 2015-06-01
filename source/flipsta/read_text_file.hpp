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
