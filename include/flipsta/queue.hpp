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

#ifndef FLIPSTA_QUEUE_HPP_INCLUDED
#define FLIPSTA_QUEUE_HPP_INCLUDED

#include <stack>
#include <vector>

namespace flipsta {

/**
\brief Last-in, first-out queue.

This implements a stack: elements that are pushed onto this queue last are
popped off first.
*/
template <class Element> class LifoQueue {
    std::stack <Element, std::vector <Element>> data_;
public:
    /// \brief Return whether this queue is empty.
    bool empty() const { return data_.empty(); }

    /** \brief
    Push an element onto the queue.
    */
    void push (Element const & element) { return data_.push (element); }

    /** \brief
    Return the next element that will be returned by pop().

    Does not remove the element.

    \pre \c !empty().
    */
    Element const & head() const { return data_.top(); }

    /** \brief
    Return a reference to the next element that will be returned by pop().

    Does not remove the element.

    \pre \c !empty().
    */
    Element & head() { return data_.top(); }

    /**
    \brief Pop an element off the stack.

    This is the last element that was pushed onto the stack that has not yet
    been popped off.

    \pre \c !empty().
    */
    Element pop() {
        Element e = data_.top();
        data_.pop();
        return e;
    }
};

} // namespace flipsta

#endif // FLIPSTA_QUEUE_HPP_INCLUDED
