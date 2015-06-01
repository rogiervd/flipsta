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

#define BOOST_TEST_MODULE test_flipsta_map
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/map.hpp"

#include <vector>
#include "range/tuple.hpp"
#include "range/std.hpp"

BOOST_AUTO_TEST_SUITE(test_suite_flipsta_map)

/* Without default value. */

template <class Map> void checkMapWithoutDefault7 (Map & m) {
    BOOST_CHECK (!m.contains (2));
    BOOST_CHECK (!m.contains (6));
    BOOST_CHECK (m.contains (7));
    BOOST_CHECK_EQUAL (m [7], "bye");

    // Triggers an assertion.
    // m [6];
    m [7] = "doei";
    BOOST_CHECK_EQUAL (m [7], "doei");
}

template <class Map> void checkMapWithoutDefault67 (Map & m) {

    BOOST_CHECK (!m.contains (2));
    BOOST_CHECK (m.contains (6));
    BOOST_CHECK (m.contains (7));
    BOOST_CHECK_EQUAL (m [6], "hello");
    BOOST_CHECK_EQUAL (m [7], "bye");

    m.remove (6);
    checkMapWithoutDefault7 (m);
}

template <class Map> void checkMapWithoutDefault6 (Map & m) {
    BOOST_CHECK (!m.contains (2));
    BOOST_CHECK (m.contains (6));
    BOOST_CHECK (!m.contains (7));
    BOOST_CHECK_EQUAL (m [6], "hello");

    m.set (7, "bye");
    checkMapWithoutDefault67 (m);
}

template <class Map> void checkMapWithoutDefault() {
    // This causes a compile error.
    // Map m ("default");

    {
        Map m;
        m.set (6, "hello");
        checkMapWithoutDefault6 (m);
    }
    // With one initial value.
    {
        Map m (range::make_tuple (std::make_pair (6, "hello")));

        checkMapWithoutDefault6 (m);
    }
    // With two initial values.
    {
        std::vector <range::tuple <int, std::string>> initialValues;
        initialValues.push_back (range::make_tuple (7, "to be overwritten"));
        initialValues.push_back (range::make_tuple (6, "hello"));
        initialValues.push_back (range::make_tuple (7, "bye"));
        Map m (initialValues);

        checkMapWithoutDefault67 (m);
    }
}

// Without default value.
BOOST_AUTO_TEST_CASE (test_flipsta_Map_withoutDefault) {
    checkMapWithoutDefault <flipsta::Map <int, std::string>>();
    checkMapWithoutDefault <flipsta::Map <int, std::string, false>>();
    checkMapWithoutDefault <
        flipsta::Map <flipsta::Dense <int>, std::string, false, false>>();
}

/* With default value. */

template <class Map> void checkMapWithDefault7 (Map & m) {
    BOOST_CHECK (!m.contains (2));
    BOOST_CHECK (!m.contains (6));
    BOOST_CHECK (m.contains (7));
    BOOST_CHECK_EQUAL (m [2], "default");
    BOOST_CHECK_EQUAL (m [6], "default");
    BOOST_CHECK_EQUAL (m [7], "bye");
}

template <class Map> void checkMapWithDefault67 (Map & m) {
    BOOST_CHECK (!m.contains (2));
    BOOST_CHECK (m.contains (6));
    BOOST_CHECK (m.contains (7));
    BOOST_CHECK_EQUAL (m [2], "default");
    BOOST_CHECK_EQUAL (m [6], "hello");
    BOOST_CHECK_EQUAL (m [7], "bye");

    m.remove (6);
    checkMapWithDefault7 (m);
}

template <class Map> void checkMapWithDefault6 (Map & m) {
    BOOST_CHECK (!m.contains (2));
    BOOST_CHECK (m.contains (6));
    BOOST_CHECK (!m.contains (7));
    BOOST_CHECK_EQUAL (m [2], "default");
    BOOST_CHECK_EQUAL (m [6], "hello");
    BOOST_CHECK_EQUAL (m [7], "default");

    m.set (7, "bye");
    checkMapWithDefault67 (m);
}

template <class Map> void checkMapWithDefault() {
    {
        // This causes a static assert.
        // Map m;
        Map m ("default");

        BOOST_CHECK (!m.contains (6));
        m.set (6, "hello");
        checkMapWithDefault6 (m);
    }

    // With one initial value.
    {
        Map m ("default", range::make_tuple (std::make_pair (6, "hello")));

        checkMapWithDefault6 (m);
    }

    // With two initial values.
    {
        std::vector <range::tuple <int, std::string>> initialValues;
        initialValues.push_back (range::make_tuple (7, "to be overwritten"));
        initialValues.push_back (range::make_tuple (6, "hello"));
        initialValues.push_back (range::make_tuple (7, "bye"));
        Map m ("default", initialValues);

        checkMapWithDefault67 (m);
    }
}

/* With default, and always containing values. */

template <class Map> void checkMapAlwaysContain7 (Map & m) {
    BOOST_CHECK (m.contains (2));
    BOOST_CHECK (m.contains (6));
    BOOST_CHECK (m.contains (7));
    BOOST_CHECK_EQUAL (m [2], "default");
    BOOST_CHECK_EQUAL (m [6], "default");
    BOOST_CHECK_EQUAL (m [7], "bye");
}

template <class Map> void checkMapAlwaysContain67 (Map & m) {
    BOOST_CHECK (m.contains (2));
    BOOST_CHECK (m.contains (6));
    BOOST_CHECK (m.contains (7));
    BOOST_CHECK_EQUAL (m [2], "default");
    BOOST_CHECK_EQUAL (m [6], "hello");
    BOOST_CHECK_EQUAL (m [7], "bye");

    m.remove (6);
    checkMapAlwaysContain7 (m);
}

template <class Map> void checkMapAlwaysContain6 (Map & m) {
    BOOST_CHECK (m.contains (2));
    BOOST_CHECK (m.contains (6));
    BOOST_CHECK (m.contains (7));
    BOOST_CHECK_EQUAL (m [2], "default");
    BOOST_CHECK_EQUAL (m [6], "hello");
    BOOST_CHECK_EQUAL (m [7], "default");

    m.set (7, "bye");
    checkMapAlwaysContain67 (m);
}

template <class Map> void checkMapAlwaysContain() {
    {
        // This causes a static assert:
        // Map m;
        Map m ("default");
        BOOST_CHECK_EQUAL (m [227], "default");

        m.set (6, "hello");
        checkMapAlwaysContain6 (m);
    }
    // With one initial value.
    {
        Map m ("default", range::make_tuple (std::make_pair (6, "hello")));

        checkMapAlwaysContain6 (m);
    }

    // With two initial values.
    {
        std::vector <range::tuple <int, std::string>> initialValues;
        initialValues.push_back (range::make_tuple (7, "to be overwritten"));
        initialValues.push_back (range::make_tuple (6, "hello"));
        initialValues.push_back (range::make_tuple (7, "bye"));
        Map m ("default", initialValues);

        checkMapAlwaysContain67 (m);
    }
}

// With default value.
BOOST_AUTO_TEST_CASE (test_flipsta_Map_withDefault) {
    checkMapWithDefault <flipsta::Map <int, std::string, true, false>>();
    checkMapWithDefault <
        flipsta::Map <flipsta::Dense <int>, std::string, true, false>>();

    checkMapAlwaysContain <flipsta::Map <int, std::string, true, true>>();
    checkMapAlwaysContain <
        flipsta::Map <flipsta::Dense <int>, std::string, true, true>>();

    // Check that some operations do not involve actually addressing lots of
    // memory when using Dense.
    {
        flipsta::Map <flipsta::Dense <std::size_t>, std::string, true, true>
            map ("OK");

        BOOST_CHECK_EQUAL (map [std::size_t (-1)], "OK");
        map.remove (std::size_t (-1));
    }
}

BOOST_AUTO_TEST_SUITE_END()
