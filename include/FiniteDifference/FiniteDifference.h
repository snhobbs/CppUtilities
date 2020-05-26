/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * FiniteDifference.h
 *
 *  Created on: Nov 15, 2018
 *      Author: simon
 */

#pragma once
#ifndef FINITEDIFFERENCE_FINITEDIFFERENCE_H_
#define FINITEDIFFERENCE_FINITEDIFFERENCE_H_
#include <array>

enum class BootStage{
    kWindow0 = 0,
    kWindow1 = 1,
    kWindow2 = 2,
    kDone = 3
};

class SecFiniteDif{
    static const int kSecondDerivOrder = 2;
    static const constexpr int kWindowCount = kSecondDerivOrder+2;
    const uint32_t boxcar_length_;

    uint8_t newest_boxcar = 0;
    int32_t last_value = 0;
    unsigned int average_count{0};
    BootStage boot_stage{BootStage::kWindow0};  //  At first bootup the windows need to all be filled

 protected:
    std::array<int32_t, kWindowCount> windows{};

    void bootup(const int32_t data) {
        /*
         * During bootup fill all the windows before proceeding
         * */
        switch (boot_stage) {
            case(BootStage::kDone): {
                break;
            }
            case(BootStage::kWindow0): {
                windows[0] += data;
                if (!(average_count < boxcar_length_)) {
                    boot_stage = BootStage::kWindow1;
                    average_count = 0;
                }
                break;
            }
            case(BootStage::kWindow1): {
                windows[1] += data;
                if (!(average_count < boxcar_length_)) {
                    boot_stage = BootStage::kWindow2;
                    average_count = 0;
                    newest_boxcar++;
                }
                break;
            }
            case(BootStage::kWindow2): {
                windows[2] += data;
                if (!(average_count < boxcar_length_)) {
                    boot_stage = BootStage::kDone;
                    average_count = 0;
                    newest_boxcar++;
                }
                break;
            }
        }
    }

    void rotateWindows(void) {
        //  window 0 is the oldest, window 3 is being filled, window 2 is the newest completed
        windows[0] = windows[1];
        windows[1] = windows[2];
        windows[2] = windows[3];
        windows[3] = 0;
        average_count = 0;
        static_assert(kWindowCount == 4, "Rotate Windows is wrong");
    }

 public:
    BootStage get_status(void) const {
        return boot_stage;
    }

    void reset(void) {
        average_count = 0;
        boot_stage = BootStage::kWindow0;  //  At first bootup the windows need to all be filled
        for (auto& window : windows) {
            window = 0;
        }
    }
    void run(const int32_t data) {
        last_value = data;
        average_count++;

        if (boot_stage == BootStage::kDone) {  //  We're all setup to run the algo
            windows[3] += data;
            if (average_count >= boxcar_length_) {  //  rotate windows and evaluate algo
                rotateWindows();
            }
        } else {  //  continue with bootup setup
            bootup(data);
        }
    }
    int32_t get_newest_boxcar(void) const {
        return windows[newest_boxcar];
    }
    int32_t get_last_value(void) const {
        return last_value;
    }
    int32_t get_sfdiff(void) const {
        //  The actual sfdiff is this value divided by the (boxcar length)**3 as we need to take the average and then divide by the step**2
        if (boot_stage == BootStage::kDone) {
            return windows[2]-2*windows[1]+windows[0];
        } else {
            return 0;
        }
    }
    explicit SecFiniteDif(const uint32_t box_car_length = 16) :
        boxcar_length_{box_car_length} {}
};




#endif  //  FINITEDIFFERENCE_FINITEDIFFERENCE_H_
