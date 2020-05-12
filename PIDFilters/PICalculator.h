/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * */
#pragma once
#ifndef PIDFILTERS_PICALCULATOR_H_
#define PIDFILTERS_PICALCULATOR_H_

#include <Calculators/CalculatorBase.h>
#include <PIDFilters/PIDFilter.h>
#include <cassert>
#include <cstdint>

struct TuningParameters {
  double ki = 0;
  double kp = 0;
  constexpr TuningParameters(const double ki_in, const double kp_in) :
    ki{ki_in}, kp{kp_in} {}
};

class DelayIntegratorPIFilterCoefficientCalculator {
  static const constexpr double kMicroFactor = 1e6;

 public:
  static constexpr TuningParameters SmicTuning(double delay_micro_seconds,
                                               double micro_slope,
                                               double update_frequency) {
    const double delay_seconds = delay_micro_seconds / kMicroFactor;
    const double slope = micro_slope / kMicroFactor;
    const double alpha = 16;
    const double beta = 0.4;
    const double kp = beta / (2 * slope * delay_seconds);
    const double ti = alpha * delay_seconds;
    const double ki = (kp / ti) / update_frequency;
    return TuningParameters{ki, kp};
  }
};

namespace Utilities {
template<typename T>
inline constexpr PIFilterCoeffs CalculateMultiplyShiftFilterCoefficients(
    T delay_microseconds, T micro_slope, T update_rate_hz, std::size_t shift_max = 14) {
  const TuningParameters tuning_parameters_{
      DelayIntegratorPIFilterCoefficientCalculator::SmicTuning(
          static_cast<double>(delay_microseconds),
          static_cast<double>(micro_slope),
          static_cast<double>(update_rate_hz))};
  const auto kMultiplierMax = static_cast<double>(1 << shift_max);
  const auto kKiEstimate =
      Utilities::MultiplyShiftEstimate<int32_t>(tuning_parameters_.ki,
                                                kMultiplierMax, shift_max);
  const auto kKpEstimate =
      Utilities::MultiplyShiftEstimate<int32_t>(tuning_parameters_.kp,
                                                kMultiplierMax, shift_max);
  //  static_assert(std::get<1>(kKpEstimate) != 0, "");
  //  static_assert(std::get<1>(kKiEstimate) != 0, "");
  assert(kKpEstimate.multiplier != 0);
  assert(kKiEstimate.multiplier != 0);

  return PIFilterCoeffs{kKpEstimate.shift, kKiEstimate.shift,
    kKpEstimate.multiplier, kKiEstimate.multiplier};
}
}  // namespace Utilities

#endif  //  PIDFILTERS_PICALCULATOR_H_
