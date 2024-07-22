/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * FiniteDifference.h
 *
 *  Created on: Nov 15, 2018
 *      Author: simon
 */

#pragma once
#ifndef FINITEDIFFERENCE_FINITEDIFFERENCE_H_
#define FINITEDIFFERENCE_FINITEDIFFERENCE_H_
#include <algorithm>
#include <array>
#include <cassert>

class SecFiniteDif {
 public:
  static const size_t kSecondDerivOrder = 2;
  static const size_t kWindowCount = kSecondDerivOrder + 2;
  size_t boxcar_length_ = 0;
  std::array<int32_t, kWindowCount> windows{};

 private:
  size_t average_count_ = 0;
  size_t window_index_ = 0;

  void rotate_windows(void) {
    //  window 0 is the oldest, window n-1 is being filled, window n-2 is the
    //  newest completed
    for (size_t i = 0; i < windows.size() - 1; i++) {
      windows[i] = windows[i + 1];
    }
    assert(window_index_ == windows.size() - 1);
    windows.back() = 0;
  }

 public:
  void set_boxcar_length(const size_t boxcar_length) {
    boxcar_length_ = boxcar_length;
  }

  bool is_primed(void) const { return window_index_ >= windows.size() - 1; }

  void reset(void) {
    average_count_ = 0;
    window_index_ = 0;
    std::fill(windows.begin(), windows.end(), 0);
    assert(windows.front() == 0);
    assert(windows.back() == 0);
  }

  void run(const int32_t data) {
    average_count_ += 1;
    windows[window_index_] += data;

    if (average_count_ >= boxcar_length_) {
      average_count_ = 0;
      if (!is_primed()) {
        window_index_ += 1;
        return;
      }
      assert(window_index_ == windows.size() - 1);
      rotate_windows();
    }
  }

  int32_t get_sfdiff(void) const {
    /*
     * The actual sfdiff is this value divided by the (boxcar length)**3 as we
     * need to take the average and then divide by the step**2
     */
    if (!is_primed()) {
      return 0;
    }
    return static_cast<int32_t>(static_cast<int64_t>(windows[0]) -
                                2 * static_cast<int64_t>(windows[1]) +
                                static_cast<int64_t>(windows[2]));
  }

  int32_t get_fdiff(void) const {
    /*
     * The actual sfdiff is this value divided by the (boxcar length)**3 as we
     * need to take the average and then divide by the step**2
     */
    if (!is_primed()) {
      return 0;
    }
    return static_cast<int32_t>(static_cast<int64_t>(windows[2]) -
                                static_cast<int64_t>(windows[1]));
  }

  int32_t get_value(void) const {
    /*
     * The actual sfdiff is this value divided by the (boxcar length)**3 as we
     * need to take the average and then divide by the step**2
     */
    if (!is_primed()) {
      return 0;
    }
    return windows[2];
  }
  SecFiniteDif(const size_t box_car_length = 0)
      : boxcar_length_{box_car_length} {}
};

#endif  //  FINITEDIFFERENCE_FINITEDIFFERENCE_H_
