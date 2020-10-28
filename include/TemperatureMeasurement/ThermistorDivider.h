/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * */

#pragma once
#ifndef TEMPERATUREMEASUREMENT_THERMISTORDIVIDER_H_
#define TEMPERATUREMEASUREMENT_THERMISTORDIVIDER_H_

#include <Calculators/CalculatorBase.h>
#include <Calculators/ThermistorCalculator.h>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

/*
 * Make a table that can take an adc value and lookup the temperature
 * has the adc value, the temp and the temp slope to the next point
 *
 * */

namespace TemperatureMeasurement {
/*
 * Two node voltage divider with one node as a thermistor and another as a fixed
 * resistor ADC measuring at node of divider
 * */
struct InterpolatedTemperatureLine {
  int32_t adc_reading = 0;
  int32_t t0 = 0;
  int32_t DeltaTByDeltaADCSlope = 0;
};


template <int32_t kAdcBits, int32_t kThermistorMicroVolts, int32_t kFixedMicroVolts,
    int32_t kAdcReferenceMicroVolts, int32_t kFixedResistor, int32_t kT0Celsius,
	int32_t kBFactor, int32_t kR0>
inline constexpr int32_t TemperatureToAdcTwoNodeThermistor(const double kelvin) {
  const auto r_inf = Utilities::TemperatureCalculator::Calculate_r_inf(kR0, kBFactor, kT0Celsius);
  assert(r_inf > 0);
  const auto thermistor_resistance = Utilities::TemperatureCalculator::ResistanceFromTemperature(kelvin, kBFactor, r_inf);
  const auto thermistor_resistance_rounded = Utilities::round<decltype(thermistor_resistance), int64_t>(thermistor_resistance);
  const auto thermistor_resistance_int = Utilities::StaticCastQuickFail<int64_t>(thermistor_resistance_rounded);
  assert(thermistor_resistance_int > 0);
  const auto node_micro_volts = Calculator<int64_t>::TwoNodeVoltageDivider<int64_t>(
          kThermistorMicroVolts, kFixedMicroVolts, thermistor_resistance_int, kFixedResistor);
  const int32_t adc_value = Calculator<int32_t>::ScaleToDigitalValue<int64_t>(
      node_micro_volts, kAdcBits, 0, kAdcReferenceMicroVolts);
  //  assert(adc_value < (1<<kAdcBits));
  return adc_value;
}

//  todo need to do something to handle different temps being the same adc value
template <int32_t kTCelsiusStart, int32_t kTCelsiusEnd, uint32_t kNumPoints, typename F>
class TemperatureTableMaker {
  static const constexpr auto kNumIntervals = (kNumPoints - 1);
  static_assert(kTCelsiusStart < kTCelsiusEnd, "Not increasing");

  static const constexpr double kTemperatureStepSize =
      static_cast<double>(kTCelsiusEnd - kTCelsiusStart) / kNumIntervals;
  static_assert(kTemperatureStepSize * kNumIntervals + kTCelsiusStart ==
                kTCelsiusEnd);

  static constexpr void SetSlopeBetweenPoints(std::array<InterpolatedTemperatureLine, kNumPoints>* arr) {
    for (std::size_t i = 0; i < arr->size() - 1; i++) {
      const double adc_reading = arr->at(i).adc_reading;
      const double next_adc_reading = arr->at(i + 1).adc_reading;
      const double tdiff = arr->at(i + 1).t0 - arr->at(i).t0;
      const auto slope = Utilities::round<decltype(tdiff), int32_t>(
          (tdiff) / (next_adc_reading - adc_reading));
      const auto rounded_slope = Utilities::StaticCastQuickFail<int32_t>(slope);
      const auto tdiff_calc = (next_adc_reading - adc_reading) * rounded_slope;

      arr->at(i).DeltaTByDeltaADCSlope = rounded_slope;
    }
    //  last point has same slope as preceeding one
    arr->at(arr->size() - 1).DeltaTByDeltaADCSlope =
        arr->at(arr->size() - 2).DeltaTByDeltaADCSlope;
  }

 public:
  /*
   * Generate an array of interpolation points with slopes between them
   * */
  static constexpr std::array<InterpolatedTemperatureLine, kNumPoints>
  GetTable(F TemperatureToAdc) {
    std::array<InterpolatedTemperatureLine, kNumPoints> arr;

    for (std::size_t i = 0; i < arr.size(); i++) {
      const double t = kTCelsiusStart + i * kTemperatureStepSize;
      const auto adc_reading = TemperatureToAdc(Utilities::CelsiusToKelvin(t));
      assert(adc_reading >= 0);
      arr[i].adc_reading = Utilities::StaticCastQuickFail<int32_t>(adc_reading);
      arr[i].t0 = Utilities::StaticCastQuickFail<int32_t>(
          Utilities::round<double, int32_t>(
              Calculator<double>::TranslateToMicro(t)));
    }

    SetSlopeBetweenPoints(&arr);
    return arr;
  }
};

class ThermistorDividerBase {
 //protected:
 public:
  static constexpr std::size_t 
  FindMatchingIndexLower(const std::size_t index,
                         const InterpolatedTemperatureLine *const table,
                         const std::size_t array_size) {
    const int32_t adc_reading = static_cast<int32_t>(index);
    for (std::size_t i = 0; i < array_size; i++) {
      if (table[i].adc_reading < adc_reading) { //  table is decreasing in adc
        return i;
      }
    }
    //  assert(i < kTable.size());//out of range
    return array_size - 1;
  }

  static constexpr std::size_t
  FindClosestMatchingIndex(const std::size_t index,
                           const InterpolatedTemperatureLine *const table,
                           const std::size_t array_size) {
    const int32_t adc_reading = static_cast<int32_t>(index);
    int32_t last_error = std::numeric_limits<int32_t>::max();
    std::size_t i = 0;
    for (i = 0; i < array_size; i++) {
      const int32_t err = table[i].adc_reading > adc_reading
                              ? table[i].adc_reading - adc_reading
                              : adc_reading - table[i].adc_reading;
      if (err > last_error) {
        if (i) {
          return i - 1;
        } else {
          return 0;
        }
      }
      last_error = err;
    }
    //  assert(i < kTable.size());//out of range
    assert(i == array_size);
    return array_size - 1;
  }

  static constexpr int32_t
  InterpolatePoint(const int32_t adc_reading, const InterpolatedTemperatureLine& pt_close) {

    //  assert(lower_match_index);
    const int32_t step_size = (adc_reading - pt_close.adc_reading);
    const int32_t gain_calc =
        (pt_close.DeltaTByDeltaADCSlope * step_size) + pt_close.t0;

    //  pin the value to the next known point if higher
    return gain_calc < pt_close.t0 ? pt_close.t0 : gain_calc;
  }
};

} //  namespace TemperatureMeasurement
#endif  //  TEMPERATUREMEASUREMENT_THERMISTORDIVIDER_H_
