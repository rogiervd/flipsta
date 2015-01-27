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

/** \file
Test flipsta::draw.

This outputs a text file in Graphviz dot format.
This test does not actually test whether the format is correct at all, just
whether draw() runs.
If no argument is given on the command line, then the output goes into a
stringstream and then disappears.
If one argument is given on the command line, this is the name of the output
file.
This can then be run through Graphviz dot, assuming it is installed, and it can
be checked that it actually does the right thing.
*/

#define BOOST_TEST_MODULE test_flipsta_draw_examples
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/draw.hpp"

#include <fstream>
#include <sstream>

#include "example_automata.hpp"

BOOST_AUTO_TEST_SUITE(test_suite_draw_examples)

BOOST_AUTO_TEST_CASE (testDrawExamples) {
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char ** argv = boost::unit_test::framework::master_test_suite().argv;

    if (argc >= 2) {
        std::ofstream outputFile (argv [1]);
        flipsta::draw (outputFile, acyclicExample());
    } else {
        std::stringstream output;
        flipsta::draw (output, acyclicExample());
    }

    if (argc >= 3) {
        std::ofstream outputFile (argv [2]);
        flipsta::draw (outputFile, acyclicSequenceExample());
    } else {
        std::stringstream output;
        flipsta::draw (output, acyclicSequenceExample());
    }
}

BOOST_AUTO_TEST_SUITE_END()
