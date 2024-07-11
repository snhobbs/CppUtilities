/* Copyright (C) 2020 Electrooptical Innovations
 * ----------------------------------------------------------------------
 * Project:      Modbus
 * Title:        test_Coil.cpp
 * Description:  Test coil controller
 *               (coils & discrete inputs)
 *
 * $Date:        13. May 2020
 * $Revision:    V.1.0.1
 * ----------------------------------------------------------------------
 */

#include <gtest/gtest.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "../BitField.h"

namespace Tests {
struct CoilDataFixture : public ::testing::Test {
  static const constexpr std::size_t kCoilCount = 69;
  BitField<kCoilCount> dc{};
};
TEST_F(CoilDataFixture, BitFieldRead) {
  for (std::size_t address = 0; address < dc.size(); address++) {
    dc.WriteElement(address, true);
    EXPECT_TRUE(dc.ReadElement(address) == true);
    dc.WriteElement(address, false);
    EXPECT_TRUE(dc.ReadElement(address) == false);
    dc.WriteElement(address, true);
    EXPECT_TRUE(dc.ReadElement(address) == true);
  }
  for (std::size_t index = 0; index < dc.size(); index++) {
    EXPECT_TRUE(dc.ReadElement(index) == true);
  }
  EXPECT_TRUE(dc.ReadElement(kCoilCount - 1) == true);
}

TEST_F(CoilDataFixture, BitFieldWriteSingleBit) {
  for (std::size_t index = 0; index < dc.size(); index++) {
    dc.WriteElement(index, true);
    EXPECT_TRUE(dc.ReadElement(index) == true);
    for (std::size_t sub_address = 0; sub_address < dc.size(); sub_address++) {
      if (sub_address == index) {
        EXPECT_TRUE(dc.ReadElement(sub_address) == true);
      } else {
        EXPECT_TRUE(dc.ReadElement(sub_address) == false);
      }
    }
    dc.WriteElement(index, false);
    EXPECT_TRUE(dc.ReadElement(index) == false);
  }
}

#if 0
TEST(ModbusCoils, WriteMultipleCoilsResponse) {
  Modbus::Response response;
  const uint16_t coil_count = 0xccaa;
  const uint16_t address = 0x34;
  const uint8_t slave_address = 0x56;
  Modbus::WriteMultipleCoilsCommand::FillResponseHeader(slave_address, address,
                                                        coil_count, &response);
  EXPECT_EQ(slave_address,
              response[Modbus::Command::ResponsePacket::kSlaveAddress]);
  EXPECT_EQ(
      static_cast<int>(Modbus::Function::kWriteMultipleCoils),
      static_cast<int>(response[Modbus::Command::ResponsePacket::kFunction]));
  const uint16_t response_coil_address = Modbus::WriteMultipleCoilsCommand::ReadResponseDataAddressStart(response);
  EXPECT_EQ(address, response_coil_address);
  const uint16_t response_coil_count = Modbus::WriteMultipleCoilsCommand::ReadResponseCoilCount(response);
  EXPECT_EQ(coil_count, response_coil_count);
}

struct CoilControllerArbitraryDataFixture : public CoilControllerFixture {
  static const std::size_t kBytes = 4;
  static const constexpr std::size_t coil_count = (8 * kBytes) - 3;
  static const constexpr std::array<bool, coil_count> coil_settings{
      false, true, false, true,  false, true,  false, false, false, true,
      false, true, false, true,  false, false, false, true,  false, true,
      false, true, false, false, false, true,  false, true,  false};
  void ApplyCoilSettings(const uint16_t starting_address) {
    std::size_t address = starting_address;
    for (bool val : coil_settings) {
      if (val)
        cc.WriteElement(address++, true);
      else
        cc.WriteElement(address++, false);
    }
  }
};

TEST_F(CoilControllerArbitraryDataFixture, CoilControllerReadWriteMulti) {
  ApplyCoilSettings(0);
  std::size_t index = 0;
  while (cc.IsAddressValid((index))) {
    if (index < coil_count) {
      EXPECT_EQ(coil_settings[index], cc.ReadCoil(index) ==
                                            Modbus::CoilState::kOn);
    } else {
      EXPECT_TRUE(cc.ReadCoil(index) == Modbus::CoilState::kOff);
    }
    index++;
  }
}
#endif
}  // namespace Tests
