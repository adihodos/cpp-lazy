#pragma once

#ifndef LZ_JOIN_WHERE_ITERATOR_HPP
#define LZ_JOIN_WHERE_ITERATOR_HPP

#include "Lz/IterBase.hpp"
#include "Lz/detail/CompilerChecks.hpp"
#include "Lz/detail/FakePointerProxy.hpp"
#include "Lz/detail/FunctionContainer.hpp"

namespace lz {
namespace detail {
#ifdef LZ_HAS_EXECUTION
template<class IterA, class IterB, class SelectorA, class SelectorB, class ResultSelector, class Execution>
class JoinWhereIterator
    : public IterBase<JoinWhereIterator<IterA, IterB, SelectorA, SelectorB, ResultSelector, Execution>,
                      IteratorFnRetT<ResultSelector, IterA, IterB>,
                      FakePointerProxy<IteratorFnRetT<ResultSelector, IterA, IterB>>, std::ptrdiff_t, std::forward_iterator_tag> {
#else
template<class IterA, class IterB, class SelectorA, class SelectorB, class ResultSelector>
class JoinWhereIterator
    : public IterBase<JoinWhereIterator<IterA, IterB, SelectorA, SelectorB, ResultSelector>,
                      IteratorFnRetT<ResultSelector, IterA, IterB>,
                      FakePointerProxy<IteratorFnRetT<ResultSelector, IterA, IterB>>, std::ptrdiff_t, std::forward_iterator_tag> {
#endif // LZ_HAS_EXECUTION
private:
    using IterTraitsA = std::iterator_traits<IterA>;
    using IterTraitsB = std::iterator_traits<IterB>;
    using ValueTypeB = typename IterTraitsB::value_type;
    using RefTypeA = typename IterTraitsA::reference;

    using SelectorARetVal = Decay<FunctionReturnType<SelectorA, RefTypeA>>;

    IterA _iterA{};
    IterA _endA{};
    IterB _iterB{};
    IterB _beginB{};
    IterB _endB{};
#ifdef LZ_HAS_EXECUTION
    LZ_NO_UNIQUE_ADDRESS
    Execution _exec{};
#endif // LZ_HAS_EXECUTION
    mutable FunctionContainer<SelectorA> _selectorA{};
    mutable FunctionContainer<SelectorB> _selectorB{};
    mutable FunctionContainer<ResultSelector> _resultSelector{};

    void findNext() {
#ifdef LZ_HAS_EXECUTION
        if constexpr (isCompatibleForExecution<Execution, IterA>()) {
            _iterA = std::find_if(_iterA, _endA, [this](const ValueType<IterA>& a) {
                auto&& toFind = _selectorA(a);
                _iterB =
                    std::lower_bound(std::move(_iterB), _endB, toFind,
                                     [this](const ValueTypeB& b, const SelectorARetVal& val) { return _selectorB(b) < val; });
                if (_iterB != _endB && !(toFind < _selectorB(*_iterB))) { // NOLINT
                    return true;
                }
                _iterB = _beginB;
                return false;
            });
        }
        else {
            std::mutex mutex;
            std::lock_guard guard(mutex);
            _iterA = std::find_if(_exec, _iterA, _endA, [this, &mutex](const ValueType<IterA>& a) {
                auto&& toFind = _selectorA(a);
                _iterB =
                    std::lower_bound(std::move(_iterB), _endB, toFind,
                                     [this](const ValueTypeB& b, const SelectorARetVal& val) { return _selectorB(b) < val; });
                if (_iterB != _endB && !(toFind < _selectorB(*_iterB))) { // NOLINT
                    return true;
                }
                _iterB = _beginB;
                return false;
            });
        }
#else
        _iterA = std::find_if(_iterA, _endA, [this](const ValueType<IterA>& a) {
            auto&& toFind = _selectorA(a);
            _iterB = std::lower_bound(std::move(_iterB), _endB, toFind,
                                      [this](const ValueTypeB& b, const SelectorARetVal& val) { return _selectorB(b) < val; });
            if (_iterB != _endB && !(toFind < _selectorB(*_iterB))) { // NOLINT
                return true;
            }
            _iterB = _beginB;
            return false;
        });
#endif // LZ_HAS_EXECUTION
    }

public:
    using reference = decltype(_resultSelector(*_iterA, *_iterB));
    using value_type = Decay<reference>;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = FakePointerProxy<reference>;

#ifdef LZ_HAS_EXECUTION
    LZ_CONSTEXPR_CXX_20 JoinWhereIterator(IterA iterA, IterA endA, IterB iterB, IterB endB, SelectorA a, SelectorB b,
                                          ResultSelector resultSelector, Execution execution)
#else
    LZ_CONSTEXPR_CXX_20
    JoinWhereIterator(IterA iterA, IterA endA, IterB iterB, IterB endB, SelectorA a, SelectorB b, ResultSelector resultSelector)
#endif // LZ_HAS_EXECUTION
        :
        _iterA(std::move(iterA)),
        _endA(std::move(endA)),
        _iterB(iterB),
        _beginB(iterB == endB ? endB : std::move(iterB)),
        _endB(std::move(endB)),
#ifdef LZ_HAS_EXECUTION
        _exec(execution),
#endif // LZ_HAS_EXECUTION
        _selectorA(std::move(a)),
        _selectorB(std::move(b)),
        _resultSelector(std::move(resultSelector)) {
        if (_iterA == _endA || _iterB == _endB) {
            return;
        }
        findNext();
    }

    constexpr JoinWhereIterator() = default;

    LZ_NODISCARD LZ_CONSTEXPR_CXX_20 reference dereference() const {
        return _resultSelector(*_iterA, *_iterB);
    }

    LZ_NODISCARD LZ_CONSTEXPR_CXX_20 pointer arrow() const {
        return FakePointerProxy<decltype(**this)>(**this);
    }

    LZ_CONSTEXPR_CXX_20 void increment() {
        ++_iterB;
        findNext();
    }

    LZ_NODISCARD LZ_CONSTEXPR_CXX_20 bool eq(const JoinWhereIterator& b) const noexcept {
        return _iterA == b._iterA;
    }
};

} // namespace detail
} // namespace lz
#endif // LZ_JOIN_WHERE_ITERATOR_HPP
