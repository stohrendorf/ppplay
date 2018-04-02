#pragma once

#include <cstdint>
#include <iostream>

struct FixedPoint final
{
    static constexpr const int64_t Scale = 1 << 16;
    int64_t value;

    struct RawTag
    {
    };

    explicit constexpr FixedPoint(int32_t i) noexcept
        : value{i * Scale}
    {}

    explicit constexpr FixedPoint(int64_t v, const RawTag&) noexcept
        : value{v}
    {}

    constexpr FixedPoint(const FixedPoint& rhs) noexcept
        : value{rhs.value}
    {}

    constexpr static FixedPoint fromFraction(int32_t nominator, int32_t denominator)
    {
        return FixedPoint{nominator * Scale / denominator, RawTag{}};
    }

    constexpr int32_t trunc() const noexcept
    {
        return static_cast<int32_t>(value / Scale);
    }

    constexpr FixedPoint truncated() const noexcept
    {
        return FixedPoint{static_cast<int32_t>(value / Scale)};
    }

    constexpr FixedPoint onlyFraction() const noexcept
    {
        return FixedPoint{value % Scale, RawTag{}};
    }

    constexpr int32_t frac() const noexcept
    {
        return static_cast<int32_t>(value % Scale);
    }

    constexpr float floatFrac() const noexcept
    {
        return static_cast<int32_t>(value % Scale);
    }

    constexpr float asFloat() const noexcept
    {
        return float(trunc()) + float(frac()) / Scale;
    }

    constexpr FixedPoint& operator=(const FixedPoint& rhs) noexcept = default;

    constexpr FixedPoint& operator=(int32_t i) noexcept
    {
        value = i * Scale;
        return *this;
    }

    constexpr FixedPoint& operator++() noexcept
    {
        value += Scale;
        return *this;
    }

    constexpr FixedPoint operator+(int32_t i) const noexcept
    {
        return FixedPoint{value + i * Scale, RawTag{}};
    }

    constexpr FixedPoint operator+(const FixedPoint& rhs) const noexcept
    {
        return FixedPoint{value + rhs.value, RawTag{}};
    }

    constexpr FixedPoint operator-(int32_t i) const noexcept
    {
        return FixedPoint{value - i * Scale, RawTag{}};
    }

    constexpr FixedPoint operator-(const FixedPoint& rhs) const noexcept
    {
        return FixedPoint{value - rhs.value, RawTag{}};
    }

    constexpr FixedPoint operator-() const noexcept
    {
        return FixedPoint{-value, RawTag{}};
    }

    constexpr FixedPoint operator/(const FixedPoint& rhs) const noexcept
    {
        return FixedPoint{value * Scale / rhs.value, RawTag{}};
    }

    constexpr FixedPoint operator/(int32_t i) const noexcept
    {
        return FixedPoint{value / i, RawTag{}};
    }

    constexpr FixedPoint operator*(int32_t i) const noexcept
    {
        return FixedPoint{value * i, RawTag{}};
    }

    constexpr FixedPoint operator%(int32_t i) const noexcept
    {
        return FixedPoint{value % (i * Scale), RawTag{}};
    }

    constexpr FixedPoint& operator+=(const FixedPoint& rhs) noexcept
    {
        value += rhs.value;
        return *this;
    }

    constexpr FixedPoint& operator-=(const FixedPoint& rhs) noexcept
    {
        value -= rhs.value;
        return *this;
    }
};

inline constexpr bool operator==(const FixedPoint& l, const FixedPoint& r) noexcept
{
    return l.value == r.value;
}

inline constexpr bool operator<(const FixedPoint& l, const FixedPoint& r) noexcept
{
    return l.value < r.value;
}

inline constexpr bool operator>(const FixedPoint& l, const FixedPoint& r) noexcept
{
    return l.value > r.value;
}

inline constexpr bool operator<=(const FixedPoint& l, const FixedPoint& r) noexcept
{
    return l.value <= r.value;
}

inline constexpr bool operator>=(const FixedPoint& l, const FixedPoint& r) noexcept
{
    return l.value >= r.value;
}

inline constexpr FixedPoint operator-(int32_t i, const FixedPoint& rhs) noexcept
{
    return FixedPoint{i} - rhs;
}

inline constexpr FixedPoint operator*(int32_t i, const FixedPoint& rhs) noexcept
{
    return rhs * i;
}

inline std::ostream& operator<<(std::ostream& o, const FixedPoint& f)
{
    return o << f.asFloat();
}
