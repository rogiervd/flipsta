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

/* \file
Declare functions to register automaton classes that can be called from the
module definition.
*/

#ifndef FLIPSTA_PYTHON_MODULE_HPP
#define FLIPSTA_PYTHON_MODULE_HPP

void exposeState();
void exposeSemiring();
void exposeAutomaton();
void exposeExceptions();

#endif // FLIPSTA_PYTHON_MODULE_HPP
