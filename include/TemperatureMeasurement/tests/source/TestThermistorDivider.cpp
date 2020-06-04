/*
 * Copyright 2020 Electrooptical Innovations
 * */

#include <Calculators/ThermistorCalculator.h>
#include <TemperatureMeasurement/ThermistorDivider.h>
#include <gtest/gtest.h> 

#include <cstdio>
#include <iostream>

class Thermistor : public TemperatureMeasurement::ThermistorDividerBase {
 public:
  static const uint32_t kADCBits = 12;
  static const constexpr int32_t kRPullup = 17.4e3; // R46
  static const uint32_t kT0Celsius = 25;
  static const uint32_t kBFactor = 4000;
  static const constexpr uint32_t kR0 = 10e3;
  static const int32_t kTemperatureStart = -30;
  static const int32_t kTemperatureEnd = 50;
  static const uint32_t kNumPoints = 64;

  static const int32_t kPullupResistorMicroVolts = 3.3e6;
  static const int32_t kThermistorMicroVolts = 0;

  static const int32_t kAdcReferenceMicroVolts = kPullupResistorMicroVolts;

  // static constexpr const auto kTable =
  // TemperatureTableMaker<kTemperatureStart, kTemperatureEnd,
  // kNumPoints>::GetTable(kADCBits, kRPullup, kT0Celsius, kBFactor, kR0);
  static const constexpr auto temperature_to_adc =
      TemperatureMeasurement::TemperatureToAdcTwoNodeThermistor<
          kADCBits, kThermistorMicroVolts, kPullupResistorMicroVolts,
          kAdcReferenceMicroVolts, kRPullup, kT0Celsius, kBFactor, kR0>;

 public:
  static constexpr const auto kTable =
    TemperatureMeasurement::TemperatureTableMaker<kTemperatureStart, kTemperatureEnd, kNumPoints,
      decltype(temperature_to_adc)>::GetTable(temperature_to_adc);

  static constexpr int32_t GetMicroCelsiusFromAdcReading(uint32_t adc_reading) {
    const auto close_index = FindMatchingIndex(adc_reading);
    return InterpolatePoint(adc_reading, kTable[close_index]);
  }
  static constexpr std::size_t FindLowerMatchingIndex(std::size_t index) {
     return FindMatchingIndexLower(index, kTable.data(), kTable.size());
  }
  static constexpr std::size_t FindMatchingIndex(std::size_t index) {
     return FindClosestMatchingIndex(index, kTable.data(), kTable.size());
  }
};

#if 0
TEST(TableValues) {
  Thermistor cont{};

  // need n-1 > n

  for (auto entry : cont.kTable) {
    EXPECT_TRUE();
  }
}
#endif

TEST(Thermistor, FixedTemperatureIncreases) {
  auto last_temp = std::numeric_limits<decltype(Thermistor::kTable[0].t0)>::min();
  for (auto pt : Thermistor::kTable) {
    EXPECT_TRUE(pt.t0 >= last_temp);
    last_temp = pt.t0;
  }
}

TEST(Thermistor, IndexDecreases) {
  auto last = std::numeric_limits<decltype(Thermistor::kTable[0].adc_reading)>::max();
  for (auto pt : Thermistor::kTable) {
    EXPECT_TRUE(pt.adc_reading < last);
    last = pt.adc_reading;
  }
}

TEST(Thermistor, SlopeNegative) {
  for (auto pt : Thermistor::kTable) {
    EXPECT_TRUE(pt.DeltaTByDeltaADCSlope < 0);
  }
}

TEST(Thermistor, FindMatchingIndexLower) {
  uint32_t last_index = std::numeric_limits<uint32_t>::max();
  for (std::size_t i = 0; i < Thermistor::kTable[0].adc_reading; i += 1) {
    const auto index = Thermistor::FindLowerMatchingIndex(i);
    EXPECT_TRUE(index <= last_index);
    last_index = index;
  }
}

TEST(Thermistor, FindMatchingIndex) {
  uint32_t last_index = std::numeric_limits<uint32_t>::max();
  for (std::size_t i = 0; i < Thermistor::kTable[0].adc_reading; i += 1) {
    const auto index = Thermistor::FindMatchingIndex(i);
    EXPECT_TRUE(index <= last_index);
    last_index = index;
  }
}

TEST(Thermistor, T0CloseToInterpolatedValue) {
  for (auto pt : Thermistor::kTable) {
    EXPECT_EQ(pt.t0, Thermistor::GetMicroCelsiusFromAdcReading(pt.adc_reading));
  }
}

/*
 * Check that all interpolated temperatures between ranges are between the measured points
 * */
TEST(Thermistor, InterpolatedTemperatureLessThanNextPoint) {
  for (std::size_t i = 0; i < Thermistor::kTable.size() - 1; i++) {
    for (std::size_t index = Thermistor::kTable[i].adc_reading; index > Thermistor::kTable[i+1].adc_reading; index--) {
      const auto temp = Thermistor::GetMicroCelsiusFromAdcReading(index);
      EXPECT_LE(temp, Thermistor::kTable[i + 1].t0);
      EXPECT_GE(temp, Thermistor::kTable[i].t0);
    }
  }
}

#if 1

/*
 * ADC value decreases with increasing temp.
 * Check temperature calculated for the lower index is higher than the current calculated value
 * */
TEST(Thermistor, InterpolatedTemperatureIncreasesByIndex) {
  auto last_temp = std::numeric_limits<decltype(Thermistor::kTable[0].t0)>::max();
  for (std::size_t i = 0; i < Thermistor::kTable[0].adc_reading; i++) {
    const auto temp = Thermistor::GetMicroCelsiusFromAdcReading(i);

    EXPECT_GE(last_temp, temp);
    if(last_temp < temp) {
      printf("%u\tLast Index: %u, %u\t This Index: %u, %u\n", i, Thermistor::FindMatchingIndex(i-1), last_temp, Thermistor::FindMatchingIndex(i), temp);
    }
    last_temp = temp;
  }
}
#endif
