#pragma once

#include <numeric>

// I call this a lazy rational because all arithmetic operations yield another lazy rational
// without reducing the fraction. This can result in blowup and integer overflow unless
// 1) reduce is called explicitly or 2) small numbers are guaranteed a priori. Why not reduce?
// Because it's FASTER to avoid division.
class LazyRational
{
public:
    int num;
    int denom;

    LazyRational() = default;

    constexpr LazyRational(int num, int denom = 1) : num(num), denom(denom) {}

    constexpr LazyRational operator+(const LazyRational& other) const {
        return {other.denom * num + denom * other.num, denom * other.denom};
    }

    constexpr LazyRational operator-(const LazyRational& other) const {
        return {other.denom * num - denom * other.num, denom * other.denom};
    }

    constexpr LazyRational operator*(const LazyRational& other) const {
        return {num * other.num, denom * other.denom};
    }

    constexpr LazyRational operator/(const LazyRational& other) const {
        return {num * other.denom, denom * other.num};
    }

    constexpr LazyRational operator==(const LazyRational& other) const {
        if (num == other.num && denom == other.denom)
            return true;
        return (other.denom * num) == (denom * other.num);
    }

    constexpr void reduce() {
        auto gcd = std::gcd(num, denom);
        num /= gcd;
        denom /= gcd;
    }
};