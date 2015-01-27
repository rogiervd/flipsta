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

#ifndef FLIPSTA_ARC_HPP_INCLUDED
#define FLIPSTA_ARC_HPP_INCLUDED

namespace flipsta {

/**
\brief An arc type that stores its data explicitly.

\tparam State The type of the states.
\tparam Label The type of the label.
*/
template <class State, class Label> class ExplicitArc {
    State source_;
    State destination_;
    Label label_;

public:
    ExplicitArc (State const & source, State const & destination,
        Label const & label)
    : source_ (source), destination_ (destination), label_ (label) {}

    /**
    \brief Returns the source state.
    */
    State const & state (Backward) const { return source_; }

    /**
    \brief Returns the destination state.
    */
    State const & state (Forward) const { return destination_; }

    /**
    \brief Returns the label on the arc.
    */
    Label const & label() const { return label_; }
};

} // namespace flipsta

#endif // FLIPSTA_ARC_HPP_INCLUDED
