#pragma once

#ifndef LZ_MAP_ITERATOR_HPP
#define LZ_MAP_ITERATOR_HPP


#include "LzTools.hpp"
#include "FunctionContainer.hpp"


namespace lz { namespace internal {
    template<LZ_CONCEPT_ITERATOR Iterator, class Function>
    class MapIterator {
        Iterator _iterator{};
        mutable FunctionContainer<Function> _function{};

        using IterTraits = std::iterator_traits<Iterator>;

    public:
		using reference = decltype(_function(*_iterator));
        using value_type = Decay<reference>;
        using iterator_category = typename IterTraits::iterator_category;
        using difference_type = typename IterTraits::difference_type;
        using pointer = FakePointerProxy<reference>;

        MapIterator(Iterator iterator, Function function) :
            _iterator(std::move(iterator)),
            _function(std::move(function)) {
        }

        MapIterator() = default;

        reference operator*() const {
            return _function(*_iterator);
        }

        pointer operator->() const {
            return FakePointerProxy<decltype(**this)>(**this);
        }

        MapIterator& operator++() {
            ++_iterator;
            return *this;
        }

        MapIterator operator++(int) {
            MapIterator tmp(*this);
            ++*this;
            return tmp;
        }

        MapIterator& operator--() {
            --_iterator;
            return *this;
        }

        MapIterator operator--(int) {
            MapIterator tmp(*this);
            --*this;
            return tmp;
        }

        MapIterator& operator+=(const difference_type offset) {
            _iterator += offset;
            return *this;
        }

        MapIterator& operator-=(const difference_type offset) {
            _iterator -= offset;
            return *this;
        }

        MapIterator operator+(const difference_type offset) const {
            MapIterator tmp(*this);
            tmp += offset;
            return tmp;
        }

        MapIterator operator-(const difference_type offset) const {
            MapIterator tmp(*this);
            tmp -= offset;
            return tmp;
        }

        difference_type operator-(const MapIterator& other) const {
            return _iterator - other._iterator;
        }

        reference operator[](const difference_type offset) const {
            return *(*this + offset);
        }

        friend bool operator==(const MapIterator& a, const MapIterator& b) {
            return !(a != b); // NOLINT
        }

        friend bool operator!=(const MapIterator& a, const MapIterator& b) {
            return a._iterator != b._iterator;
        }

        friend bool operator<(const MapIterator& a, const MapIterator& b) {
            return a._iterator < b._iterator;
        }

        friend bool operator>(const MapIterator& a, const MapIterator& b) {
            return b < a;
        }

        friend bool operator<=(const MapIterator& a, const MapIterator& b) {
            return !(b < a); // NOLINT
        }

        friend bool operator>=(const MapIterator& a, const MapIterator& b) {
            return !(a < b); // NOLINT
        }
    };
}} // end lz::internal

#endif