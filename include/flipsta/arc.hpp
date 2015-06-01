/*
Copyright 2014, 2015 Rogier van Dalen.

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

#ifndef FLIPSTA_ARC_HPP_INCLUDED
#define FLIPSTA_ARC_HPP_INCLUDED

#include "core.hpp"

namespace flipsta {

/** \brief
An arc type for automata that stores its data explicitly.

And arc is characterised by three pieces of information, which this class holds
explicitly:
\li The state it comes from, the source state.
\li The state it goes to, the destination state.
\li The label on the arc.

\tparam State The type of the states.
\tparam Label The type of the label.
*/
template <class State_, class Label_> class ExplicitArc {
public:
    /// The type of the states.
    typedef State_ State;
    /// The type of the label.
    typedef Label_ Label;
private:
    State source_;
    State destination_;
    Label label_;

public:
    /** \brief
    Construct with the data explicitly.

    \param direction
        Indicate that the second argument is the source and the third the
        destination.
    \param source
        The source state.
    \param destination
        The destination state.
    \param label
        The label on the arc.
    */
    ExplicitArc (Forward const & direction,
        State const & source, State const & destination, Label const & label)
    : source_ (source), destination_ (destination), label_ (label) {}

    /** \brief
    Construct with the data explicitly.

    \param direction
        Indicate that the second argument is the destination and the third the
        source.
    \param destination
        The destination state.
    \param source
        The source state.
    \param label
        The label on the arc.
    */
    ExplicitArc (Backward const & direction,
        State const & destination, State const & source, Label const & label)
    : source_ (source), destination_ (destination), label_ (label) {}

    /** \brief
    Construct by copying the data from another arc.
    */
    template <class OtherArc> explicit ExplicitArc (OtherArc const & other)
    : source_ (other.state (backward)), destination_ (other.state (forward)),
        label_ (other.label()) {}

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
