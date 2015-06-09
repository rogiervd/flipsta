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

#include "state.hpp"

namespace flipsta { namespace python {

    class ConvertPythonObjectToState {
    public:
        ConvertPythonObjectToState() {
            boost::python::converter::registry::push_back (
                &convertible, &construct, boost::python::type_id <State>());
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
                rvalue_from_python_storage <State> storage_type;
            void * storage =
                reinterpret_cast <storage_type*> (data)->storage.bytes;

            new (storage) State (object);

            data->convertible = storage;
        }
    };

    struct ConvertStateToPython {
        static PyObject* convert (State const & s)
        { return boost::python::incref (s.underlying().ptr()); }
    };

} // namespace python

} // namespace flipsta

void exposeState() {
    using namespace boost::python;
    using namespace flipsta::python;

    ConvertPythonObjectToState();
    to_python_converter <State, ConvertStateToPython>();
}
