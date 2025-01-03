#pragma once

#ifndef LZ_CARTESIAN_PRODUCT_ITERATOR_HPP
#define LZ_CARTESIAN_PRODUCT_ITERATOR_HPP

#include "Lz/IterBase.hpp"
#include "Lz/detail/FakePointerProxy.hpp"
#include "Lz/detail/Traits.hpp"

#include <numeric>

namespace lz {
namespace detail {
// Edited version of https://github.com/mirandaconrado/product-iterator
template<class... Iterators>
class CartesianProductIterator : public IterBase<CartesianProductIterator<Iterators...>, std::tuple<RefType<Iterators>...>,
                                                 FakePointerProxy<std::tuple<RefType<Iterators>...>>,
                                                 CommonType<DiffType<Iterators>...>, CommonType<IterCat<Iterators>...>> {
    static_assert(sizeof...(Iterators) > 1, "The size of the iterators must be greater than 1");

public:
    using value_type = std::tuple<ValueType<Iterators>...>;
    using reference = std::tuple<RefType<Iterators>...>;
    using pointer = FakePointerProxy<reference>;
    using iterator_category = CommonType<IterCat<Iterators>...>;
    using difference_type = CommonType<DiffType<Iterators>...>;

private:
    std::tuple<Iterators...> _begin{};
    std::tuple<Iterators...> _iterator{};
    std::tuple<Iterators...> _end{};

#ifndef __cpp_if_constexpr
    template<std::size_t I>
    LZ_CONSTEXPR_CXX_14 EnableIf<I == 0, void> next() const noexcept {
    }

    template<std::size_t I>
    LZ_CONSTEXPR_CXX_14 EnableIf<I == 0, void> previous() const noexcept {
    }

#ifdef LZ_MSVC
#pragma warning(push)
#pragma warning(disable : 4127)
#endif // LZ_MSVC

    template<std::size_t I>
    LZ_CONSTEXPR_CXX_20 EnableIf<(I > 0), void> next() {
        auto& prev = std::get<I - 1>(_iterator);
        ++prev;

        if (prev == std::get<I - 1>(_end)) {
            if (I != 1) {
                prev = std::get<I - 1>(_begin);
                next<I - 1>();
            }
        }
    }

    template<std::size_t I>
    LZ_CONSTEXPR_CXX_20 void doPrev() {
        --std::get<I>(_iterator);
    }

    template<std::size_t I>
    LZ_CONSTEXPR_CXX_20 EnableIf<I == 0> doPrevAll() {
        doPrev<0>();
    }

    template<std::size_t I>
    LZ_CONSTEXPR_CXX_20 EnableIf<(I > 0)> doPrevAll() {
        doPrev<I>();
        doPrevAll<I - 1>();
    }

    template<std::size_t I>
    LZ_CONSTEXPR_CXX_20 EnableIf<(I > 0)> previous() {
        if (_iterator == _end) {
            doPrevAll<I - 1>();
        }
        else {
            auto& prev = std::get<I - 1>(_iterator);
            if (prev == std::get<I - 1>(_begin)) {
                if (I != 1) {
                    prev = std::get<I - 1>(_end);
                    doPrev<I - 1>();
                    previous<I - 1>();
                }
            }
            else {
                doPrev<I - 1>();
            }
        }
    }

#ifdef LZ_MSVC
#pragma warning(pop)
#endif // LZ_MSVC

    template<std::size_t I>
    LZ_CONSTEXPR_CXX_20 EnableIf<I == 0> operatorPlusImpl(const difference_type offset) {
        auto& iterator = std::get<0>(_iterator);
        iterator = std::next(std::move(iterator), offset);
    }

    template<std::size_t I>
    EnableIf<(I > 0)> operatorPlusImpl(const difference_type offset) {
        auto& iterator = std::get<I>(_iterator);
        const auto& begin = std::get<I>(_begin);
        const auto& end = std::get<I>(_end);
        difference_type dist;
        if (offset < 0) {
            if (iterator == begin) {
                iterator = end;
                dist = iterator - begin;
            }
            else {
                dist = iterator - begin + 1;
            }
        }
        else {
            dist = end - iterator;
        }
        const auto offsets = std::lldiv(offset, dist);
        iterator += static_cast<difference_type>(offsets.rem);
        operatorPlusImpl<I - 1>(static_cast<difference_type>(offsets.quot));
    }

#else
    template<std::size_t I>
    LZ_CONSTEXPR_CXX_20 void next() {
        if constexpr (I == 0) {
            return;
        }
        else {
            auto& prev = std::get<I - 1>(_iterator);
            ++prev;

            if (prev == std::get<I - 1>(_end)) {
                if constexpr (I != 1) {
                    prev = std::get<I - 1>(_begin);
                    next<I - 1>();
                }
            }
        }
    }

    template<std::size_t I>
    void operatorPlusImpl(const difference_type offset) {
        auto& iterator = std::get<I>(_iterator);
        if constexpr (I == 0) {
            iterator = iterator + offset;
        }
        else {
            const auto& begin = std::get<I>(_begin);
            const auto& end = std::get<I>(_end);
            difference_type dist;
            if (offset < 0) {
                if (iterator == begin) {
                    iterator = end;
                    dist = iterator - begin;
                }
                else {
                    dist = iterator - begin + 1;
                }
            }
            else {
                dist = end - iterator;
            }
            const auto [quot, rem] = std::lldiv(offset, dist);
            iterator += static_cast<difference_type>(rem);
            operatorPlusImpl<I - 1>(static_cast<difference_type>(quot));
        }
    }

    template<std::size_t I>
    LZ_CONSTEXPR_CXX_20 void doPrev() {
        --std::get<I>(_iterator);
    }

    template<std::size_t I>
    LZ_CONSTEXPR_CXX_20 void doPrevAll() {
        doPrev<I>();
        if constexpr (I > 0) {
            doPrevAll<I - 1>();
        }
    }

    template<std::size_t I>
    LZ_CONSTEXPR_CXX_20 void previous() {
        if constexpr (I == 0) {
            return;
        }
        if (_iterator == _end) {
            doPrevAll<I - 1>();
        }
        else {
            auto& prev = std::get<I - 1>(_iterator);
            if (prev == std::get<I - 1>(_begin)) {
                if constexpr (I != 1) {
                    prev = std::get<I - 1>(_end);
                    doPrev<I - 1>();
                    previous<I - 1>();
                }
            }
            else {
                doPrev<I - 1>();
            }
        }
    }

#endif // __cpp_if_constexpr

    template<std::size_t... Is>
    LZ_CONSTEXPR_CXX_20 reference dereference(IndexSequence<Is...>) const {
        return reference{ *std::get<Is>(_iterator)... };
    }

    template<std::size_t... Is>
    LZ_CONSTEXPR_CXX_20 difference_type distanceImpl(IndexSequence<Is...>, const CartesianProductIterator& c) const {
        const difference_type distances[] = { static_cast<difference_type>(std::get<Is>(c._iterator) -
                                                                           std::get<Is>(_iterator))... };
        return std::accumulate(std::begin(distances), std::end(distances), difference_type{ 1 },
                               std::multiplies<difference_type>{});
    }

    using IndexSequenceForThis = MakeIndexSequence<sizeof...(Iterators)>;

    void checkEnd() {
        if (std::get<0>(_iterator) == std::get<0>(_end)) {
            _iterator = _end;
        }
    }

public:
    constexpr CartesianProductIterator() = default;

    LZ_CONSTEXPR_CXX_20
    CartesianProductIterator(std::tuple<Iterators...> iterator, std::tuple<Iterators...> begin, std::tuple<Iterators...> end) :
        _begin(std::move(begin)),
        _iterator(std::move(iterator)),
        _end(std::move(end)) {
    }

    LZ_NODISCARD LZ_CONSTEXPR_CXX_20 reference dereference() const {
        return dereference(IndexSequenceForThis());
    }

    LZ_NODISCARD LZ_CONSTEXPR_CXX_20 pointer arrow() const {
        return FakePointerProxy<decltype(**this)>(**this);
    }

    void increment() {
        next<sizeof...(Iterators)>();
        checkEnd();
    }

    void decrement() {
        previous<sizeof...(Iterators)>();
    }

    void plusIs(const difference_type n) {
        operatorPlusImpl<sizeof...(Iterators) - 1>(n);
        checkEnd();
    }

    LZ_NODISCARD LZ_CONSTEXPR_CXX_20 bool eq(const CartesianProductIterator& other) const {
        return _iterator == other._iterator;
    }

    LZ_NODISCARD LZ_CONSTEXPR_CXX_20 difference_type difference(const CartesianProductIterator& other) const {
        return other.distanceImpl(IndexSequenceForThis(), *this);
    }
};
} // namespace detail
} // namespace lz

#endif // LZ_CARTESIAN_PRODUCT_ITERATOR_HPP
