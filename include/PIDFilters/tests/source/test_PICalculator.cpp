/*
 * Copyright 2020 Electrooptical Innovations
 * */

#include "UnitTest++/UnitTest++.h"
#include <PIDFilters/ControllerBase.h>
#include <PIDFilters/DelayIntegratorPlantModel.h>
#include <PIDFilters/PICalculator.h>
#include <iostream>
#include <vector>
#include <cstdint>
namespace {
TEST(runner) {
  const constexpr double kDelaySeconds = 1.1;
  const constexpr double kDegreesPerSecondPerAmp = 0.6;
  const constexpr int32_t update_rate = 100;
  const constexpr auto kDelayMicroSeconds =
      static_cast<int32_t>(kDelaySeconds * 1e6);
  const constexpr auto kSlope =
      static_cast<int32_t>(kDegreesPerSecondPerAmp * 1e6); // micro amps
  auto set_point = static_cast<int32_t>(26e6);
  auto plant_temp = static_cast<double>(35e6);

  DelayIntegratorPlantModel<kDelayMicroSeconds, kSlope, update_rate> plant {plant_temp};
  const PIFilterCoeffs coeffs = plant.GetPIFilterCoeffs();
  const FilterLimits lims{-3000000, 3000000};

  ControllerBase controller{set_point, coeffs, lims};
  PI_Filter_Status status;
  for (int i = 0; i < 20 * update_rate; i++) {
    const int32_t temp = static_cast<int32_t>(plant.GetTemperature());
    auto control = static_cast<double>(controller.RunFilter(temp, false));
    plant.CalculateStep(control);
    controller.GetFilterStatus(status);
    // std::cout << i << "\tTemp: " << temp << "\t Control: " << control << "\t"
    //          << std::endl;
  }
}
}  //  namespace
