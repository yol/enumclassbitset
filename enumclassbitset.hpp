/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Philipp Kerling <pkerling@casix.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <bitset>
#include <type_traits>

namespace Util {

    /**
     * Provide information about a custom enum class type
     *
     * Needed only by \ref EnumClassBitset.
     *
     * Has to have exactly one static const element named \c max of type T that
     * equals the last possible value of the enumeration type.
     *
     * \tparam T enum class type to describe
     */
    template<typename T>
    struct EnumTraits;

    /**
     * std::bitset for enum class types
     *
     * Provides a type-safe interface to std::bitset when used with enum class types.
     * \ref EnumTraits must be specialized for the enum type.
     * This also provides an iterator interface for easy enumeration of all members
     * in the set with range-based for loops.
     *
     * Most operations from std::bitset are provided except the shift operators
     * which should not be particularly useful on enum class sets.
     * The boolean "value" of an enumeration member in the context of this class means
     * whether it is part of the set.
     *
     * Inspired by http://stackoverflow.com/questions/17350214/using-enum-class-with-stdbitset
     *
     * Example:
     * \code{.cpp}
     * enum class ConntrackState {
     *     NEW,
     *     ESTABLISHED,
     *     RELATED
     * };
     *
     * template<>
     * struct EnumTraits<ConntrackState> {
     *     static constexpr ConntrackState max = ConntrackState::RELATED
     * }
     *
     * typedef EnumClassBitset<ConntrackState> ConntrackStateSet;
     *
     * void example() {
     *     ConntrackStateSet states;
     *     states.set(ConntrackState::NEW);
     *     states.set(ConntrackState::ESTABLISHED);
     *     bool result = states.test(ConntrackState::NEW); // true
     *     for (auto state : states) {
     *         switch (state) {
     *             case ConntrackState::NEW:
     *                 std::cout << "NEW is in the set" << std::endl;         // will be output
     *                 break;
     *             case ConntrackState::ESTABLISHED:
     *                 std::cout << "ESTABLISHED is in the set" << std::endl; // will be output
     *                 break;
     *             case ConntrackState::RELATED:
     *                 std::cout << "RELATED is in the set" << std::endl;     // will not be output
     *                 break;
     *         }
     *     }
     * }
     * \endcode
     *
     * \tparam T enum class type
     */
    template<typename T>
    class EnumClassBitset
    {
    public:
        /// Type of the bitset implementation
        typedef std::bitset<static_cast<std::size_t>(EnumTraits<T>::max)> ImplType;

    private:
        ImplType mBitset;

        /**
         * Cast the enum class type away to use it as an offset into the bitset
         * \param member enumeration member to convert
         * \return member casted to std::size_t
         */
        inline std::size_t enumIndex(T member) const
        {
            return static_cast<std::size_t>(member);
        }

    public:
        /**
         * Readonly iterator for elements in the set
         */
        struct iterator : std::iterator<std::forward_iterator_tag, T>
        {
        private:
            /// Enum set the iterator belongs to
            EnumClassBitset const & bitset;
            /// Index into the set of the current value
            std::size_t index;
            /// Index cast to T
            T value;

            /**
             * Private constructor for begin() and end()
             *
             * This constructor can be used to generate an invalid iterator when
             * \c argIndex is set to an element that is not inside the set.
             * To find the first correct element, use \ref findNext.
             *
             * \param argBitset enum set the iterator belongs to
             * \param argIndex index into the set of the current value
             */
            iterator(EnumClassBitset const & argBitset, std::size_t argIndex)
            : bitset(argBitset), index(argIndex)
            {
                value = static_cast<T> (index);
            }

            /**
             * Starting with the current index, find the first index
             * that corresponds to an element actually in the set
             *
             * If none is found, \c index is bitset.size(), which should be equivalent
             * to bitset.end() as iterator.
             */
            void findNext()
            {
                while (index < bitset.size()) {
                    value = static_cast<T> (index);
                    if (bitset.test(value)) {
                        break;
                    }
                    index++;
                };
            }

            friend class EnumClassBitset;

        public:
            /**
             * Get the enumeration member this iterator represents
             */
            T& operator*()
            {
                return value;
            }
            /**
             * Advance the iterator
             * \return iterator for the next enumeration member in the set, or
             *         a past-the-end iterator
             */
            iterator& operator++()
            {
                index++;
                findNext();
                return *this;
            }
            /**
             * Compare with another iterator for equality
             * \param other other iterator
             * \return \c true if the iterators point to the same element in the same set, \c false otherwise
             */
            bool operator==(iterator const & other) const
            {
                return (std::addressof(bitset) == std::addressof(other.bitset)) && (index == other.index);
            }
            /**
             * Compare with another iterator for inequality
             * \param other other iterator
             * \return \c false if the iterators point to the same element in the same set, \c true otherwise
             */
            bool operator!=(iterator const & other) const
            {
                return !((*this) == other);
            }
        };

        typedef typename ImplType::reference reference;

        /**
         * Default constructor
         *
         * All enumeration members are considered to be not in the set.
         */
        EnumClassBitset()
        {
        }

        /**
         * Compare with another enumeration set for equality
         * \param other set to compare
         * \return \c true if the members of both sets are equivalent, \c false otherwise
         */
        bool operator==(const EnumClassBitset& other) const
        {
            return mBitset == other.mBitset;
        }
        /**
         * Compare with another enumeration set for inequality
         * \param other set to compare
         * \return \c false if the members of both sets are equivalent, \c true otherwise
         */
        bool operator!=(const EnumClassBitset& other) const
        {
            return mBitset != other.mBitset;
        }

        /**
         * Access the enumeration member \c member
         *
         * Unlike \ref test, does not throw exceptions: the behaviour is undefined
         * if \c member is out of bounds.
         *
         * \param member member to access the value of
         * \return \c true if the \c member is in the set, \c false otherwise
         */
        constexpr bool operator[](T member) const
        {
            return mBitset[enumIndex(member)];
        }

        /**
         * Access the enumeration member \c member
         *
         * Unlike \ref test, does not throw exceptions: the behaviour is undefined
         * if \c member is out of bounds.
         *
         * \param member member to access the value of
         * \return an object of type reference, which allows including/excluding
         *         the member from the set by writing to it
         */
        reference operator[](T member)
        {
            return mBitset[enumIndex(member)];
        }

        /**
         * Return the value of the enumeration member \c member
         *
         * \throws std::out_of_range if \c member is not a valid enumeration member
         *
         * \param member member to return the value of
         * \return \c true if the requested enumeration member is in the set, \c false otherwise
         */
        bool test(T member) const
        {
            return mBitset.test(enumIndex(member));
        }

        /**
         * Check if all possible enumeration members are part of the set
         */
        bool all() const
        {
            return mBitset.all();
        }

        /**
         * Check if any of the possible enumeration members are part of the set
         */
        bool any() const
        {
            return mBitset.any();
        }

        /**
         * Check if none of the possible enumeration members are part of the set
         */
        bool none() const
        {
            return mBitset.none();
        }

        /**
         * Get the number of enumeration members that are part of the set
         */
        std::size_t count() const
        {
            return mBitset.count();
        }

        /**
         * Returns the number of enumeration members that the set can hold
         *
         * \return number of members the set can hold, i.e. EnumTraits<T>::max
         */
        constexpr std::size_t size() const
        {
            return mBitset.size();
        }

        /**
         * Set the value of the enumeration member \c member to \c false
         * \param member member to set the value of to \c false
         * \return \c *this
         */
        EnumClassBitset& reset(T member)
        {
            mBitset.reset(enumIndex(member));
            return *this;
        }

        /**
         * Flip the values of all enumeration members, i.e. get the inverse set
         * \return \c *this
         */
        EnumClassBitset& flip()
        {
            mBitset.flip();
            return *this;
        }

        /**
         * Flip the value of the enumeration member \c member
         * \param member member to flip the value of
         * \return \c *this
         */
        EnumClassBitset& flip(T member)
        {
            mBitset.flip(enumIndex(member));
            return *this;
        }

        /**
         * Set the value of the enumeration member \c member to value
         * \param member member to set the value of
         * \param value value to set the enumeration member to
         * \return \c *this
         */
        EnumClassBitset& set(T member, bool value = true)
        {
            mBitset.set(enumIndex(member), value);
            return *this;
        }

        /**
         * Intersect this set with another set
         * \param other another set
         * \return \c *this
         */
        EnumClassBitset& operator&=(EnumClassBitset const & other)
        {
            mBitset &= other.mBitset;
            return *this;
        }

        /**
         * Combine this set with another set
         * \param other another set
         * \return \c *this
         */
        EnumClassBitset& operator|=(EnumClassBitset const & other)
        {
            mBitset |= other.mBitset;
            return *this;
        }

        /**
         * Combine this set with another set using an exclusive-OR relation
         * \param other another set
         * \return \c *this
         */
        EnumClassBitset& operator^=(EnumClassBitset const & other)
        {
            mBitset ^= other.mBitset;
            return *this;
        }

        /**
         * Build the intersection with another set
         * \param other another set
         * \return set with all elements that are set in both this set and \c other
         */
        EnumClassBitset operator&(EnumClassBitset const & other) const
        {
            EnumClassBitset newBitset(*this);
            newBitset.mBitset &= other.mBitset;
            return newBitset;
        }

        /**
         * Build the union set with another set
         * \param other another set
         * \return set with all elements that are set in this set or \c other
         */
        EnumClassBitset operator|(EnumClassBitset const & other) const
        {
            EnumClassBitset newBitset(*this);
            newBitset.mBitset |= other.mBitset;
            return newBitset;
        }

        /**
         * Build the union set with another set
         * \param other another set
         * \return set with all elements that are set in either this set or \c other
         */
        EnumClassBitset operator^(EnumClassBitset const & other) const
        {
            EnumClassBitset newBitset(*this);
            newBitset.mBitset ^= other.mBitset;
            return newBitset;
        }

        /**
         * Get a set with the values of all enumeration members, i.e. the inverse set
         * \sa flip
         * \return set which contains all enumeration members that are not contained
         *         in this set
         */
        EnumClassBitset operator~() const
        {
            EnumClassBitset newBitset(*this);
            newBitset.flip();
            return newBitset;
        }

        /**
         * Constant iterator to the first enumeration member in the set
         */
        iterator begin() const
        {
            auto iter = iterator(*this, 0);
            // Advance to first element in set
            iter.findNext();
            return iter;
        }

        /**
         * Constant iterator past the last enumeration member in the set
         */
        iterator end() const
        {
            return iterator(*this, static_cast<std::size_t>(EnumTraits<T>::max));
        }

        /**
         * Convert the enumeration member set to a raw \c std::bitset
         *
         * \return \c std::bitset instance with bit positions corresponding to
         *         enumeration members in this set set to \c true
         */
        ImplType to_raw_bitset() const
        {
            return mBitset;
        }

    };

}
