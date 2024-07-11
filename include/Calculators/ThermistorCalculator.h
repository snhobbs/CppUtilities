/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * TemperatureCalculator.h
 *
 *  Created on: Nov 12, 2019
 *      Author: simon
 */

#pragma once
#ifndef CALCULATORS_THERMISTORCALCULATOR_H_
#define CALCULATORS_THERMISTORCALCULATOR_H_

#include <Calculators/CalculatorBase.h>
#include <Utilities/TypeConversion.h>
#include <Utilities/math.h>

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>

namespace Utilities {
const constexpr double kCelsiusOffsetKelvin = 273.15;
inline constexpr double KelvinToCelsius(const double kelvin) {
  return kelvin - kCelsiusOffsetKelvin;
}

inline constexpr double CelsiusToKelvin(const double celsius) {
  return kCelsiusOffsetKelvin + celsius;
}

namespace TemperatureCalculator {
inline constexpr double Calculate_r_inf(const double R0, const double BFactor,
                                        const double temp0_celsius) {
  return R0 * Utilities::exp(-BFactor / (CelsiusToKelvin(temp0_celsius)));
}

inline constexpr int32_t Calculate_r_inf_micro_ohms(double R0, double BFactor,
                                                    double temp0_celsius) {
  return Calculator<int32_t>::TranslateToMicro<double>(
      Calculate_r_inf(R0, BFactor, temp0_celsius));
}

inline constexpr double TemperatureFromResistance(double resistance,
                                                  double BFactor,
                                                  double r_inf) {
  return BFactor / Utilities::log(resistance / r_inf);
}

inline constexpr double ResistanceFromTemperature(double kelvin, double BFactor,
                                                  double r_inf) {
  return Utilities::exp(BFactor / kelvin) * r_inf;
}

const uint32_t kBFactorTest = 4000;
const constexpr double kRInfTest = Calculate_r_inf(10000, kBFactorTest, 25);

#if 0  // FIXME
static_assert(Calculate_r_inf_micro_ohms(10000, kBFactorTest, 25) <= 42100, "");
static_assert(Calculate_r_inf_micro_ohms(10000, kBFactorTest, 25) >= 42000, "");
#endif

static_assert(TemperatureFromResistance(10000, kBFactorTest, kRInfTest) <=
                  CelsiusToKelvin(25.01),
              "");
static_assert(TemperatureFromResistance(10000, kBFactorTest, kRInfTest) >=
                  CelsiusToKelvin(24.99),
              "");

}  //  namespace TemperatureCalculator
}  //  namespace Utilities

#endif  //  CALCULATORS_THERMISTORCALCULATOR_H_
