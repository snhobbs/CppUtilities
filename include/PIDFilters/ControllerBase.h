/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * */
#pragma once
#ifndef PIDFILTERS_CONTROLLERBASE_H_
#define PIDFILTERS_CONTROLLERBASE_H_

#include <PIDFilters/PIDFilter.h>
#include <cstdint>

struct FilterLimits{
    const int32_t low;
    const int32_t high;

    constexpr FilterLimits(const int32_t limit_low, const int32_t limit_high) : low{limit_low}, high{limit_high}{
        assert(low <= high);
    }
};

class ControllerBase{
 public:
    enum class State{
        kBootup,
        kNormal,
        kCurrentLimit,
        kSoftwareLimit,
    };

 private:
    State state_{State::kBootup};

 protected:
    IIR_PI_Filter filter_;
    const FilterLimits filter_limits_;

    void SetState(const State state) {
        state_ = state;
    }

 public:
    int32_t GetSetPoint(void) const {
        return filter_.GetSetPoint();
    }
    void Reset(void) {
        filter_.Clear();
        SetState(State::kBootup);
    }

    void SetFilterCoefficients(const PIFilterCoeffs& coeffs) {
        filter_.SetFilterCoefficients(coeffs);
    }

    void GetFilterStatus(PI_Filter_Status& status) const {
        filter_.GetFilterStatus(status);
    }

    void SetSetPoint(const int32_t set_point) {
        filter_.SetSetPoint(set_point);
    }

    int32_t RunFilter(const int32_t process_variable, const bool current_limit) {
        if (current_limit) {
            SetState(State::kCurrentLimit);
        }

        int32_t control = 0;
        switch (state_) {
            case(State::kNormal): {
                control = filter_.run(process_variable);
                break;
            }
            case(State::kSoftwareLimit): {
                control = filter_.RunProportional(process_variable);  //  Don't alter integral, prevents windup
                break;
            }
            case(State::kCurrentLimit): {
                control = filter_.RunProportional(process_variable);  //  Don't alter integral, prevents windup
                break;
            }
            case(State::kBootup): {
                //  control = filter_.RunProportional(processVar);  //  Primes filter
                control = filter_.run(process_variable);
                break;
            }
            default:
                break;
        }

        if (control > filter_limits_.high) {
            control = filter_limits_.high;
            SetState(State::kSoftwareLimit);
        } else if (control < filter_limits_.low) {
            control = filter_limits_.low;
            SetState(State::kSoftwareLimit);
        } else {
            SetState(State::kNormal);
        }
        return control;
    }

    ControllerBase(const int32_t set_point, const PIFilterCoeffs& PIFilterCoeffs, const FilterLimits& lims) : 
        filter_{set_point, PIFilterCoeffs}, filter_limits_{lims}{
        assert(lims.low <= lims.high);
    }
    //  virtual ~ControllerBase(void) {}
};


#endif  //  PIDFILTERS_CONTROLLERBASE_H_
