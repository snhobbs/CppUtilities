/*
 * Copyright 2020 Electrooptical Innovations
 * */
#include "UnitTest++/UnitTest++.h"
#include <Utilities/Crc.h>
#include <array>
#include <iostream>
#include <vector>

TEST(crc4) {
  std::vector<uint8_t> data;
  for (uint8_t pt = 0; pt < 128; pt++) {
    data.push_bask(pt);
  }
  CHECK_EQUAL(Utilities::crc4(data.data(), data.size()), Utilities::crc4_new(data.data(), data.size()));
}
