/*
 * Copyright 2020 Electrooptical Innovations
 * */
#include "UnitTest++/UnitTest++.h"
#include <Utilities/Crc.h>
#include <array>
#include <iostream>
#include <vector>
#include <cassert>

/*
 * Example from MS56XX AN520
 * */
TEST(crc4_data) {
  const std::size_t result = 0xB;
  const std::array<uint16_t, 8> u16data {0x3132, 0x3334, 0x3536, 0x3738, 0x3940, 0x4142, 0x4344, 0x4500};
  std::vector<uint8_t> data;
  for (auto pt : u16data) {
    data.push_back(pt >> 8);
    data.push_back(pt & 0xff);
  }
  const std::size_t crc = Utilities::crc4(data.data(), data.size());
  CHECK_EQUAL(result, crc);
}

TEST(crc8) {
  const std::size_t result = 0x1e;
  std::vector<uint8_t> data {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
  const std::size_t crc = Utilities::crc8(data.data(), data.size(), 0xff);
  CHECK_EQUAL(result, crc);
}
