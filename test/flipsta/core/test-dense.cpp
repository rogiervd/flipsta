/*
Copyright 2014 Rogier van Dalen.

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

#define BOOST_TEST_MODULE test_core_Dense
#include "utility/test/boost_unit_test.hpp"

#include "flipsta/core.hpp"

#include <type_traits>

#include <boost/functional/hash.hpp>

BOOST_AUTO_TEST_SUITE(test_suite_core)

BOOST_AUTO_TEST_CASE (test_Dense) {
    typedef flipsta::Dense <short> Dense;

    static_assert (!std::is_constructible <Dense>::value,
        "Not default-constructible.");

    Dense zero = short (0);
    BOOST_CHECK_EQUAL (short (zero), 0);

    Dense two = short (2);
    BOOST_CHECK_EQUAL (short (two), 2);
    Dense thirtySeven = short (37);
    BOOST_CHECK_EQUAL (short (thirtySeven), 37);

    // operator ==
    BOOST_CHECK (zero == zero);
    BOOST_CHECK (!(zero == two));
    BOOST_CHECK (!(zero == thirtySeven));

    BOOST_CHECK (!(two == zero));
    BOOST_CHECK (two == two);
    BOOST_CHECK (!(two == thirtySeven));

    BOOST_CHECK (!(thirtySeven == zero));
    BOOST_CHECK (!(thirtySeven == two));
    BOOST_CHECK (thirtySeven == thirtySeven);

    // operator !=
    BOOST_CHECK (!(zero != zero));
    BOOST_CHECK (zero != two);
    BOOST_CHECK (zero != thirtySeven);

    BOOST_CHECK (two != zero);
    BOOST_CHECK (!(two != two));
    BOOST_CHECK (two != thirtySeven);

    BOOST_CHECK (thirtySeven != zero);
    BOOST_CHECK (thirtySeven != two);
    BOOST_CHECK (!(thirtySeven != thirtySeven));

    // operator <
    BOOST_CHECK (!(zero < zero));
    BOOST_CHECK (zero < two);
    BOOST_CHECK (zero < thirtySeven);

    BOOST_CHECK (!(two < zero));
    BOOST_CHECK (!(two < two));
    BOOST_CHECK (two < thirtySeven);

    BOOST_CHECK (!(thirtySeven < zero));
    BOOST_CHECK (!(thirtySeven < two));
    BOOST_CHECK (!(thirtySeven < thirtySeven));

    // operator <=
    BOOST_CHECK (zero <= zero);
    BOOST_CHECK (zero <= two);
    BOOST_CHECK (zero <= thirtySeven);

    BOOST_CHECK (!(two <= zero));
    BOOST_CHECK (two <= two);
    BOOST_CHECK (two <= thirtySeven);

    BOOST_CHECK (!(thirtySeven <= zero));
    BOOST_CHECK (!(thirtySeven <= two));
    BOOST_CHECK (thirtySeven <= thirtySeven);

    // operator >
    BOOST_CHECK (!(zero > zero));
    BOOST_CHECK (!(zero > two));
    BOOST_CHECK (!(zero > thirtySeven));

    BOOST_CHECK (two > zero);
    BOOST_CHECK (!(two > two));
    BOOST_CHECK (!(two > thirtySeven));

    BOOST_CHECK (thirtySeven > zero);
    BOOST_CHECK (thirtySeven > two);
    BOOST_CHECK (!(thirtySeven > thirtySeven));

    // operator >=
    BOOST_CHECK (zero >= zero);
    BOOST_CHECK (!(zero >= two));
    BOOST_CHECK (!(zero >= thirtySeven));

    BOOST_CHECK (two >= zero);
    BOOST_CHECK (two >= two);
    BOOST_CHECK (!(two >= thirtySeven));

    BOOST_CHECK (thirtySeven >= zero);
    BOOST_CHECK (thirtySeven >= two);
    BOOST_CHECK (thirtySeven >= thirtySeven);
}

BOOST_AUTO_TEST_CASE (test_Dense_hash) {
    boost::hash <long> longHasher;
    typedef flipsta::Dense <long> Dense;
    boost::hash <Dense> denseHasher;

    BOOST_CHECK_EQUAL (longHasher (0l), denseHasher (Dense (0l)));
    BOOST_CHECK_EQUAL (longHasher (6l), denseHasher (Dense (6l)));
    BOOST_CHECK_EQUAL (longHasher (5698712l), denseHasher (Dense (5698712l)));
}

BOOST_AUTO_TEST_SUITE_END()
