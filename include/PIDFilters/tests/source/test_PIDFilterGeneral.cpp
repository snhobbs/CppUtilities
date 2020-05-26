/*
 * Copyright 2020 Electrooptical Innovations
 * */
#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>
#include <vector>

#define private public
#include <PIDFilters/PIDFilter.h>

static const constexpr int32_t kShiftp = 3;
static const constexpr int32_t kShifti = 3;
static const constexpr int32_t kMultp = 109;
static const constexpr int32_t kMulti = 512;

static const constexpr int32_t kInputMax = (1 << 12) - 1;
static const constexpr int32_t kInputMin = -(1 << 12);

static const int32_t set = 1000;

TEST(PID, Constructor) {
  // fixme should the coefficients allow a negative number to be set?
  std::cout << "PIDFilter\t" << __TIME__ << "\n";

  PIFilterCoeffs coeffs{kShiftp, kShifti, kMultp, kMulti};
  IIR_PI_Filter filter{set, coeffs};

  EXPECT_EQ(filter.coeffs_.shift_p, kShiftp);
  EXPECT_EQ(filter.coeffs_.shift_i, kShifti);
  EXPECT_EQ(filter.coeffs_.mult_p, kMultp);
  EXPECT_EQ(filter.coeffs_.mult_i, kMulti);
}

TEST(PID, Startup) {
  /*
   Check that the start of the filter is running as a proportional controller
   until the filter is primed
   */
  PIFilterCoeffs coeffs{kShiftp, kShifti, kMultp, kMulti};
  IIR_PI_Filter filter{set, coeffs};

  int32_t procv = 100;
  auto err = procv - set;
  auto cntrl = filter.RunProportional(procv);

  int32_t expected = -(err * filter.coeffs_.mult_p) >> filter.coeffs_.shift_p;
  EXPECT_NEAR(cntrl, expected, 1);
}

TEST(PID, Integral) {
  // test the integral over a range of legal value

  PIFilterCoeffs coeffs{kShiftp, kShifti, kMultp, kMulti};

  IIR_PI_Filter filter{set, coeffs};
  int intVal = 8384512; // int from (x) from 0-4095 -> 1/2(x**2) = 8384512
  int integ = 0;
  int last = 0;

  for (int i = 0; i <= kInputMax; i++) {
    filter.x_m1_ = filter.x_;
    filter.x_ = i;
    filter.MakeIntegral();
    integ += (i + last);
    last = i;
    EXPECT_NEAR(filter.GetIntegralValue(), integ / 2, 2);
  }
  EXPECT_NEAR(intVal, filter.GetIntegralValue(), 2);
}

TEST(PID, NegativeIntegral) {
  // test the integral over a range of legal value

  PIFilterCoeffs coeffs{0, 0, 1, 1};

  IIR_PI_Filter filter{set, coeffs};
  int intVal = -8384512; // int from (x) from 0-4095 -> 1/2(x**2) = 8384512
  int integ = 0;
  int last = 0;
  for (int i = kInputMin; i < 0; i++) {
    filter.x_m1_ = filter.x_;
    filter.x_ = i;
    filter.MakeIntegral();
    integ += (i + last);
    last = i;
    EXPECT_NEAR(filter.GetIntegralValue(), integ / 2, 2);
  }
  EXPECT_NEAR(intVal, filter.GetIntegralValue(), 10000);
}
#if 0
TEST(PID, PositiveIntegral) {
  // test the integral over a range of legal value

  PIFilterCoeffs coeffs{0, 0, 1, 1};

  IIR_PI_Filter filter{set, coeffs};
  int intVal = 8384512; // int from (x) from 0-4095 -> 1/2(x**2) = 8384512
  int integ = 0;
  int last = 0;
  for (int i = kInputMin; i < 0; i++) {
    filter.x_m1_ = filter.x_;
    filter.x_ = i;
    filter.MakeIntegral();
    integ += (i + last);
    last = i;
    EXPECT_NEAR(filter.GetIntegralValue(), integ / 2, 2);
  }
  EXPECT_NEAR(intVal, filter.GetIntegralValue(), 10000);
}
#endif
