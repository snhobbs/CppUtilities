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
  const size_t boxcar_length_;
  std::array<int32_t, kWindowCount> windows{};

 private:
  size_t average_count = 0;
  size_t window_index = 0;

  void rotate_windows(void) {
    //  window 0 is the oldest, window n-1 is being filled, window n-2 is the
    //  newest completed
    for (size_t i = 0; i < windows.size() - 1; i++) {
      windows[i] = windows[i + 1];
    }
    assert(windows.size() - 1 == window_index);
    windows[window_index] = 0;
  }

 public:
  bool is_primed(void) const { return window_index >= windows.size() - 1; }

  void reset(void) {
    average_count = 0;
    window_index = 0;
    std::fill(windows.begin(), windows.end(), 0);
    assert(windows[0] == 0);
    assert(windows[window_index] == 0);
  }

  void run(const int32_t data) {
    average_count += 1;
    windows[window_index] += data;

    if (average_count >= boxcar_length_) {
      average_count = 0;
      if (!is_primed()) {
        window_index += 1;
        return;
      } else {
        assert(windows.size() - 1 == window_index);
        rotate_windows();
      }
    }
  }

  int32_t get_sfdiff_(void) const {
    return windows[0] - 2 * windows[1] + windows[2];
  }

  int32_t get_sfdiff(void) const {
    /*
     * The actual sfdiff is this value divided by the (boxcar length)**3 as we
     * need to take the average and then divide by the step**2
     */
    if (!is_primed()) {
      return 0;
    }
    return get_sfdiff_();
  }
  explicit SecFiniteDif(const size_t box_car_length)
      : boxcar_length_{box_car_length} {}
};

#endif  //  FINITEDIFFERENCE_FINITEDIFFERENCE_H_
