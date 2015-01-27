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

#define BOOST_TEST_MODULE test_flipsta_queue
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/queue.hpp"

BOOST_AUTO_TEST_SUITE(test_suite_flipsta_queue)

using flipsta::LifoQueue;

BOOST_AUTO_TEST_CASE (test_flipstaLifoQueue) {
    LifoQueue <int> queue;
    BOOST_CHECK (queue.empty());
    queue.push (1);
    BOOST_CHECK_EQUAL (queue.head(), 1);

    queue.push (17);
    BOOST_CHECK_EQUAL (queue.head(), 17);
    // Mutate the head.
    queue.head() = 18;
    BOOST_CHECK_EQUAL (queue.head(), 18);
    int e = queue.pop();
    BOOST_CHECK_EQUAL (e, 18);
    BOOST_CHECK (!queue.empty());
    e = queue.pop();
    BOOST_CHECK_EQUAL (e, 1);
    BOOST_CHECK (queue.empty());

    queue.push (-87);
    BOOST_CHECK (!queue.empty());
    e = queue.pop();
    BOOST_CHECK_EQUAL (e, -87);
    BOOST_CHECK (queue.empty());
}

BOOST_AUTO_TEST_SUITE_END()
