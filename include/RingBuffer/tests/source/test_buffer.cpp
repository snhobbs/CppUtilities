/*
 * Copyright 2020 Electrooptical Innovations
 * test_buffer.cpp
 *
 *  Created on: Jun 27, 2018
 *      Author: simon
 */
#include "UnitTest++/UnitTest++.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

#define private public
#define protected public
#include "RingBuffer/RingBuffer.h"

struct RingBuffSetup {
  static const std::size_t kBuffSize = 4096;
};
TEST_FIXTURE(RingBuffSetup, Count) {
  RingBuffer<char, kBuffSize> buff;
  for (std::size_t i = 0; i < kBuffSize * 5;) {
    i++;
    buff.insert(static_cast<char>((i + 1) & 0xff));
    if (i > buff.size()) {
      CHECK_EQUAL(buff.size(), buff.GetCount());
      CHECK(buff.isFull());
    } else {
      CHECK_EQUAL(i, buff.GetCount());
    }
  }
}

TEST_FIXTURE(RingBuffSetup, Peek) {
  RingBuffer<char, kBuffSize> buff;
  std::string txt = "Hello World, This is the test data";
  for (const char &b : txt) {
    buff.insert(b);
  }
  for (std::size_t i = 0; i < buff.GetCount(); i++) {
    char ch = '\0';
    buff.peek(&ch, i);
    CHECK_EQUAL(ch, txt[txt.size() - i]);
  }
}

TEST_FIXTURE(RingBuffSetup, PeekLong) {
  RingBuffer<uint32_t, kBuffSize> buff;
  std::vector<uint32_t> data{0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
                             11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  std::vector<uint32_t> long_data;
  while (long_data.size() <= kBuffSize * 2) {
    long_data.insert(long_data.end(), data.begin(), data.end());
  }
  // std::size_t count = 0;
  for (auto b : long_data) {
    // count++;
    buff.insert(b);
  }

  CHECK_EQUAL(true, buff.isFull());
  CHECK_EQUAL(false, buff.isEmpty());
  CHECK(buff.GetCount() == buff.GetSize());

  {
    uint32_t dp = 0;

    buff.peek(&dp, 0);
    CHECK_EQUAL(dp, buff.data[buff.MaskIndex(buff.GetHead())]);

    buff.peek(&dp, 1);
    CHECK_EQUAL(dp, buff.data[buff.MaskIndex(buff.GetHead() - 1)]);

    buff.peek(&dp, 2);
    CHECK_EQUAL(dp, buff.data[buff.MaskIndex(buff.GetHead() - 2)]);

    buff.peek(&dp, buff.GetCount());
    CHECK_EQUAL(dp, *buff.data.begin()); // [buff.GetTail()]);
  }

  for (std::size_t i = 0; i <= buff.GetCount(); i++) {
    uint32_t dp = 0;
    buff.peek(&dp, i);
    std::size_t index = buff.size() - i;
    auto ltent = buff.data[buff.MaskIndex(static_cast<uint32_t>(index))];
    CHECK_EQUAL(dp, ltent);
  }
}

TEST_FIXTURE(RingBuffSetup, InsertMulti) {
  RingBuffer<char, kBuffSize> buff;
  std::string txt = "Hello World, This is the test data";
  std::string longTxt;
  while (longTxt.size() < kBuffSize) {
    longTxt.append(txt);
  }
  buff.insert(const_cast<char *>(longTxt.data()), kBuffSize); // Overruns!

  for (std::size_t i = 0; i < buff.GetCount(); i++) {
    char ch = '\0';
    buff.peek(&ch, i);
    CHECK_EQUAL(ch, buff.data[buff.data.size() - i]);
  }
}

TEST_FIXTURE(RingBuffSetup, InsertOverRun) {
  RingBuffer<char, kBuffSize> buff;
  std::string txt = "Hello World, This is the test data";
  std::string longTxt;
  std::vector<char> outTxt;
  outTxt.reserve(kBuffSize);

  while (longTxt.size() <= kBuffSize) { // longtext is longer than buffer
    longTxt.append(txt);
  }

  CHECK((longTxt.size()) > (kBuffSize));

  for (auto &&ch : longTxt) {
    buff.insertOverwrite(std::move(ch));
  }
  buff.pop(outTxt.data(), kBuffSize);

  for (std::size_t i = 0; i < outTxt.size(); i++) {
    CHECK_EQUAL(outTxt[outTxt.size() - 1 - i], longTxt[longTxt.size() - 1 - i]);
  }
}

TEST_FIXTURE(RingBuffSetup, Pop) {
  RingBuffer<uint32_t, kBuffSize> buff;
  std::vector<uint32_t> long_data;

  uint32_t i = 0;
  while (long_data.size() < kBuffSize) {
    long_data.push_back(i++);
  }

  for (auto b : long_data) {
    CHECK_EQUAL(false, buff.isFull());
    buff.insert(b);
  }

  CHECK_EQUAL(true, buff.isFull());
  CHECK_EQUAL(false, buff.isEmpty());
  CHECK(buff.GetCount() == buff.GetSize());

  std::reverse(long_data.begin(), long_data.end());
  while (!buff.isEmpty()) {
    uint32_t dp = 0;
    buff.pop(&dp);
    CHECK_EQUAL(dp, long_data.back());
    long_data.pop_back();
  }
}

TEST_FIXTURE(RingBuffSetup, PopMulti) {
  RingBuffer<char, kBuffSize> buff;
  std::string txt = "Hello World, This is the test data";
  std::string longTxt;
  std::vector<char> outTxt;
  outTxt.reserve(kBuffSize);

  while (longTxt.size() <= kBuffSize) {
    longTxt.append(txt);
  }
  buff.insert(longTxt.c_str(), kBuffSize);
  buff.pop(outTxt.data(), kBuffSize);
  for (std::size_t i = 0; i < buff.GetCount(); i++) {
    CHECK_EQUAL(outTxt[i], longTxt[i]);
  }
}
