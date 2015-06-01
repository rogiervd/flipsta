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

#ifndef FLIPSTA_PYTHON_SEMIRING_HPP_INCLUDED
#define FLIPSTA_PYTHON_SEMIRING_HPP_INCLUDED

#include <boost/python/object.hpp>
#include <boost/python/handle.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/str.hpp>

#include "rime/nothing.hpp"

#include "math/magma.hpp"

namespace flipsta { namespace python {

    using boost::python::object;

    /**
    A dangerous-to-use singleton which makes it possible to expose a singleton
    object to Python, always using the same Python object.
    This makes it possible to use "is" to compare objects.
    "initialise()" should be called in the module initialisation, and from that
    moment "get()" returns a copy of the Python object representing the C++
    singleton object.

    The PyObject remains alive after the library has gone out of memory.
    Sorry.
    At library exit, the Python interpreter may already be done; then it's too
    late to unlink the object.
    However, not holding a reference to the object is also a problem, since it
    may disappear from the Python scope, and then get() will start returning
    dangling pointers.
    */
    template <class Type> class Singleton {
    private:
        static Type & getSingleton() {
            static Type singleton;
            return singleton;
        }

        static PyObject * pythonSingleton;

    public:
        static void initialise() {
            boost::python::object result = boost::python::make_function (
                &getSingleton,
                boost::python::return_value_policy <
                    boost::python::reference_existing_object>()) ();
            pythonSingleton = boost::python::incref (result.ptr());
        }

        static boost::python::object get() {
            return boost::python::object (boost::python::handle<> (
                boost::python::borrowed (pythonSingleton)));
        }
    };

    struct Zero : Singleton <Zero> {
    private:
        Zero() {}
        friend class Singleton <Zero>;
    public:
        Zero (Zero const &) = delete;

        bool nonzero() const { return false; }
    };

    struct One : Singleton <One> {
    private:
        One() {}
        friend class Singleton <One>;
    public:
        One (One const &) = delete;
    };

    class Semiring {
        object underlying_;
    public:
        explicit Semiring (object const & underlying)
        : underlying_ (underlying) {}

        Semiring (Zero const &) : underlying_ (Zero::get()) {}
        Semiring (One const &) : underlying_ (One::get()) {}

        boost::python::object const & underlying() const { return underlying_; }
    };

    struct SemiringTag;

    /* Comparison. */
    inline bool operator == (Zero const &, Zero const &) { return true; }
    inline bool operator != (Zero const &, Zero const &) { return false; }

    inline bool operator == (One const &, One const &) { return true; }
    inline bool operator != (One const &, One const &) { return false; }

    inline bool operator== (Semiring const & left, Semiring const & right)
    { return left.underlying() == right.underlying(); }
    inline bool operator!= (Semiring const & left, Semiring const & right)
    { return left.underlying() != right.underlying(); }

    // Cross-comparison.
    inline bool operator == (Zero const &, One const &) { return false; }
    inline bool operator != (Zero const &, One const &) { return true; }

    inline bool operator == (One const &, Zero const &) { return false; }
    inline bool operator != (One const &, Zero const &) { return true; }

    inline bool operator== (Zero const &, Semiring const & right)
    { return right.underlying() == Zero::get(); }
    inline bool operator!= (Zero const &, Semiring const & right)
    { return right.underlying() != Zero::get(); }

    inline bool operator== (Semiring const & left, Zero const &)
    { return left.underlying() == Zero::get(); }
    inline bool operator!= (Semiring const & left, Zero const &)
    { return left.underlying() != Zero::get(); }

    inline bool operator== (One const &, Semiring const & right)
    { return right.underlying() == One::get(); }
    inline bool operator!= (One const &, Semiring const & right)
    { return right.underlying() != One::get(); }

    inline bool operator== (Semiring const & left, One const &)
    { return left.underlying() == One::get(); }
    inline bool operator!= (Semiring const & left, One const &)
    { return left.underlying() != One::get(); }

    // Plus.
    inline Semiring operator+ (Zero const &, Zero const & right)
    { return right; }

    inline Semiring operator+ (Zero const &, One const & right)
    { return right; }

    inline Semiring operator+ (One const & left, Zero const &) { return left; }

    // One + One is not defined.

    // This is not exposed to Python.
    inline Semiring operator+ (Semiring const & left, Semiring const & right)
    { return Semiring (left.underlying() + right.underlying()); }

    // Times.
    inline Semiring operator* (Zero const & left, Zero const & right)
    { return left; }

    inline Semiring operator* (Zero const & left, One const & right)
    { return left; }

    inline Semiring operator* (One const & left, Zero const & right)
    { return right; }

    inline Semiring operator* (One const & left, One const & right)
    { return right; }

    // This is not exposed to Python.
    inline Semiring operator* (Semiring const & left, Semiring const & right)
    { return Semiring (left.underlying() * right.underlying()); }

    // Print.
    inline std::ostream & operator<< (std::ostream & os, Zero const &)
    { return os << "<Zero>"; }

    inline std::ostream & operator<< (std::ostream & os, One const &)
    { return os << "<One>"; }

    inline std::ostream & operator<< (std::ostream & os, Semiring const & s) {
        std::string str = boost::python::extract <std::string> (
            boost::python::str (s.underlying()));
        return os << str;
    }

}} // namespace python::flipsta

namespace math {

template<> struct decayed_magma_tag <flipsta::python::Semiring>
{ typedef flipsta::python::SemiringTag type; };

template<> struct decayed_magma_tag <flipsta::python::Zero>
{ typedef flipsta::python::SemiringTag type; };

template<> struct decayed_magma_tag <flipsta::python::One>
{ typedef flipsta::python::SemiringTag type; };

namespace operation {

    template <> struct equal <flipsta::python::SemiringTag>
    : operator_equal <bool> {};

    template <> struct identity <flipsta::python::SemiringTag, callable::times>
    {
        flipsta::python::Semiring operator() () const
        { return flipsta::python::Semiring (flipsta::python::One::get()); }
    };

    template <> struct identity <flipsta::python::SemiringTag, callable::plus>
    {
        flipsta::python::Semiring operator() () const
        { return flipsta::python::Semiring (flipsta::python::Zero::get()); }
    };

    template <>
        struct annihilator <flipsta::python::SemiringTag, callable::times>
    {
        flipsta::python::Semiring operator() () const
        { return flipsta::python::Semiring (flipsta::python::Zero::get()); }
    };

    /* Operations. */

    // "choose" is not defined: it is not known whether it should be the
    // maximum or the minimum.

    template <> struct times <flipsta::python::SemiringTag>
    : operator_times <flipsta::python::Semiring>, associative {};

    template <> struct plus <flipsta::python::SemiringTag>
    : operator_plus <flipsta::python::Semiring>, associative, commutative {};

    template <> struct is_semiring <flipsta::python::SemiringTag, either,
        callable::times, callable::plus>
    : rime::true_type {};

    template <> struct print <flipsta::python::SemiringTag>
    : operator_shift_left_stream {};

    // unify_type.
    template <class Magma1, class Magma2>
        struct unify_type <flipsta::python::SemiringTag, Magma1, Magma2>
    { typedef flipsta::python::Semiring type; };

    template <class Magma>
        struct unify_type <flipsta::python::SemiringTag, Magma, Magma>
    { typedef Magma type; };

} // namespace operation

} // namespace math

static_assert (std::is_same <
    math::result_of <math::callable::zero <flipsta::python::One>()>::type,
    flipsta::python::Semiring
    >::value, "");

#endif // FLIPSTA_PYTHON_SEMIRING_HPP_INCLUDED
