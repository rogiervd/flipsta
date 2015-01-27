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
