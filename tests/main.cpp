/*
 * Copyright 2020 Electrooptical Innovations
 * main.cpp

 *
 *      Author: simon
 */

#include "UnitTest++/UnitTest++.h"

#include <stdint.h>
#include <array>
#include <iostream>
#include <vector>
TEST(Sanity) { CHECK_EQUAL(1, 1); }

int main(void) {
  auto ret = UnitTest::RunAllTests();

  std::cout << "Compiled: " << __TIME__ << '\t' << __DATE__ << std::endl;
#if 0
	for(auto i = 0; i < argc; i++) {
		std::cout << argv[i] << '\n';
	}
#endif
  return ret;
}
