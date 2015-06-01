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
