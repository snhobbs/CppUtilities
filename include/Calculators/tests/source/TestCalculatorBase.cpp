/*
 * Copyright 2020 Electrooptical Innovations
 * */
#include <gtest/gtest.h> 
#include <Calculators/CalculatorBase.h>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <typeinfo>
#include <vector>

#define PRINT(x)                                                               \
  { std::cout << x << "\n"; }

TEST(Calculator, ScaleDigitalValue) {
  const uint32_t kBits = 18;
  double kScaleHigh[]{3.3e6, 5e6, 15e6};
  double kScaleLow[]{0, -5e6, -15e6};
  for (auto high : kScaleHigh) {
    for (auto low : kScaleLow) {
      int64_t last = std::numeric_limits<int64_t>::min();
      for (uint32_t i = 0; i < (1 << kBits); i++) {
        int64_t value = Calculator<int64_t>::ScaleDigitalValue<int64_t>(
            i, kBits, static_cast<int64_t>(low), static_cast<int64_t>(high));
        EXPECT_GT(value, last);
        last = value;
      }
      EXPECT_NEAR(last, high, 200);
    }
  }
}

TEST(Calculator, ScaleToDigitalValue) {
  const uint32_t kBits = 16;
  double kScaleHigh[]{3.3e6, 5e6, 15e6};
  double kScaleLow[]{0, -5e6, -15e6};
  for (auto high : kScaleHigh) {
    for (auto low : kScaleLow) {
      for (uint32_t i = 0; i < (1 << kBits); i++) {
        int64_t value = Calculator<int64_t>::ScaleDigitalValue<int64_t>(
            i, kBits, static_cast<int64_t>(low), static_cast<int64_t>(high));
        int64_t scaled = Calculator<int64_t>::ScaleToDigitalValue<int64_t>(
            value, kBits, static_cast<int64_t>(low), static_cast<int64_t>(high));

        EXPECT_EQ(i, scaled);
      }
    }
  }
}
#if 0
TEST(Calculator, ScaleToDigitalValuePrint) {
  const uint32_t kBits = 12;
  double kScaleHigh[]{3.3e6};
  double kScaleLow[]{0};
  for (auto high : kScaleHigh) {
    for (auto low : kScaleLow) {
      for (uint32_t i = 0; i < (1 << kBits); i++) {
        int64_t value = Calculator<int64_t>::ScaleDigitalValue<int64_t>(
            i, kBits, low, high);
        int64_t scaled = Calculator<int64_t>::ScaleToDigitalValue<int64_t>(
            value, kBits, low, high);

        EXPECT_EQ(i, scaled);
        std::cout << i << "\t" << value << "\t" << scaled << "\n";
      }
    }
  }
}
#endif

TEST(Calculator, MultipyShiftEstimate) {
  int kShiftMax = 16;
  double kMultMax = (1 << 16);
  double values[] = {-100,   -0.22222, 0.111199,  0,        0.1,
                     0.1313, 123.4545, 234.34230, 10000.999};
  for (auto pt : values) {
    auto out = Utilities::MultiplyShiftEstimate<int32_t>(pt, kMultMax, kShiftMax);
    //std::cout << pt << "\t" << out.error << "\n";
    if (pt != 0) {
      EXPECT_LT(static_cast<double>(out.error)/pt, 1e5);
    } else {
      EXPECT_LT(out.error, 1e5);
    }
  }
}

TEST(Calculator, TwoNodeVoltageDivider) {
  // TwoNodeVoltageDivider(const ValueType value, const Input fixed_value, const
  // Input fixed_resistor, const Input input_resistor, const Output kScale = 1,
  // const Output kValueScale = 1){

  std::vector<std::tuple<double, double, double, double, double>> cases{
      {5, 0, 1e3, 1e3, 2.5},   {0, 5, 1e3, 1e3, 2.5},   {50, 0, 1e6, 1e6, 25},
      {0, 50, 1e6, 1e6, 25},   {1, 1, 3e4, 1e6, 1},     {0, 0, 1e6, 3e6, 0},
      {-5, 0, 1e3, 1e3, -2.5}, {0, -5, 1e3, 1e3, -2.5}, {-50, 0, 1e6, 1e6, -25},
      {0, -50, 1e6, 1e6, -25}, {-1, -1, 3e4, 1e6, -1},  {0, 0, 1e6, 3e6, 0},
  };

  for (auto pt : cases) {
    double out = Calculator<double>::TwoNodeVoltageDivider<double>(
        std::get<0>(pt), std::get<1>(pt), std::get<2>(pt), std::get<3>(pt));
    EXPECT_NEAR(std::get<4>(pt), out, 1e-12);
    double v2 = Calculator<double>::TwoNodeVoltageDividerReverse<double>(
        std::get<0>(pt), std::get<4>(pt), std::get<2>(pt), std::get<3>(pt));
    EXPECT_NEAR(std::get<1>(pt), v2, 1e-12);
  }
}

TEST(Calculator, ThreeNodeVoltageDivider) {
  std::vector<
      std::tuple<double, double, double, double, double, double, double>>
      cases{
          {5, 0, 0, 1e3, 2e3, 2e3, 2.5},   {0, 5, 0, 2e3, 1e3, 2e3, 2.5},
          {0, 0, 5, 2e3, 2e3, 1e3, 2.5},   {-5, 0, 0, 1e3, 2e3, 2e3, -2.5},
          {0, -5, 0, 2e3, 1e3, 2e3, -2.5}, {0, 0, -5, 2e3, 2e3, 1e3, -2.5},
      };

  for (auto pt : cases) {
    double out = Calculator<double>::ThreeNodeVoltageDivider<double>(
        std::get<0>(pt), std::get<1>(pt), std::get<2>(pt), std::get<3>(pt),
        std::get<4>(pt), std::get<5>(pt));
    EXPECT_NEAR(std::get<6>(pt), out, 1e-12);
    double v3 = Calculator<double>::ThreeNodeVoltageDividerReversed<double>(
        std::get<0>(pt), std::get<1>(pt), std::get<6>(pt), std::get<3>(pt),
        std::get<4>(pt), std::get<5>(pt));
    EXPECT_NEAR(std::get<2>(pt), v3, 1e-12);
  }
}

TEST(Calculator, ThreeNodeVoltageDividerReversed) {
  constexpr double node_value = 1; // 1v
  double node_a_value = 0;         // gnd
  constexpr double node_a_resistor = 11e3;
  constexpr double node_b_resistor = 20.4e3;
  constexpr double output_fb_resistor = 36e3;

  double node_b_value_high = 3.3;
  double node_b_value_low = 0;

  double low = Calculator<double>::ThreeNodeVoltageDividerReversed<double>(
      node_a_value, node_b_value_high, node_value, node_a_resistor,
      node_b_resistor, output_fb_resistor);
  double high = Calculator<double>::ThreeNodeVoltageDividerReversed<double>(
      node_a_value, node_b_value_low, node_value, node_a_resistor,
      node_b_resistor, output_fb_resistor);
  EXPECT_NEAR(0.2, low, 0.05);
  EXPECT_NEAR(6, high, 0.5);
}

TEST(Calculator, InvertingAmplifier) {
  double noninverting_value = 5;
  double resistor_in = 2e5;
  double fb_resistor = 18e5;
  double high = 10;
  double low = 0;
  double kStep = 1e-6;
  double last = std::numeric_limits<double>::max();
  for (double inverting_value = low; inverting_value < high;
       inverting_value += kStep) {
    double output = Calculator<double>::AmplifierOutput<double>(
        noninverting_value, inverting_value, resistor_in, fb_resistor, high,
        low);
    double input_calculated =
        Calculator<double>::CalculateInvertingInputFromAmplifierOutput<double>(
            noninverting_value, output, resistor_in, fb_resistor);
    if (output >= high || output <= low) {
      continue;
    }
    EXPECT_NEAR(inverting_value, input_calculated, 0.0001);
    EXPECT_LT(output, last);
    last = output;
  }
}

TEST(Calculator, NonInvertingAmplifier) {
  double inverting_value = 5;
  double resistor_in = 2e5;
  double fb_resistor = 18e5;
  double high = 10;
  double low = 0;
  double kStep = 1e-3;
  double last = std::numeric_limits<double>::min();
  for (double noninverting_value = low; noninverting_value < high;
       noninverting_value += kStep) {
    double output = Calculator<double>::AmplifierOutput<double>(
        noninverting_value, inverting_value, resistor_in, fb_resistor, high,
        low);
    double input_calculated =
        Calculator<double>::CalculateNoneInvertingInputFromAmplifierOutput<
            double>(inverting_value, output, resistor_in, fb_resistor);
    if (output >= high || output <= low)
      continue;
    EXPECT_NEAR(noninverting_value, input_calculated, 0.0001);
    EXPECT_GT(output, last);
    last = output;
  }
}
