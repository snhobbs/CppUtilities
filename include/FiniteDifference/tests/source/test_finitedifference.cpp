#include <vector>
#include <iostream>
#include "UnitTest++/UnitTest++.h"
#define private public
#include <FiniteDifference/FiniteDifference.h>

const int boxcarLen = 500/40;
const int windowCount = 3;
TEST(Bootup) {

    //std::vector<int16_t> data;
    //data.reserve(1024);

    SecFiniteDif sdiff{boxcarLen};

    CHECK(sdiff.get_status() == BootStage::kWindow0);
    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(500);
    }

    CHECK(sdiff.get_status() == BootStage::kWindow1);

    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(500);
    }

    CHECK(sdiff.get_status() == BootStage::kWindow2);

    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(500);
    }
}

TEST(Reset) {
    SecFiniteDif sdiff{boxcarLen};
    while (sdiff.get_status() != BootStage::kDone) {
        sdiff.run(100);
    }

    CHECK(sdiff.get_status() == BootStage::kDone);

    CHECK(sdiff.windows[0] != 0);
    CHECK(sdiff.windows[1] != 0);
    CHECK(sdiff.windows[2] != 0);
    CHECK_EQUAL(sdiff.windows[3], 0);


    sdiff.reset();
    CHECK(sdiff.get_status() == BootStage::kWindow0);
    CHECK_EQUAL(sdiff.get_newest_boxcar(), 0);
    CHECK_EQUAL(sdiff.get_sfdiff(), 0);

    CHECK_EQUAL(sdiff.windows[0], 0);
    CHECK_EQUAL(sdiff.windows[1], 0);
    CHECK_EQUAL(sdiff.windows[2], 0);
}

TEST(ConstValues) {
    SecFiniteDif sdiff{boxcarLen};
    //  Test Const Value

    int16_t value = 500;
    for (int i = 0; i < 1024; i++) {
        sdiff.run(value);
    }

    CHECK_EQUAL(sdiff.get_newest_boxcar(), value * boxcarLen);
    CHECK_EQUAL(sdiff.get_sfdiff(), 0);
}

constexpr int GetAveIncreaseLineValue(int start, int finish, int averages) {
    return ((finish + start)*averages)/2;
}

constexpr int GetAveLineValue(int start, int step, int averages) {
    return start*averages + (step*averages)/2;
}

TEST(LinearValues) {
    //  Enter a line of slope step and offset 0
    constexpr int step = 4;
    const int datasize = 1024;
    const int start = step;

    constexpr int fWVal = GetAveIncreaseLineValue(start, boxcarLen*step, boxcarLen);
    constexpr int sWVal = GetAveIncreaseLineValue(start+boxcarLen*step, boxcarLen*step*2, boxcarLen);
    constexpr int tWVal = GetAveIncreaseLineValue(start+boxcarLen*step*2, boxcarLen*step*3, boxcarLen);


    SecFiniteDif sdiff{boxcarLen};

    std::vector<int16_t> data;
    data.reserve(datasize);

    for (int i = 0; i < datasize; i++) {
      const auto value = static_cast<int16_t>((datasize - i)*step);
      data.push_back(value);  //  lowest value ready to be popped
    }

    CHECK_EQUAL(data.back(), start);

    int value = 0;
    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(data.back());
        data.pop_back();
    }

    CHECK_EQUAL(fWVal, sdiff.windows[0]);
    CHECK_EQUAL(0, sdiff.windows[1]);
    CHECK_EQUAL(0, sdiff.windows[2]);
    CHECK_EQUAL(0, sdiff.windows[3]);


    value += boxcarLen * step;
    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(data.back());
        data.pop_back();
    }

    CHECK_EQUAL(sWVal, sdiff.windows[1]);
    CHECK_EQUAL(0, sdiff.windows[2]);
    CHECK_EQUAL(0, sdiff.windows[3]);

    value += boxcarLen * step;
    for (int i = 0; i < boxcarLen; i++) {
        sdiff.run(data.back());
        data.pop_back();
    }

    CHECK(sdiff.get_status() == BootStage::kDone);
    CHECK_EQUAL(0, sdiff.windows[3]);

    value += boxcarLen * step;

    CHECK_EQUAL(tWVal, sdiff.windows[2]);
    CHECK_EQUAL(tWVal, sdiff.get_newest_boxcar());
    CHECK_EQUAL(sdiff.get_sfdiff(), 0);  //  Entering a line, so the second diff should be 0
}

constexpr int getparabola(int i, int gain, int offset) {
    return i*i*gain+offset;
}

TEST(ParabolicValues) {
    //  Enter a parabola gain*x**2 + offset
    constexpr int gain = 4;
    constexpr int offset = 140;

    SecFiniteDif sdiff{boxcarLen};

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

    CHECK(sdiff.get_status() == BootStage::kDone);

    CHECK_EQUAL(2*gain*boxcarLen*boxcarLen*boxcarLen, sdiff.get_sfdiff());  //  Entering a parabola, so the second diff should be the gain
}
