#pragma once
#include <cstdint>
#ifdef __clang__
#include <Sprout/sprout/cmath.hpp>
#include <Sprout/sprout/math/exp.hpp>
#include <Sprout/sprout/math/log.hpp>
#else
#include <cmath>
#endif

namespace Utilities {
#ifdef __clang__
using sprout::math::ceil;
using sprout::math::exp;
using sprout::math::floor;
using sprout::math::log;
using sprout::math::pow;
//  using sprout::math::round;
//  using sprout::math::abs;
//  using exp = sprout::math::detail::exp;
//  using log = sprout::math::detail::log;

#else
using std::ceil;
using std::exp;
using std::floor;
using std::log;
using std::pow;
//  using std::round;
//  using std::abs;
#endif

#if 0
template <typename T, typename U = int64_t>
inline constexpr T floor(const T value) {
#ifdef __clang__
  static_assert(std::is_integral<U>());
  return (static_cast<T>(static_cast<U>(value)));
#else
  return std::floor(value);
#endif
}

template <typename T, typename U = int64_t>
inline constexpr T ceil(const T value) {
#ifdef __clang__
  static_assert(std::is_integral<U>());
  return (static_cast<T>(static_cast<U>(value - 0.5)) + 1);
#else
  return std::ceil(value);
#endif
}
#endif

template <typename T, typename U = int64_t>
inline constexpr T round(const T value) {
#ifdef __clang__
  static_assert(std::is_integral<U>());
  if (value >= 0) {
    return (static_cast<T>(static_cast<U>(value + 0.5)));
  } else {
    return (static_cast<T>(static_cast<U>(value - 0.5)));
  }
#else
  return std::round(value);
#endif
}

template <typename T>
inline constexpr T abs(const T t) {
#ifdef __clang__
  return t > 0 ? t : -t;
#else
  return std::abs(t);
#endif
}

template <typename T>
constexpr T AbsDiff(const T a, const T b) {
  return a > b ? a - b : b - a;
}
}  //  namespace Utilities
