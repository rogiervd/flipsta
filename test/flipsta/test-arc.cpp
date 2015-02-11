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

#define BOOST_TEST_MODULE test_flipsta_arc
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/arc.hpp"

#include <string>

BOOST_AUTO_TEST_SUITE(test_suite_flipsta_arc)

using flipsta::forward;
using flipsta::backward;

using flipsta::ExplicitArc;

struct OtherArc {
    typedef short State;
    typedef std::string Label;

    short state (flipsta::Backward) const { return -20; }
    short state (flipsta::Forward) const { return +147; }

    Label label() const { return "hello"; }
};

BOOST_AUTO_TEST_CASE (test_flipstaExplicitArc) {
    {
        ExplicitArc <int, double> a (4, 5, 7.5);
        BOOST_CHECK_EQUAL (a.state (backward), 4);
        BOOST_CHECK_EQUAL (a.state (forward), 5);
        BOOST_CHECK_EQUAL (a.label(), 7.5);
    }
    {
        OtherArc other;
        ExplicitArc <short, std::string> a (other);
        BOOST_CHECK_EQUAL (a.state (backward), -20);
        BOOST_CHECK_EQUAL (a.state (forward), +147);
        BOOST_CHECK_EQUAL (a.label(), "hello");
    }
}

BOOST_AUTO_TEST_SUITE_END()
