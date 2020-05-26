/*
 * Copyright 2020 Electrooptical Innovations
 * */
#include <FiniteDifference/FiniteDifference.h>
#include <gtest/gtest.h>
#include <vector>
#include <iostream>

class FiniteDifferenceTester : public SecFiniteDif {
 public:
  int32_t GetWindow(std::size_t index) {
    assert(index < kWindowCount);
    return windows[index];
  }

  explicit FiniteDifferenceTester(std::size_t boxcar_length) : SecFiniteDif{boxcar_length} {}
};



const int boxcarLen = 500/40;
const int windowCount = 3;
TEST(FiniteDifferencer, Bootup) {

    //std::vector<int16_t> data;
    //data.reserve(1024);

    SecFiniteDif sdiff{boxcarLen};

    EXPECT_TRUE(sdiff.get_status() == BootStage::kWindow0);
    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(500);
    }

    EXPECT_TRUE(sdiff.get_status() == BootStage::kWindow1);

    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(500);
    }

    EXPECT_TRUE(sdiff.get_status() == BootStage::kWindow2);

    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(500);
    }
}


TEST(FiniteDifferencer, Reset) {
    FiniteDifferenceTester sdiff{boxcarLen};
    while (sdiff.get_status() != BootStage::kDone) {
        sdiff.run(100);
    }

    EXPECT_TRUE(sdiff.get_status() == BootStage::kDone);

    EXPECT_NE(sdiff.GetWindow(0), 0);
    EXPECT_NE(sdiff.GetWindow(1), 0);
    EXPECT_NE(sdiff.GetWindow(2), 0);
    EXPECT_EQ(sdiff.GetWindow(3), 0);

    sdiff.reset();
    EXPECT_TRUE(sdiff.get_status() == BootStage::kWindow0);
    EXPECT_EQ(sdiff.get_newest_boxcar(), 0);
    EXPECT_EQ(sdiff.get_sfdiff(), 0);

    EXPECT_EQ(sdiff.GetWindow(0), 0);
    EXPECT_EQ(sdiff.GetWindow(1), 0);
    EXPECT_EQ(sdiff.GetWindow(2), 0);
}

TEST(FiniteDifferencer, ConstValues) {
    FiniteDifferenceTester sdiff{boxcarLen};
    //  Test Const Value

    int16_t value = 500;
    for (int i = 0; i < 1024; i++) {
        sdiff.run(value);
    }

    EXPECT_EQ(sdiff.get_newest_boxcar(), value * boxcarLen);
    EXPECT_EQ(sdiff.get_sfdiff(), 0);
}

constexpr int GetAveIncreaseLineValue(int start, int finish, int averages) {
    return ((finish + start)*averages)/2;
}

constexpr int GetAveLineValue(int start, int step, int averages) {
    return start*averages + (step*averages)/2;
}

TEST(FiniteDifferencer, LinearValues) {
    //  Enter a line of slope step and offset 0
    constexpr int step = 4;
    const int datasize = 1024;
    const int start = step;

    constexpr int fWVal = GetAveIncreaseLineValue(start, boxcarLen*step, boxcarLen);
    constexpr int sWVal = GetAveIncreaseLineValue(start+boxcarLen*step, boxcarLen*step*2, boxcarLen);
    constexpr int tWVal = GetAveIncreaseLineValue(start+boxcarLen*step*2, boxcarLen*step*3, boxcarLen);

    FiniteDifferenceTester sdiff{boxcarLen};

    std::vector<int16_t> data;
    data.reserve(datasize);

    for (int i = 0; i < datasize; i++) {
      const auto value = static_cast<int16_t>((datasize - i)*step);
      data.push_back(value);  //  lowest value ready to be popped
    }

    EXPECT_EQ(data.back(), start);

    int value = 0;
    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(data.back());
        data.pop_back();
    }

    EXPECT_EQ(sdiff.GetWindow(0), fWVal);
    EXPECT_EQ(sdiff.GetWindow(1), 0);
    EXPECT_EQ(sdiff.GetWindow(2), 0);
    EXPECT_EQ(sdiff.GetWindow(3), 0);

    value += boxcarLen * step;
    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(data.back());
        data.pop_back();
    }

    EXPECT_EQ(sdiff.GetWindow(1), sWVal);
    EXPECT_EQ(sdiff.GetWindow(2), 0);
    EXPECT_EQ(sdiff.GetWindow(3), 0);

    value += boxcarLen * step;
    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(data.back());
        data.pop_back();
    }

    EXPECT_TRUE(sdiff.get_status() == BootStage::kDone);

    EXPECT_EQ(sdiff.GetWindow(3), 0);

    value += boxcarLen * step;

    EXPECT_EQ(sdiff.GetWindow(2), tWVal);
    EXPECT_EQ(tWVal, sdiff.get_newest_boxcar());
    EXPECT_EQ(sdiff.get_sfdiff(), 0);  //  Entering a line, so the second diff should be 0
}

constexpr int getparabola(int i, int gain, int offset) {
    return i*i*gain+offset;
}

TEST(FiniteDifferencer, ParabolicValues) {
    //  Enter a parabola gain*x**2 + offset
    constexpr int gain = 4;
    constexpr int offset = 140;

    FiniteDifferenceTester sdiff{boxcarLen};

    //  constexpr auto getparabola = [](int i) {return gain*i*i + offset;};
    static_assert(getparabola(0, gain, offset) == offset, "getparabola");
    static_assert(getparabola(1, gain, offset) == offset + gain, "getparabola");
    static_assert(getparabola(10, gain, offset) == offset + 100*gain, "getparabola");

    int i = 0;
    BootStage status = BootStage::kWindow0;
    int sum = 0;
    while (sdiff.get_status() != BootStage::kDone) {
        int value = getparabola(i++, gain, offset);
        sum += value;
        if (status != sdiff.get_status()) {
            status = sdiff.get_status();
            sum = value;
        }
        sdiff.run(value);
    }

    EXPECT_TRUE(sdiff.get_status() == BootStage::kDone);

    EXPECT_EQ(2*gain*boxcarLen*boxcarLen*boxcarLen, sdiff.get_sfdiff());  //  Entering a parabola, so the second diff should be the gain
}
