#pragma once

#include <cstddef>
#include <array>
#include <span>

// Contains M indices of numbers in the range [0, N) to be used to iterate through permutations
template<size_t N, size_t M>
struct MultiInd {
    std::array<size_t, M> inds = {}; // default initialized to 0

    MultiInd() = default;

    template<typename... T>
    constexpr explicit MultiInd(T... Inds) : inds{Inds...} {}

    constexpr MultiInd& operator++() {
        for (size_t i = 0; i < M; ++i) {
            if (++inds[i] >= N && i < M - 1)
                inds[i] = 0;
            else
                break;
        }

        return *this;
    }

    constexpr bool operator==(const MultiInd& other) const {
        return inds == other.inds;
    }

    [[nodiscard]] constexpr bool valid() const {
        return inds[M - 1] < N;
    }

    template<typename T, size_t P = std::dynamic_extent, typename To = std::remove_cvref_t<T>>
    auto extract(std::span<T, P> vals) {
        std::array<To, M> res;
        for (size_t i = 0; i < M; ++i)
            res[i] = vals[inds[i]];
        return res;
    }
};

// Contains M ordered indices of numbers in the range [0, N) to be used to iterate through combinations
template<size_t N, size_t M>
struct MultiIndCombo {
    std::array<size_t, M> inds;

    constexpr MultiIndCombo() {
        size_t i = M - 1;
        for (auto& ind : inds)
            ind = i--;
    }

    constexpr MultiIndCombo& operator++() {
        for (size_t i = 0; i < M; ++i) {
            if (++inds[i] < N - i) {
                for (size_t j = i; j > 0; --j)
                    inds[j - 1] = inds[j] + 1;
                break;
            }
        }

        return *this;
    }

    constexpr bool operator==(const MultiIndCombo& other) const {
        return inds == other.inds;
    }

    [[nodiscard]] constexpr bool valid() const {
        return inds[M - 1] < N - (M - 1);
    }

    template<typename T, size_t P = std::dynamic_extent, typename To = std::remove_cvref_t<T>>
    auto extract(std::span<T, P> vals) {
        std::array<To, M> res;
        for (size_t i = 0; i < M; ++i)
            res[i] = vals[inds[M - i - 1]];
        return res;
    }
};