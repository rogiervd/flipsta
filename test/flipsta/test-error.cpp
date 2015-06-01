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

#define BOOST_TEST_MODULE test_error
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/error.hpp"

#include <boost/exception/get_error_info.hpp>

BOOST_AUTO_TEST_SUITE(test_suite_error)

BOOST_AUTO_TEST_CASE (testError) {
    BOOST_CHECK_THROW (throw flipsta::Error(), std::exception);
    BOOST_CHECK_THROW (throw flipsta::Error(), boost::exception);

    typedef int State;
    try {
        // Throw error related to state 5.
        throw flipsta::Error() << flipsta::errorInfoState <State> (5);
    } catch (boost::exception & error) {
        State const * state = boost::get_error_info <
            flipsta::TagErrorInfoStateType <State>::type> (error);
        BOOST_CHECK (!!state);
        if (state) {
            BOOST_CHECK_EQUAL (*state, 5);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
