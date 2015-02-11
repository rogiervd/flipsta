/*
Copyright 2015 Rogier van Dalen.

This file is part of Rogier van Dalen's Mathematical tools library for C++.

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

#include <boost/python.hpp>

#include "semiring.hpp"

namespace flipsta { namespace python {

    class ConvertPythonObjectToSemiring {
    public:
        ConvertPythonObjectToSemiring() {
            boost::python::converter::registry::push_back (
                &convertible, &construct, boost::python::type_id <Semiring>());
        }

    private:
        // Always convertible.
        static void * convertible (PyObject * pointer)
        { return pointer; }

        static void construct (PyObject* python_object,
            boost::python::converter::rvalue_from_python_stage1_data * data)
        {
            boost::python::object object (boost::python::handle<> (
                boost::python::borrowed (python_object)));
            typedef boost::python::converter::
                rvalue_from_python_storage <Semiring> storage_type;
            void * storage =
                reinterpret_cast <storage_type*> (data)->storage.bytes;

            new (storage) Semiring (object);

            data->convertible = storage;
        }
    };

    struct ConvertSemiringToPython {
        static PyObject* convert (Semiring const & s)
        { return boost::python::incref (s.underlying().ptr()); }
    };

} // namespace python

} // namespace flipsta

template <class Other, class Cls> void defineOperatorsWith (Cls & cls) {
    using namespace boost::python;
    cls.def (self == other <Other>());
    cls.def (self != other <Other>());
    cls.def (other <Other>() == self);
    cls.def (other <Other>() != self);

    cls.def (self + other <Other>());
    cls.def (other <Other>() + self);

    cls.def (self * other <Other>());
    cls.def (other <Other>() * self);
}

template<> PyObject * flipsta::python::Singleton <flipsta::python::Zero>
    ::pythonSingleton = nullptr;
template<> PyObject * flipsta::python::Singleton <flipsta::python::One>
    ::pythonSingleton = nullptr;

void exposeSemiring() {
    using namespace boost::python;
    using namespace flipsta::python;

    auto zeroClass = class_ <Zero, boost::shared_ptr <Zero>, boost::noncopyable>
        ("ZeroType", no_init)
        .def (self == self)
        .def (self != self)
        .def (self + self)
        .def (self * self)
        // self_ns:: is needed to disambiguate.
        .def (self_ns::str (self))
        .def ("__nonzero__", &Zero::nonzero)
        .add_static_property ("instance", &Zero::get)
        ;
    defineOperatorsWith <One> (zeroClass);
    // The semiring must define operators with Zero and One.

    auto oneClass = class_ <One, boost::shared_ptr <One>, boost::noncopyable>
        ("OneType", no_init)
        .def (self == self)
        .def (self != self)
        .def (self * self)
        .def (self_ns::str (self))
        .add_static_property ("instance", &One::get)
        ;
    defineOperatorsWith <Zero> (oneClass);

    Zero::initialise();
    One::initialise();
    scope().attr ("Zero") = Zero::get();
    scope().attr ("One") = One::get();

    ConvertPythonObjectToSemiring();
    to_python_converter <Semiring, ConvertSemiringToPython>();
}
