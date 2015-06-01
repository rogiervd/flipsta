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
        ExplicitArc <int, double> a (forward, 4, 5, 7.5);
        BOOST_CHECK_EQUAL (a.state (backward), 4);
        BOOST_CHECK_EQUAL (a.state (forward), 5);
        BOOST_CHECK_EQUAL (a.label(), 7.5);
    }
    {
        ExplicitArc <int, double> a (backward, 4, 5, 7.5);
        BOOST_CHECK_EQUAL (a.state (forward), 4);
        BOOST_CHECK_EQUAL (a.state (backward), 5);
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
