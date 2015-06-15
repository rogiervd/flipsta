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

/** \file
Define structures related to labels.
*/

#ifndef FLIPSTA_LABEL_HPP_INCLUDED
#define FLIPSTA_LABEL_HPP_INCLUDED

#include <memory>
#include <type_traits>

#include "utility/returns.hpp"

#include "rime/core.hpp"

#include "range/equal.hpp"
#include "range/transform.hpp"
#include "range/zip.hpp"
#include "range/call_unpack.hpp"

#include "math/magma.hpp"
#include "math/generalise_type.hpp"
#include "math/product.hpp"
#include "math/lexicographical.hpp"
#include "math/sequence.hpp"
#include "math/alphabet.hpp"

namespace flipsta {

namespace label {

    /** \brief
    Evaluate to a type that can hold \c Label and the value of "zero" in
    the same semiring.

    This is useful for \c terminalLabel, which needs to be able to return zero
    when the state is not a start (or final) state.
    */
    template <class Label> struct GeneraliseToZero
    : math::merge_magma::apply <Label,
        typename std::result_of <math::callable::zero <Label>()>::type> {};

    /** \brief
    Evaluate to a label type that generalises \a Label.

    The resulting type can hold values of type \a Label, but also values that
    arise from calling math::plus and math::times on this type, and zero and one
    in this semiring.

    This is often useful for return types.
    */
    template <class Label> struct GeneraliseSemiring {
        typedef typename GeneraliseToZero <Label>::type WithZero;
        typedef typename math::merge_magma::apply <Label, typename
            std::result_of <math::callable::one <Label>()>::type>::type WithOne;

        typedef typename math::generalise_type <
                meta::vector <math::callable::plus, math::callable::times>,
                Label>::type type;
    };

    /** \brief
    Evaluate to the type that should normally be used for the terminal
    label corresponding to \c Label.

    This is computed as the return type of <c>math::one \<Label>()</c>.
    */
    template <class Label> struct GetDefaultTerminalLabel
    : std::decay <typename std::result_of <math::callable::one <Label>()>::type>
    {};

    /** \brief
    Give the default type of the descriptor object that implements the
    compression of labels.

    Internally, labels can sometimes be represented more tersely, by splitting
    the label up into an compressed representation and a "descriptor".
    There needs to be only one descriptor object, but there can be many label
    objects.

    By default, math::sequence and friends are compressed to sequences of
    symbols from a math::alphabet, and the components of math::product and
    math::lexicographical are compressed recursively.

    Specialise this for other label types that need special treatment.
    */
    template <class Label, class Enable = void> struct DefaultDescriptorFor;

    /** \struct CompressedLabelType
    \brief Compute the compressed label type that the descriptor will convert a
    label type to.
    */
    template <class Descriptor, class Label> struct CompressedLabelType;

    /* Implementation. */

    /** \class NoDescriptor
    \brief Descriptor for any label that there is no other descriptor for.

    This does not perform any compression: the compressed representation is
    exactly equal to the external representation.
    */
    class NoDescriptor;

    /** \brief
    Descriptor that compresses math::sequence using an alphabet.

    The compressed representation uses dense representations for the external
    symbols.
    The descriptor keeps a std::shared_ptr to an alphabet.
    These alphabets can be shared.
    Two AlphabetDescriptor objects compare equal iff they share an alphabet.

    Note that the ordering of the compressed representation is different than
    that of the external representation.
    This sorting order is normally used merely as a tie-breaker of last resort.
    In practice, therefore, changing the ordering does not normally make results
    worse, but it may make them slightly different.
    */
    template <class Symbol> class AlphabetDescriptor;

    /** \brief
    Descriptor for composite labels.

    This recursively deals with math::product and math::lexicographical.
    \tparam Descriptors
        The sequence of descriptors for the components of the composite label.
    */
    template <class ... Descriptors> class CompositeDescriptor;

    /**
    Special symbol for in an alphabet so that an empty symbol can be kept more
    efficiently.
    This is currently not used; it requires support from math::optional_sequence
    (and friends, probably) which should be told (at compile time) what the
    dense symbol for emptiness is.
    */
    struct Empty;

    /// \cond DONT_DOCUMENT

    // Default implementation.
    template <class ExternalLabel> struct DefaultDescriptorFor <ExternalLabel>
    { typedef NoDescriptor type; };

    /**
    Find an alphabet that has \a Symbol as the external symbol type, but adds
    \a Empty as a special symbol.
    \todo Empty is not currently used for this.
    */
    template <class Symbol> struct ChooseAlphabet
    { typedef AlphabetDescriptor <Symbol> type; };

    // Sequences.
    template <class Symbol, class Direction>
        struct DefaultDescriptorFor <math::sequence <Symbol, Direction>>
    : ChooseAlphabet <Symbol> {};

    template <class Symbol, class Direction>
        struct DefaultDescriptorFor <math::empty_sequence <Symbol, Direction>>
    : ChooseAlphabet <Symbol> {};

    template <class Symbol, class Direction>
        struct DefaultDescriptorFor <math::single_sequence <Symbol, Direction>>
    : ChooseAlphabet <Symbol> {};

    template <class Symbol, class Direction>
        struct DefaultDescriptorFor <
            math::optional_sequence <Symbol, Direction>>
    : ChooseAlphabet <Symbol> {};

    template <class Symbol, class Direction>
        struct DefaultDescriptorFor <
            math::sequence_annihilator <Symbol, Direction>>
    : ChooseAlphabet <Symbol> {};

    /* Product: use CompositeDescriptor. */
    template <class ... Components, class Inverses>
        struct DefaultDescriptorFor <
            math::product <math::over <Components ...>, Inverses>>
    {
        typedef CompositeDescriptor <
            typename DefaultDescriptorFor <Components>::type ...> type;
    };

    /* Lexicographical: use CompositeDescriptor. */
    template <class ... Components>
        struct DefaultDescriptorFor <
            math::lexicographical <math::over <Components ...>>>
    {
        typedef CompositeDescriptor <
            typename DefaultDescriptorFor <Components>::type ...> type;
    };

    /// \endcond

    /* Descriptors. */

    namespace callable {

        struct Compress {
            template <class Descriptor, class ExternalLabel> inline
                auto operator() (
                    Descriptor const & descriptor, ExternalLabel const & l)
                const
            RETURNS (descriptor.compress() (l));
        };

        struct Expand {
            template <class Descriptor, class CompressedLabel> inline
                auto operator() (
                    Descriptor const & descriptor, CompressedLabel const & l)
                const
            RETURNS (descriptor.expand() (l));
        };

    } // namespace callable

    /**
    Convert a value to its compressed representation through the descriptor.
    \param descriptor The descriptor to use for the conversion.
    \param label The external label to convert.
    */
    static auto const compress = callable::Compress();

    /**
    Convert a value from its compressed representation to its external
    representation.
    \param descriptor The descriptor to use for the conversion.
    \param label The compressed label to convert.
    */
    static auto const expand = callable::Expand();

    /** \brief
    Find the compressed label type that \a Descriptor converts \a ExpandedLabel
    to.
    */
    template <class Descriptor, class ExpandedLabel> struct CompressedLabelType
    : std::decay <typename std::result_of <
        callable::Compress (Descriptor, ExpandedLabel)>::type> {};

    /** \brief
    Find the expanded label type that \a Descriptor converts \a CompressedLabel
    to.
    */
    template <class Descriptor, class CompressedLabel> struct ExpandedLabelType
    : std::decay <typename std::result_of <
        callable::Expand (Descriptor, CompressedLabel)>::type> {};

    class NoDescriptor {
        struct Identity {
            template <class Type> Type const & operator() (Type const & o) const
            { return o; }

            template <class Type> Type operator() (Type && o) const
            { return std::forward <Type> (o); }
        };

    public:
        Identity compress() const { return Identity(); }
        Identity expand() const { return Identity(); }

        rime::true_type operator== (NoDescriptor const & that) const
        { return rime::true_; }
    };

    template <class Symbol> class AlphabetDescriptor {
    public:
        typedef math::alphabet <Symbol> Alphabet;
        typedef typename Alphabet::dense_symbol_type DenseSymbol;

    private:
        std::shared_ptr <Alphabet> alphabet_;

        /// Closure that adds symbols to an alphabet.
        struct AddSymbol {
            Alphabet & alphabet_;

            AddSymbol (Alphabet & alphabet) : alphabet_(alphabet) {}

            DenseSymbol operator() (Symbol const & s) const
            { return alphabet_.add_symbol (s); }
        };

        struct ConvertToSymbol {
            Alphabet & alphabet_;

            ConvertToSymbol (Alphabet & alphabet) : alphabet_(alphabet) {}

            Symbol operator() (DenseSymbol const & s) const {
                // Should be normal symbol type.
                return alphabet_.template get_symbol <
                    typename Alphabet::normal_symbol_type> (s);
            }
        };

        template <class SourceSymbol, class TargetSymbol, class ConvertSymbol>
            struct ConvertSequence
        {
            ConvertSymbol convert_symbol;

            ConvertSequence (Alphabet & alphabet) : convert_symbol (alphabet) {}

            template <class Direction,
                class Result = math::sequence <TargetSymbol, Direction>>
                    Result operator() (math::sequence <
                        SourceSymbol, Direction> const & s) const
            {
                if (s.is_annihilator())
                    return math::zero <Result>();
                return Result (range::transform (s.symbols(), convert_symbol));
            }

            template <class Direction,
                class Result = math::single_sequence <TargetSymbol, Direction>>
            Result operator() (
                math::single_sequence <SourceSymbol, Direction> const & s) const
            { return Result (convert_symbol (s.symbol())); }

            template <class Direction, class Result
                = math::optional_sequence <TargetSymbol, Direction>>
            Result operator() (
                math::optional_sequence <SourceSymbol, Direction> const & s)
            const
            { return Result (range::transform (s.symbols(), convert_symbol)); }

            template <class Direction>
                math::empty_sequence <TargetSymbol, Direction> operator() (
                    math::empty_sequence <SourceSymbol, Direction>)
                const
            { return math::empty_sequence <TargetSymbol, Direction>(); }

            template <class Direction>
                math::sequence_annihilator <TargetSymbol, Direction>
                    operator() (
                        math::sequence_annihilator <SourceSymbol, Direction>)
                const
            { return math::sequence_annihilator <TargetSymbol, Direction>(); }
        };

        typedef ConvertSequence <Symbol, DenseSymbol, AddSymbol> Compress;
        typedef ConvertSequence <DenseSymbol, Symbol, ConvertToSymbol>
            Expand;

    public:
        /**
        Construct with a new alphabet that is unique to the descriptor.
        */
        AlphabetDescriptor() : alphabet_ (std::make_shared <Alphabet>()) {}

        /**
        Construct with a shared alphabet.
        Multiple descriptors can share alphabets.
        They must shared alphabets if they are to be matched for composition.
        */
        AlphabetDescriptor (std::shared_ptr <Alphabet> alphabet)
        : alphabet_ (alphabet) {}

        /**
        Return a pointer to the alphabet.
        */
        std::shared_ptr <Alphabet> const & alphabet() const
        { return alphabet_; }

        Compress compress() const { return Compress (*alphabet_); }
        Expand expand() const { return Expand (*alphabet_); }

        bool operator== (AlphabetDescriptor const & that) const
        { return alphabet_ == that.alphabet_; }
    };

    /* CompositeDescriptor. */
    template <class ... Descriptors> class CompositeDescriptor {
        typedef range::tuple <Descriptors ...> DescriptorTuple;
        DescriptorTuple descriptors_;

        /**
        Closure that converts composite labels by applying \a ConvertComponent
        to each component.
        */
        template <class ConvertComponent> class ConvertLabel {
            DescriptorTuple const & descriptors_;

        public:
            explicit ConvertLabel (DescriptorTuple const & descriptors)
            : descriptors_ (descriptors) {}

            // Convert math::product.
            template <class ... Components, class Inverses>
                auto operator() (math::product <math::over <Components ...>,
                    Inverses> const & p) const
            RETURNS (math::make_product_over <Inverses> (
                // Call ConvertComponent() with each descriptor and
                // corresponding component.
                range::transform (range::zip (descriptors_, p.components()),
                    range::call_unpack (ConvertComponent()))));

            // Convert math::lexicographical.
            template <class ... Components>
                auto operator() (math::lexicographical <
                    math::over <Components ...>> const & l) const
            RETURNS (math::make_lexicographical_over (
                range::transform (range::zip (descriptors_, l.components()),
                    range::call_unpack (ConvertComponent()))));
        };

        typedef ConvertLabel <callable::Compress> Compress;
        typedef ConvertLabel <callable::Expand> Expand;

    public:
        CompositeDescriptor() : descriptors_() {}

        template <class ... DescriptorArguments>
            CompositeDescriptor (DescriptorArguments && ... descriptorArguments)
        : descriptors_ (
            std::forward <DescriptorArguments> (descriptorArguments) ...) {}

        Compress compress() const { return Compress (descriptors_); }
        Expand expand() const { return Expand (descriptors_); }

        /** \brief
        Return a tuple with the contained descriptors.
        */
        DescriptorTuple const & components() const { return descriptors_; }

        bool operator== (CompositeDescriptor const & that) const
        { return range::equal (this->descriptors_, that.descriptors_); }
    };

} // namespace label

} // namespace flipsta

#endif // FLIPSTA_LABEL_HPP_INCLUDED
