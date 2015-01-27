/*
Copyright 2015 Rogier van Dalen.

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
