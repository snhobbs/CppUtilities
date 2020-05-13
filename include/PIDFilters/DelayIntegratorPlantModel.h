/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * */
#pragma once
#ifndef PIDFILTERS_DELAYINTEGRATORPLANTMODEL_H_
#define PIDFILTERS_DELAYINTEGRATORPLANTMODEL_H_

#include <Calculators/CalculatorBase.h>
#include <PIDFilters/PICalculator.h>
#include <PIDFilters/PIDFilter.h>
#include <cstdint>

template <int64_t kDelayMicroSeconds, int64_t kSlopeMicroKelvinPerSecondPerActuatorUnit, int64_t kUpdateRateHz>
class DelayIntegratorPlantModel {
  static const constexpr double kHeatLeakFactor = 0;  //  1e-9;
  static const constexpr double kSlope =
      static_cast<double>(kSlopeMicroKelvinPerSecondPerActuatorUnit) / 1e6;
  static const constexpr double kTimeStep = 1. / kUpdateRateHz;  //  seconds/step

  uint32_t _time_step_count = 0;
  double control_ = 0;
  double temperature_;
  const double kAmbient;

  double CalculateHeatLeak(double temperature, double ambient, double factor) {
    //  acts like a control value in the direction of ambient temp
    double temp_diff = (ambient - temperature);
    double heat_leak = temp_diff * temp_diff * factor;
    return heat_leak;
  }

 public:
  void CalculateStep(double control) {
    _time_step_count++;
    double time = kTimeStep * _time_step_count;  //  seconds
    if (time * 1e6 < kDelayMicroSeconds) {
      return;
    }

    //  CalculateHeatLeak(GetTemperature(), kAmbient, kHeatLeakFactor);
    double temperature_forcing = 0;
    temperature_ += (kSlope * (control_ + temperature_forcing) * kTimeStep);
    //  temp/s/control, use last control setting as there is a delay
    control_ = control;
  }
  double GetTemperature(void) const { return temperature_; }

  static inline constexpr PIFilterCoeffs GetPIFilterCoeffs(void) {
    return Utilities::CalculateMultiplyShiftFilterCoefficients(kDelayMicroSeconds, kSlopeMicroKelvinPerSecondPerActuatorUnit, kUpdateRateHz);
  }
  explicit DelayIntegratorPlantModel(double temp_start = 0)
      : temperature_{temp_start}, kAmbient{temp_start} {}
};
#endif  //  PIDFILTERS_DELAYINTEGRATORPLANTMODEL_H_
