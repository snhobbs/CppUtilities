/*
 * Copyright 2020 Electrooptical Innovations
 * */
#include "UnitTest++/UnitTest++.h"
#include <Utilities/Crc.h>
#include <array>
#include <iostream>
#include <vector>
#include <cassert>

TEST(crc4) {
  std::vector<uint8_t> data;
  for (uint8_t pt = 0; pt < 128; pt++) {
    data.push_back(pt);
    const std::size_t crc_new = Utilities::crc4_new(data.data(), data.size());
    const std::size_t crc = Utilities::crc4(data.data(), data.size());
    CHECK_EQUAL(crc, crc_new);
  }
}
