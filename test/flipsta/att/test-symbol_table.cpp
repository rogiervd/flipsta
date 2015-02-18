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
Test readSymbolTable.
Pass into this as the first command line argument either --with_empty, if the
symbol list contains the empty symbol (with index 0), or --without_empty, if
not.
The second argument must be the file name of the symbol list to be read.
*/

#define BOOST_TEST_MODULE test_read_symbol_table
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/att/symbol_table.hpp"

#include <iostream>
#include <iostream>

#include "parse_ll/core/error.hpp"

#include "flipsta/explain_exception.hpp"

BOOST_AUTO_TEST_SUITE(test_readSymbolTable)

BOOST_AUTO_TEST_CASE (from_example) {
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char ** argv = boost::unit_test::framework::master_test_suite().argv;

    // Otherwise there are no files to test on.
    BOOST_REQUIRE_EQUAL (argc, 3);

    if (std::string (argv [1]) == "--fail") {
        try {
            flipsta::att::readSymbolTable (argv [2]);
        } catch (parse_ll::error & e) {
            std::cout << "As expected, an error occurred while parsing:\n";
            flipsta::explainException (std::cout, e);
            return;
        }
        BOOST_FAIL ("This file should have led to an exception.");
    }

    try {
        flipsta::att::SymbolTable symbolTable
            = *flipsta::att::readSymbolTable (argv [2]);
        if (std::string (argv [1]) == "--with_empty") {
            BOOST_CHECK_EQUAL (symbolTable.emptySymbol(), "<eps>");
        } else {
            BOOST_CHECK_EQUAL (argv [1], "--without_empty");
            BOOST_CHECK (!symbolTable.hasEmptySymbol());
        }
        // All the symbols must have an id one less than in the original file.
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("<del>").id(), 0);
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("<ins>").id(), 1);
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("<sub>").id(), 2);
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("a").id(), 3);
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("b").id(), 4);
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("c").id(), 5);
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("d").id(), 6);
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("e").id(), 7);
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("f").id(), 8);
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("g").id(), 9);
        BOOST_CHECK_EQUAL (symbolTable.alphabet()->get_dense ("sil").id(), 10);
    } catch (boost::exception &e) {
        std::cerr << "Unexpected error while parsing symbol table.\n";
        flipsta::explainException (std::cerr, e);

        BOOST_FAIL ("No exception should have been thrown.");
    }
}

BOOST_AUTO_TEST_SUITE_END()
