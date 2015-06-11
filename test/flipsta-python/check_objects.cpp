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

/** \file
Test flipsta::python::State and flipsta::python::Semiring.
These are fairly simple wrappers around Python objects that are used as states
or as labels, respectively.
*/

#include <cassert>
#include <vector>
#include <iostream>
#include <ostream>

#include <boost/python.hpp>
#include <boost/functional/hash.hpp>

#include "range/core.hpp"
#include "range/for_each_macro.hpp"
#include "range/python/range.hpp"
#include "range/std/container.hpp"

// These includes are really internal to the library, hence the relative paths.
#include "../source/flipsta-python/state.hpp"
#include "../source/flipsta-python/semiring.hpp"

using boost::python::object;
using flipsta::python::State;
using flipsta::python::Semiring;

/** \brief
Check equality for Python objects.

If two objects are equal, they are not unequal, and they have the same hash
value.
If not, they are unequal, and there should be an extremely high probability that
they have different hash values.

Return number of pairs that are the same.
*/
template <class Type, class Examples>
    int checkEquality (Examples const & examples)
{
    boost::hash <Type> hasher;
    int equalNum = 0;
    std::cout << "Checking equality and hash values for:\n";
    RANGE_FOR_EACH (left, examples) {
        std::cout << "    " << left << " with hash " << hasher (left)
            << std::endl;
    }
    RANGE_FOR_EACH (left, examples) {
        RANGE_FOR_EACH (right, examples) {
            assert ((left == right) == !(left != right));
            if (left == right) {
                ++ equalNum;
                assert (hasher (left) == hasher (right));
            } else {
                assert (hasher (left) != hasher (right));
            }
        }
    }
    return equalNum;
}

/** \brief
Check a sequence of states for internal consistency.
*/
int checkStates (range::python_range <State> examples_) {
    std::vector <State> examples;
    RANGE_FOR_EACH (state, examples_)
        examples.push_back (state);

    return checkEquality <State> (examples);
}

/** \brief
Check a sequence of semiring values for internal consistency.

\param examples_ The examples.
\param checkSemiringProperties
    Perform some checks of the properties of a semiring.
    This should only be performed if none of the examples is flipsta.One.
    (<c>one + one</c> cannot be implemented.)

\todo Instead of using this, implement checking Python semirings properly.
If math::check_semiring() did not rely on Boost.Test, it could be called from
Python.
*/
int checkSemiring (
    range::python_range <Semiring> examples_, bool checkSemiringProperties)
{
    std::vector <Semiring> examples;
    RANGE_FOR_EACH (state, examples_)
        examples.push_back (state);

    int equalCount = checkEquality <Semiring> (examples);

    if (checkSemiringProperties) {
        RANGE_FOR_EACH (example1, examples) {
            RANGE_FOR_EACH (example2, examples) {
                // + must be commutative.
                {
                    Semiring result1 = example1 + example2;
                    Semiring result2 = example1 + example2;
                    assert (result1 == result2);
                }

                RANGE_FOR_EACH (example3, examples) {
                    // + must be associative.
                    {
                        Semiring result1 = (example1 + example2) + example3;
                        Semiring result2 = example1 + (example2 + example3);
                        assert (result1 == result2);
                    }

                    // * must be associative.
                    {
                        Semiring result1 = (example1 * example2) * example3;
                        Semiring result2 = example1 * (example2 * example3);
                        assert (result1 == result2);
                    }

                    // * must distribute over +.
                    {
                        Semiring result1 = example1 * (example2 + example3);
                        Semiring result2
                            = example1 * example2 + example1 * example3;
                        assert (result1 == result2);
                    }
                    {
                        Semiring result1 = (example1 + example2) * example3;
                        Semiring result2
                            = example1 * example3 + example2 * example3;
                        assert (result1 == result2);
                    }

                    // To check zero and one, static variables from another
                    // Python extension must be loaded, which is not currently
                    // working.
                }
            }
        }
    }

    return equalCount;
}

BOOST_PYTHON_MODULE (check_objects) {
    using namespace boost::python;

    def ("check_states", &checkStates);
    def ("check_semiring", &checkSemiring);

    range::python::convert_object_to_range <range::python_range <State>>();
    range::python::convert_object_to_range <range::python_range <Semiring>>();
}
