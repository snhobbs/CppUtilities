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

#include <Utilities/TypeConversion.h>
#include <Calculators/CalculatorBase.h>
#include <cmath>
#include <cstdint>
#include <array>
#include <cassert>

class TemperatureCalculator{
    static const constexpr double kCelsiusOffsetKelvin = 273.15;

 public:
    static constexpr double KelvinToCelsius(double kelvin) {
        return kelvin - kCelsiusOffsetKelvin;
    }
    static constexpr double CelsiusToKelvin(double celsius) {
        return kCelsiusOffsetKelvin + celsius;
    }
    static constexpr double Calculate_r_inf(double R0, double BFactor, double temp0_celsius) {
        return R0 * std::exp(-BFactor/(CelsiusToKelvin(temp0_celsius)));
    }

    static constexpr int32_t Calculate_r_inf_micro_ohms(double R0, double BFactor, double temp0_celsius) {
        return Calculator<int32_t>::TranslateToMicro<double>(Calculate_r_inf(R0, BFactor, temp0_celsius));
    }

    static constexpr double TemperatureFromResistance(double resistance, double BFactor, double r_inf) {
        return BFactor/std::log(resistance/r_inf);
    }

    static constexpr double ResistanceFromTemperature(double kelvin, double BFactor, double r_inf) {
         return std::exp(BFactor/kelvin)*r_inf;
    }
};

const uint32_t kBFactorTest = 4000;
const constexpr double kRInfTest = TemperatureCalculator::Calculate_r_inf(10000, kBFactorTest, 25);

#if 0 // FIXME
static_assert(TemperatureCalculator::Calculate_r_inf_micro_ohms(10000, kBFactorTest, 25) <= 42100, "");
static_assert(TemperatureCalculator::Calculate_r_inf_micro_ohms(10000, kBFactorTest, 25) >= 42000, "");
#endif

static_assert(TemperatureCalculator::TemperatureFromResistance(10000, kBFactorTest, kRInfTest) <= TemperatureCalculator::CelsiusToKelvin(25.01), "");
static_assert(TemperatureCalculator::TemperatureFromResistance(10000, kBFactorTest, kRInfTest) >= TemperatureCalculator::CelsiusToKelvin(24.99), "");

//static_assert(TemperatureCalculator::ADCFromTemperature(25, 4000, 42095, 10e3, 8) == 255, "");

#endif  //  CALCULATORS_THERMISTORCALCULATOR_H_
