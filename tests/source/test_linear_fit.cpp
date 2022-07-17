/*
 * Copyright 2020 Electrooptical Innovations
 * main.cpp

 *
 *      Author: simon
 */

#include <gtest/gtest.h>
#include <stdint.h>
#include <array>
#include <iostream>
#include <vector>
#include "linear_fit.h"

const constexpr size_t kResponseLength = 6;

TEST(fit_linear, runs_0) {
  /*
   * Put in 0 data, check it compiles and
   * returns a slope and offset of 0;
   * */
  std::array<float, kResponseLength> res{};
  std::array<float, kResponseLength> x{1,2,3,4,5,6};
  std::array<float, kResponseLength> y{};
  fit_linear<float>(x.data(), y.data(), x.size(), &res);
  EXPECT_EQ(res[0], 0);  //  offset
  EXPECT_EQ(res[1], 0);  //  slope
}

TEST(fit_linear, check_generated_noiseless) {
  std::array<float, kResponseLength> res{};
  std::array<float, kResponseLength> x{1,2,3,4,5,6};
  const float slope = 1;
  const float offset = 2;
  for (auto& slope : {-10, -1, 0, 1, 10}) {
    for (auto& offset : {-10, -1, 0, 1, 10}) {
      std::array<float, kResponseLength> y{};
      for (size_t i=0; i<x.size(); i++) {
        y[i] = offset + x[i]*slope;
      }
      fit_linear<float>(x.data(), y.data(), x.size(), &res);

      EXPECT_NEAR(res[0], offset, 1e-3);  //  offset
      EXPECT_NEAR(res[1], slope, 1e-3);  //  slope
    }
  }
}
