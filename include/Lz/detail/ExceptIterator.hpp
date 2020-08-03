#pragma once

#include <type_traits>
#include <algorithm>
#include <functional>


namespace lz {
    template<class, class>
    class Except;

    namespace detail {
        template<class Iterator, class IteratorToExcept>
        struct ExceptIteratorHelper {
            IteratorToExcept toExceptBegin{};
            IteratorToExcept toExceptEnd{};
        };

        template<class Iterator, class IteratorToExcept>
        class ExceptIterator {
            using IterTraits = std::iterator_traits<Iterator>;
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = typename IterTraits::value_type;
            using difference_type = typename IterTraits::difference_type;
            using pointer = typename IterTraits::pointer;
            using reference = typename IterTraits::reference;

        private:
            Iterator _iterator{};
            Iterator _end{};
            const ExceptIteratorHelper<Iterator, IteratorToExcept>* _iteratorHelper;

            friend class Except<Iterator, IteratorToExcept>;

            void find() {
                IteratorToExcept it = std::find(_iteratorHelper->toExceptBegin, _iteratorHelper->toExceptEnd,
                                                *_iterator);

                while (it != _iteratorHelper->toExceptEnd) {
                    ++_iterator;
                    if (_iterator == _end) {
                        return;
                    }
                    it = std::find(_iteratorHelper->toExceptBegin, _iteratorHelper->toExceptEnd,
                                   *_iterator);
                }
            }

        public:
            // gcc 5.4.0 crashes with inline declaration
            ExceptIterator() :
                _iteratorHelper(ExceptIteratorHelper<Iterator, IteratorToExcept>()) {
            }

            explicit ExceptIterator(const Iterator begin, const Iterator end,
                                    const ExceptIteratorHelper<Iterator, IteratorToExcept>* iteratorHelper) :
                _iterator(begin),
                _end(end),
                _iteratorHelper(iteratorHelper) {
            }

            reference operator*() const {
                return *_iterator;
            }

            pointer operator->() const {
                return _iterator.operator->();
            }

            ExceptIterator& operator++() {
                ++_iterator;
                if (_iterator != _end) {
                    find();
                }
                return *this;
            }

            ExceptIterator operator++(int) {
                ExceptIterator tmp(*this);
                ++*this;
                return tmp;
            }

            bool operator!=(const ExceptIterator& other) const {
                return _iterator != other._end;
            }

            bool operator==(const ExceptIterator& other) const {
                return !(*this != other);
            }
        };
    }
}