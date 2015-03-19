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

#ifndef FLIPSTA_TRANSFORM_LABELS_HPP_INCLUDED
#define FLIPSTA_TRANSFORM_LABELS_HPP_INCLUDED

#include <utility>
#include <type_traits>

#include "utility/returns.hpp"

#include "math/magma.hpp"

#include "core.hpp"
#include "label.hpp"

// This is necessary to be able to refer to the function topologicalOrder.
#include "topological_order.hpp"

namespace flipsta {

/** \brief
Wrapper around an automaton that transforms the labels on the fly.

The function must be able to convert both general labels and terminal labels,
which can be of different types.

Normally, objects of this class should be produced using \c transformLabels.

\tparam Underlying
    The underlying automaton.
    This can be a reference, in which case the responsibility for keeping it
    alive rests with the caller.
\tparam Function
    Type of the function that transforms the internal label type.
\tparam Tag
    The label tag of the wrapper.
    This must be compatible with the label type that Function results in.
*/
template <class Underlying, class Function, class Tag>
    class TransformedLabelAutomaton;

/** \brief
Return an automaton that transforms labels on the fly.

This goes for both labels on the arcs and for terminal labels.
The function gets passed the compressed label of the underlying automaton,
and should return a compressed label for the resulting automaton.
\a tag is used to convert this label to an expanded label when required.

The transformation is allowed to keep the type of the labels the same, or to
change it.
Its return type does always need to be in one semiring.

\sa transformExpandedLabels

\todo Describe this somewhere in the documentation.

\param underlying
    The underlying automaton.
    If this is a reference type, the reference is kept, and the caller is
    responsible for keeping it alive as long as necessary.
\param function
    The function used to transform the original compressed labels to the new
    compressed labels.
\param tag
    The label tag to use for the automaton with transformed labels.
    This must be compatible with the label type that \a function returns.

\internal
Since this works on the underlying representation, it is not possible to
automatically determine the tag.
*/
template <class Underlying, class Function, class Tag> inline
    TransformedLabelAutomaton <Underlying, Function, Tag>
    transformLabels (Underlying && underlying,
        Function const & function, Tag const & tag)
{
    return TransformedLabelAutomaton <Underlying, Function, Tag> (
        std::forward <Underlying> (underlying), function, tag);
}

namespace transform_label_detail {

    /** \brief
    Allow a transformation function on expanded, not compressed, labels.

    This requires a little dance where the original compressed label is
    converted into an original expanded label, converted to the new expanded
    label by the function, and converted into the new compressed label.

    \todo When expanded labels are required (e.g. in arcsOn), compressing and
    expanding is extra work, so specialise arcsOn for this case.
    */
    template <class OldTag, class NewTag, class Function>
        class ExpandAndCompress
    {
        OldTag oldTag_;
        NewTag newTag_;
        Function function_;

    public:
        ExpandAndCompress (
            OldTag const & oldTag, NewTag const & newTag,
            Function const & function)
        : oldTag_ (oldTag), newTag_ (newTag), function_ (function) {}

        template <class OldCompressedLabel>
            auto operator() (OldCompressedLabel const & label) const
        RETURNS (newTag_.compress() (function_ (oldTag_.expand() (label))));
    };

} // namespace transform_label_detail

/** \brief
Return an automaton that transforms expanded labels on the fly.

This goes for both labels on the arcs and for terminal labels.
Unlike for \ref transformLabels, the function gets passed the expanded label of
the underlying automaton, and should return an expanded label for the resulting
automaton.
\a tag is used to convert this label to a compressed label when required.

The transformation is allowed to keep the type of the labels the same, or to
change it.
Its return type does always need to be in one semiring.

\sa transformLabels

\param underlying
    The underlying automaton.
    If this is a reference type, the reference is kept, and the caller is
    responsible for keeping it alive as long as necessary.
\param function
    The function used to transform the original expanded labels to the new
    expanded labels.
\param tag
    The label tag to use for the automaton with transformed labels.
    This must be compatible with the label type that \a function returns.

\todo It might be possible to automatically determine the default tag.
*/
template <class Underlying, class Function, class Tag,
    class ExpandAndCompress = transform_label_detail::ExpandAndCompress <
        typename TagType <Underlying>::type, Tag, Function>>
    inline TransformedLabelAutomaton <Underlying, ExpandAndCompress, Tag>
    transformExpandedLabels (Underlying && underlying,
        Function const & function, Tag const & tag)
{
    return transformLabels (std::forward <Underlying> (underlying),
        transform_label_detail::ExpandAndCompress <
            typename TagType <Underlying>::type, Tag, Function> (
                underlying.tag(), tag, function), tag);
}

struct TransformLabelsTag;

template <class Underlying, class Function, class Tag>
    struct AutomatonTagUnqualified <
        TransformedLabelAutomaton <Underlying, Function, Tag>>
{ typedef TransformLabelsTag type; };

template <class Underlying, class Function, class Tag>
    class TransformedLabelAutomaton
{
public:
    typedef typename std::decay <Underlying>::type DecayedUnderlying;

    typedef typename DecayedUnderlying::State State;

    typedef typename CompressedLabelType <DecayedUnderlying>::type
        CompressedUnderlyingLabel;
    typedef typename std::result_of <Function (CompressedUnderlyingLabel)>::type
        CompressedLabel;
    typedef typename label::ExpandedLabelType <Tag, CompressedLabel>::type
        Label;

    typedef typename DecayedUnderlying::CompressedTerminalLabel
        CompressedUnderlyingTerminalLabel;
    typedef typename std::result_of <
            Function (CompressedUnderlyingTerminalLabel)>::type
        CompressedTerminalLabel;
    typedef typename label::ExpandedLabelType <Tag, CompressedTerminalLabel
        >::type TerminalLabel;

    static_assert (std::is_same <
        typename math::magma_tag <CompressedLabel>::type,
        typename math::magma_tag <CompressedTerminalLabel>::type>::value,
        "The function must return labels and terminal labels that are "
        "in the same semiring.");

private:
    Underlying underlying_;
    Function function_;
    Tag tag_;

public:
    TransformedLabelAutomaton (Underlying && underlying,
        Function const & function, Tag const & tag)
    : underlying_ (underlying), function_ (function), tag_ (tag) {}

    Underlying const & underlying() const { return underlying_; }

    Tag const & tag() const { return tag_; }

    auto states() const RETURNS (flipsta::states (underlying_));

    // The topological order may be specialised for the underlying automaton,
    // and it is bound to be faster anyway, so forward to it.
    template <class Direction>
        auto topologicalOrder (Direction const & direction) const
    RETURNS (flipsta::topologicalOrder (underlying_, direction));

    auto hasState (State const & state) const
    RETURNS (flipsta::hasState (underlying_, state));

    template <class Direction>
        auto terminalStatesCompressed (Direction const & direction) const
    RETURNS (transformation::TransformLabelsForStates() (function_,
        flipsta::terminalStatesCompressed (underlying_, direction)));

    template <class Direction>
        auto terminalLabelCompressed (
            Direction const & direction, State const & state) const
    RETURNS (function_ (
        flipsta::terminalLabelCompressed (underlying_, direction, state)));

    template <class Direction>
        auto arcsOnCompressed (
            Direction const & direction, State const & state) const
    RETURNS (transformation::TransformLabelsOnArcs() (function_,
        flipsta::arcsOnCompressed (underlying_, direction, state)));
};

} // namespace flipsta

#endif // FLIPSTA_TRANSFORM_LABELS_HPP_INCLUDED
