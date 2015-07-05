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
        // For Python 2.
        .def ("__nonzero__", &Zero::nonzero)
        // For Python 3.
        .def ("__bool__", &Zero::nonzero)
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
