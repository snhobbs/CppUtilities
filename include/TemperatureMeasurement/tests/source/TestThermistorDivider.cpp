/*
 * Copyright 2020 Electrooptical Innovations
 * */

#include <Calculators/ThermistorCalculator.h>
#include <TemperatureMeasurement/ThermistorDivider.h>
#include <gtest/gtest.h> 

#include <cstdio>
#include <iostream>

class Thermistor : public ThermistorDividerBase {
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
  static constexpr const auto kTable = TemperatureTableMaker<
      kTemperatureStart, kTemperatureEnd, kNumPoints,
      decltype(temperature_to_adc)>::GetTable(temperature_to_adc);

  static constexpr int32_t GetMicroCelsiusFromAdcReading(uint32_t adc_reading) {
    return InterpolatePoint(
        adc_reading, FindMatchingIndex(adc_reading), kTable.data());
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

TEST(Thermistor, FindMatchingIndex) {
  Thermistor cont{};

  uint32_t last_index = std::numeric_limits<uint32_t>::max();
  for (int i = 0; i < (1 << Thermistor::kADCBits); i += 1) {
    const auto index = cont.FindMatchingIndex(i);
    EXPECT_TRUE(index <= last_index);
    last_index = index;
  }
}


TEST(Thermistor, TableMakerInterpolation) {
  Thermistor cont{};

  const int32_t kTempStep = 1e6;
  for (unsigned int i = 0; i < cont.kTable.size(); i++) {
    EXPECT_NEAR(cont.kTable[i].t0,
                cont.GetMicroCelsiusFromAdcReading(cont.kTable[i].adc_reading),
                15);
  }

  uint32_t last_index = std::numeric_limits<uint32_t>::max();
  int32_t last_temp = cont.GetMicroCelsiusFromAdcReading(0);
  for (int i = 0; i < (1 << Thermistor::kADCBits); i += 1) {
    const auto index = cont.FindMatchingIndex(i);
    EXPECT_TRUE(index <= last_index);
    // printf("%d\n", index);
    const int32_t temp = cont.GetMicroCelsiusFromAdcReading(i);
    // cout << i << "\t" << cont.GetTemperatureInMicroCelsius(i) << endl;
    EXPECT_TRUE(temp <= last_temp);
    EXPECT_NEAR(temp, last_temp, kTempStep);
    if (temp > last_temp) {
      printf("%d\t%d\t%d\t%d\t%u\n", i, temp, last_temp, index, last_index);
    }
    last_temp = temp;
    last_index = index;
  }
}
