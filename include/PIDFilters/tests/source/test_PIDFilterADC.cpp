/*
 * Copyright 2020 Electrooptical Innovations
 * */

#include <PIDFilters/PIDFilter.h>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <cstdint>

const constexpr int32_t kSetPoint = 1000;

TEST(PIDADC, Proportional) {
  // search over the whole range of a uint_12 checking the sign and value of the
  // proportional error
  const constexpr int32_t kShiftp = 6;
  const constexpr int32_t kShifti = 6;
  const constexpr int32_t kMultp = 5300;
  const constexpr int32_t kMulti = 5300;
  PIFilterCoeffs coeffs{kShiftp, kShifti, kMultp, kMulti};
  IIR_PI_Filter filter{kSetPoint, coeffs};

  const constexpr int32_t kInputMax = (1 << 12) - 1;
  for (int32_t i = 0; i <= kInputMax; i++) {
    auto err = i - kSetPoint;
    auto cntrl = filter.RunProportional(i);

    EXPECT_NEAR(cntrl, -((err * coeffs.mult_p) >> coeffs.shift_p), 1);

    // Extra sign checking ensuring negative feedback
    EXPECT_EQ(i <= kSetPoint, cntrl >= 0);
    EXPECT_EQ(i > kSetPoint, cntrl < 0);
  }
}

TEST(PIDADC, ProportionalSetLims) {
  // search over the whole range of a uint_12 checking the sign and value of the
  // proportional error
  PIFilterCoeffs coeffs{0, 0, 1, 1};
  IIR_PI_Filter filter{kSetPoint, coeffs};

  std::vector<int32_t> setpnts{0,   1,   2,    3,    128,  256,  200,
                               512, 899, 1024, 1999, 2048, 4000, 4095};
  const constexpr int32_t kInputMax = (1 << 12) - 1;

  for (auto set : setpnts) {
    filter.SetSetPoint(set);
    for (int32_t i = 0; i <= kInputMax; i++) {
      auto err = i - set;
      auto cntrl = filter.RunProportional(i);

      EXPECT_EQ(-(err * coeffs.mult_p) >> coeffs.shift_p,
                  cntrl);

      // Extra sign checking ensuring negative feedback
      EXPECT_EQ(i <= set, cntrl >= 0);
      EXPECT_EQ(i > set, cntrl < 0);
    }
  }
}

TEST(PIDADC, SetLims) {
  // search over the whole range of a uint_12 checking the sign and value of the
  // proportional error
  PIFilterCoeffs coeffs{0, 0, 1, 1};
  IIR_PI_Filter filter{kSetPoint, coeffs};

  std::vector<int32_t> setpnts{0,   1,   2,    3,    128,  256,  200,
                               512, 899, 1024, 1999, 2048, 4000, 4095};
  const constexpr int32_t kInputMax = (1 << 12) - 1;

  for (auto set : setpnts) {
    filter.SetSetPoint(set);
    for (int32_t i = 0; i <= kInputMax; i++) {
      auto cntrl = filter.run(i);
      // Extra sign checking ensuring negative feedback

      // if integral and prop are less than 0 then cntrl > 0
      // else if int and prop >= 0 cntrl <=0
      auto integ = filter.GetIntegralValue();
      if (integ <= 0 && i < set) {
        EXPECT_GT(cntrl, 0);
      } else if (integ >= 0 && i >= set) {
        EXPECT_LE(cntrl, 0);
      }
    }
  }
}
