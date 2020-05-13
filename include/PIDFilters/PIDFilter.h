/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * PIDFilter.h
 *
 *  Created on: Sep 21, 2018
 *      Author: simon
 */

#pragma once
#ifndef PIDFILTERS_PIDFILTER_H_
#define PIDFILTERS_PIDFILTER_H_

#include <Utilities/TypeConversion.h>
#include <cassert>
#include <cstdint>
#include <limits>

struct PIFilterCoeffs{
    int32_t shift_p;
    int32_t shift_i;
    int32_t mult_p;
    int32_t mult_i;
};

struct PI_Filter_Status{
    PIFilterCoeffs Coeffs;
    int32_t SetPoint{0};
    int32_t Integral{0};
    int32_t ProportionalError{0};
};

class IIR_PI_Filter{
private:
    PIFilterCoeffs coeffs_;

    int32_t integral_{0};
    int32_t x_{0}, x_m1_{0};
    int32_t control_{0};
    int32_t set_;

    int32_t MakeControl(void) const {
        //  Ki = mult_i>>shift_i
        //  Kp = mult_p>>shift_p
        //  control = Ki*i + Kp*p
        return MakeControl<int64_t>();
        //  return MakeControl();
    }

    template<typename T>
    int32_t MakeControl(void) const {
        //  Ki = mult_i>>shift_i
        //  Kp = mult_p>>shift_p
        //  control = Ki*i + Kp*p
        //  the 64 bit calculation only takes a few more cycles
        const T proportional = get_proportional();
        const T integral = GetIntegralValue();

        const T integral_multiply = static_cast<T>(integral)*coeffs_.mult_i;
        const T integral_term = (integral_multiply) >> coeffs_.shift_i;

        const T proportional_multiply = static_cast<T>(proportional)*coeffs_.mult_p;
        const T proportional_term = (proportional_multiply) >> coeffs_.shift_p;

        T result = -(integral_term + proportional_term);

        return Utilities::StaticCastQuickFail<int32_t>(result);
    }

    void MakeIntegral(void) {
        //  trapazoid rule
        //  this is 2x the integral
        const int64_t new_integral = get_integral_variable() + (static_cast<int64_t>(x_) + static_cast<int64_t>(x_m1_));
        if (new_integral > std::numeric_limits<int32_t>::max() || new_integral < std::numeric_limits<int32_t>::min()) {
            set_integral(0);  //  no divide by 2 until times T with T defined as one measurement time
        } else {
            set_integral(Utilities::StaticCastQuickFail<int32_t>(new_integral));
        }
    }

 protected:
    void set_integral(const int32_t integral) {
        integral_ = integral;
    }
    int32_t get_integral_variable(void) const {
        return integral_;
    }
    void set_control(const int32_t control) {
        control_ = control;
    }
    void MakeProportional(const int32_t process_variable) {
        x_m1_ = x_;
        x_ = process_variable - GetSetPoint();
    }

 public:
    int32_t get_control(void) const {
        return control_;
    }
    int32_t GetIntegralValue(void) const {
        //  integral divided by 2 when being used so as not to compound errors
        return (get_integral_variable())/2;
    }
    int32_t get_proportional(void) const {
        return x_;
    }

    int32_t run(const int32_t process_variable) {
        MakeProportional(process_variable);
        MakeIntegral();
        set_control(MakeControl());
        return get_control();
    }
    int32_t RunProportional(const int32_t process_variable) {
        //  Keeps integral as whatever it currently is
        MakeProportional(process_variable);
        set_control(MakeControl());
        return get_control();
    }
    void SetSetPoint(const int32_t set) {
        set_ = set;
        Clear();
    }
    int32_t GetSetPoint(void) const {
        return set_;
    }

    //  Clears error and integral
    void Clear(void) {
        set_integral(0);
        set_control(0);
        x_ = 0;
        x_m1_ = 0;
    }
    void SetFilterCoefficients(const PIFilterCoeffs& coeffs) {
        coeffs_ = coeffs;
    }
    void GetFilterStatus(PI_Filter_Status& status) const {
        status.Coeffs.shift_p = coeffs_.shift_p;
        status.Coeffs.shift_i = coeffs_.shift_i;
        status.Coeffs.mult_p = coeffs_.mult_p;
        status.Coeffs.mult_i = coeffs_.mult_i;

        status.Integral = get_integral_variable();
        status.ProportionalError = get_proportional();
        status.SetPoint = GetSetPoint();
    }
    IIR_PI_Filter(const int32_t set, const PIFilterCoeffs& coeffs): coeffs_{coeffs}, set_{set} {
        assert(coeffs_.mult_p != 0);
        assert(coeffs_.mult_i != 0);
    }
};

#endif  //  PIDFILTERS_PIDFILTER_H_
