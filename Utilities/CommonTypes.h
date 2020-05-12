/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 *
 * */
#pragma once

#ifndef UTILITIES_COMMONTYPES_H_
#define UTILITIES_COMMONTYPES_H_
#include <cassert>
namespace Utilities {

template<typename T = std::size_t>
class Range {
  const T low_;
  const T high_;
 public:
  constexpr T GetLow(void) const {
    return low_;
  }
  constexpr T GetHigh(void) const {
    return high_;
  }
  constexpr Range(T low, T high) : low_{low}, high_{high} {
    assert(low < high);
  }
};
}  //  namespace Utilities

#endif  //  UTILITIES_COMMONTYPES_H_
