/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 *
 * */
#pragma once
#ifndef CALCULATORS_CALCULATORBASE_H_
#define CALCULATORS_CALCULATORBASE_H_
#include <Utilities/TypeConversion.h>
#include <Utilities/math.h>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

/**
 * Collection of the basic calculations needed for an embedded system
 *
 *
 */
namespace Utilities {
template <typename T>
inline constexpr T TranslateToMicro(const T value) {
  const T kMicroScaleFactor = 1000 * 1000;
  return Utilities::round(value * kMicroScaleFactor);
}
/**
 * return a multiplier, shift, and micro error as an approximation of the
 * value value approx= m>>s
 */
template <typename T>
struct MultiplyShiftEstimateResult {
  const T multiplier;
  const T shift;
  const T error;
  constexpr MultiplyShiftEstimateResult(const T multiplier_in, const T shift_in,
                                        const T error_in)
      : multiplier{multiplier_in}, shift{shift_in}, error{error_in} {}
};

template <typename T>
inline constexpr MultiplyShiftEstimateResult<T> MultiplyShiftEstimate(
    const double value, const std::size_t multiplier_max,
    const std::size_t shift_max) {
  const auto abs_value = Utilities::abs(value);
  const auto dmultiplier_max = static_cast<double>(multiplier_max);
  const T kMicroScaleFactor = 1000 * 1000;
  T mult = 0;
  std::size_t shift = 0;
  double last_diff = kMicroScaleFactor * abs_value;
  for (std::size_t i = 0; i < shift_max; i++) {
    const double value_shift =
        Utilities::round(abs_value * Utilities::pow(2, i));
    const double approx_value =
        value_shift * Utilities::pow(2, -static_cast<double>(i));
    const double diff =
        kMicroScaleFactor * Utilities::abs(abs_value - approx_value);
    if (value_shift > dmultiplier_max) {
      break;
    } else if (diff < last_diff) {
      mult = static_cast<int32_t>(Utilities::round(value_shift));
      shift = i;
      last_diff = diff;
    }
  }
  const auto error = last_diff;
  const auto cast_error = Utilities::StaticCastQuickFail<T>(error);
  if (value != abs_value) {
    mult *= -1;
  }
  return MultiplyShiftEstimateResult<T>{mult, static_cast<T>(shift),
                                        cast_error};
}
}  // namespace Utilities

template <typename Output>
class Calculator {
 public:
  static const constexpr Output kMicroScaleFactor = 1000 * 1000;
  static const constexpr Output kMilliScaleFactor = 1000;

  /**
   * return a multiplier, shift, and micro error as an approximation of the
   * value value approx= m>>s
   */
  [[deprecated]] static constexpr std::tuple<Output, Output, Output>
  MultiplyShiftEstimate(double value, double multiplier_max,
                        const int32_t shift_max) {
    const auto result = Utilities::MultiplyShiftEstimate<Output>(
        value, multiplier_max, shift_max);
    return std::tuple<Output, Output, Output>{result.multiplier, result.shift,
                                              result.error};
  }

  template <typename T>
  static constexpr Output TranslateToMicro(const T value) {
    return static_cast<Output>(Utilities::round(value * kMicroScaleFactor));
  }

  template <typename T>
  static constexpr Output TranslateToMilli(const T value) {
    return static_cast<Output>(Utilities::round(value * kMilliScaleFactor));
  }

  /**
   * Change a value on a binary scale to its equivilent on another binary scale
   */
  static constexpr Output TranslateBitScale(const Output value,
                                            const int bits_of_value,
                                            const int bits_of_output) {
    const unsigned int kShift = Utilities::abs(bits_of_value - bits_of_output);
    Output out = 0;
    if (bits_of_value > bits_of_output) {
      out = (value + (1 << (kShift - 1))) >> (kShift);
    } else {
      out = value << kShift;
    }
    return out;
  }

  /**
   * Turn a binary scale into an arbitrary scale, useful for turning a digital
   * value into a voltage such as turing a DAC output into a voltage
   */
  template <typename Input>
  static constexpr Output ScaleDigitalValue(const Input value,
                                            const int bits_of_value,
                                            const Input scale_low,
                                            const Input scale_high) {
    assert(scale_high > scale_low);
    if (bits_of_value <= 0) {
      assert(0);  //  invalid
      return 0;
    }
    const auto output_range = scale_high - scale_low;

    const auto round_off = 1 << (bits_of_value - 1);
    const auto result =
        ((value * output_range + round_off) >> bits_of_value) + scale_low;
    //  const auto result = ((value * output_range) >> bits_of_value) +
    //  scale_low; const auto result = ((value * output_range + round_off) *
    //  pow(2, -bits_of_value)) + scale_low;
    return Utilities::StaticCastQuickFail<Output>(result);
  }

  /**
   * Turn an arbitrary scale into a binary scale, useful for turning a voltage
   * into a digital value such as an ADC value or a DAC output
   */
  template <typename Input>
  static constexpr Output ScaleToDigitalValue(const Input value,
                                              const int bits_of_output,
                                              const Input scale_low,
                                              const Input scale_high) {
    assert(scale_high > scale_low);
    if (bits_of_output <= 0) {
      assert(0);  //  invalid
      return 0;
    }
    const auto output_range = scale_high - scale_low;

    const auto round_off = (output_range + 1) / 2;
    //  const auto numerator = (value - scale_low) * pow(2, bits_of_output);
    assert(value >= scale_low);
    const auto numerator = (value - scale_low) << bits_of_output;
    const auto out = (numerator + round_off) / output_range;

    assert(out >= 0);
    return Utilities::StaticCastQuickFail<Output>(out);
  }

  /**
   * Calculate the potential of one input into a voltage divider given the node
   * value, one potential and both resistors
   */
  template <typename Input>
  static constexpr Output TwoNodeVoltageDividerReverse(const Input v1,
                                                       const Input v_node,
                                                       const Input r1,
                                                       const Input r2) {
    const auto Positive = (v_node * (r1 + r2)) / r1;
    const auto Negative = (v1 * r2) / r1;
    assert(std::numeric_limits<Input>::is_signed || (Positive >= Negative));
    const auto v2 = Positive - Negative;
    return Utilities::StaticCastQuickFail<Output>(v2);
  }

  /**
   * Calculate the value at the center node of two resistors connected to two
   * potentials
   */
  template <typename Input>
  static constexpr Output TwoNodeVoltageDivider(const Input v1, const Input v2,
                                                const Input r1,
                                                const Input r2) {
    //  fixme find a common divider of r1 and r2?
    const auto v_node = (v2 * r1 + v1 * r2) / (r1 + r2);
    return Utilities::StaticCastQuickFail<Output>(v_node);
  }

  /**
   * Calculate the value at the center node of two resistors connected to two
   * potentials where the difference between the node and an output is fixed (v2
   * - vnode = vp2_node)
   */
  template <typename Input>
  static constexpr Output TwoNodeVoltageDividerReverseFeedback(
      const Input v1, const Input vp2_node, const Input r1,
      const Input rfeedback) {
    /*
     * v2 - vnode = vp2_node
     * (v2 - vnode)/rfeedback + (v1 - vnode)/r1 = 0
     * vp2_node/rfeedback + (v1 - vnode)/r1 = 0
     * vnode = r1*vp2_node/rfeedback + v1
     * v2 = r1*vp2_node/rfeedback + v1 + vp2_node
     * v2 = vp2_node * (r1 + rfeedback)/rfeedback + v1
     */
    const auto v2 = (vp2_node * (r1 + rfeedback)) / rfeedback + v1;
    return Utilities::StaticCastQuickFail<Output>(v2);
  }

  /**
   * Calculate center node value of a network of three resistors and three
   * potentials
   */
  //  fixme add unsigned bit scaled function
  template <typename Input>
  static constexpr Output ThreeNodeVoltageDivider(
      const Input v1, const Input v2, const Input v3, const Input r1,
      const Input r2, const Input r3) {
    const auto sum = r2 * r3 * v1 + r1 * r3 * v2 + r1 * r2 * v3;
    const auto divider = (r1 * r2 + r1 * r3 + r2 * r3);
    const auto result = sum / divider;
    return Utilities::StaticCastQuickFail<Output>(result);
  }

  /**
   * Calculate one potential value from the center node value, two potentials
   * and the three resistors
   */
  template <typename Input>
  static constexpr Output ThreeNodeVoltageDividerReversed(
      const Input v1, const Input v2, const Input node, const Input r1,
      const Input r2, const Input r3) {
    const auto divider = r1 * (r2 + r3) + r2 * r3;
    const auto sum = node * divider;
    const auto numerator_negative = r3 * (r2 * v1 + r1 * v2);

    assert(std::numeric_limits<Output>::is_signed ||
           (sum >= numerator_negative));

    const auto v3_numerator = sum - numerator_negative;
    const auto v3_denominator = r1 * r2;
    const auto result = v3_numerator / v3_denominator;
    return Utilities::StaticCastQuickFail<Output>(result);
  }

#if 0
    template<typename Input>
    static constexpr Output ThreeNodeVariablePowerSupply(
        const Input node_value, const Input node_a_value,
        const Input node_a_resistor, const Input node_b_value,
        const Input node_b_resistor, const Input output_fb_resistor) {
        //  Calculate the signal in voltage as a fuction of the
        //  center of a three node voltage divider
        /*
           (nav - nv)/ra + (nbv - nv)/rb + (out - nv)/ofbr = 0
           out = ofbr((nv - nav)/ra + (nv - nbv)/rb) + nv
           out = ofbr(nv/ra - nav/ra + nv/rb - nvb/rb + nv/ofbr)
           out = nv(ofbr/ra + ofbr/rb + 1) - ofbr*(nav/ra + nvb/rb)
           out = ofbr/(ra*rb) * (nv*((ra*rb) + (ra + rb))) - (nav*rb + nvb*ra))


         */

        Output nv = node_value;
        Output ra = node_a_resistor;
        Output nav = node_a_value;
        Output rb = node_b_resistor;
        Output nbv = node_b_value;
        Output ofbr = output_fb_resistor;

        //  return nv*(ofbr/ra + ofbr/rb + 1) - ofbr*(nav/ra + nbv/rb);
        //  return ofbr*(nv-nav)/ra+ ofbr*(nv-nbv)/rb + nv;

        //  return (rb*ofbr*(nv-nav))/(ra*rb)+ (ra*ofbr*(nv-nbv))/(rb*ra) + nv;
        //  return ((rb*ofbr*(nv-nav)) + (ra*ofbr*(nv-nbv)))/(rb*ra) + (nv);
        //  return ((rb*ofbr*(nv-nav)) + (ra*ofbr*(nv-nbv)))/(ra*rb) + (nv);

        //  ComputationType a = (rb*ofbr*(nv-nav));
        Output a_pos = (rb*ofbr*(nv));
        Output a_neg = (rb*ofbr*(nav));
        //  ComputationType b = (ra*ofbr*(nv-nbv));
        Output b_pos = (ra*ofbr*(nv));
        Output b_neg = (ra*ofbr*(nbv));
        Output c = (nv)*(ra*rb);
        Output Positive = a_pos + b_pos + c;
        Output Negative = a_neg + b_neg;


        return (Positive - Negative)/(ra*rb);
    }
#endif

  /**
   * Calculate the expected output of an negative feedback amplifier. The vhigh
   * and vlow values are used to emulate the amplifier pegging
   */
  template <typename Input>
  static constexpr Output AmplifierOutput(const Input noninverting_value,
                                          const Input inverting_node_value,
                                          const Input inverting_node_resistor,
                                          const Input fb_resistor,
                                          const Input vhigh,
                                          const Input vlow = 0) {
    const auto Negative =
        (inverting_node_value * fb_resistor) / inverting_node_resistor;
    const auto Positive =
        (noninverting_value * fb_resistor) / inverting_node_resistor +
        noninverting_value;

    // Output Positive = (noninverting_value*(fb_resistor +
    // inverting_node_resistor))/inverting_node_resistor;

    assert(std::numeric_limits<Output>::is_signed || (Positive >= Negative));

    const auto out = Positive - Negative;
    const auto result = (out > vhigh) ? vhigh : (out < vlow ? vlow : out);
    return Utilities::StaticCastQuickFail<Output>(result);
  }

  /**
   * Calculate the noninverting input of a negative feedback amplifier from the
   * output, inverting input, input resistor, and feedback resistor
   */
  template <typename Input>
  static constexpr Output CalculateNoneInvertingInputFromAmplifierOutput(
      const Input inverting_node_value, const Input output_value,
      const Input inverting_node_resistor, const Input fb_resistor) {
    const auto numerator = (inverting_node_value * fb_resistor +
                            (output_value * inverting_node_resistor));
    const auto denominator = (fb_resistor + inverting_node_resistor);

    const auto noninverting_node_value = numerator / denominator;
    return Utilities::StaticCastQuickFail<Output>(noninverting_node_value);
  }

  /**
   * Calculate the inverting input of a negative feedback amplifier from the
   * output, noninverting input, input resistor, and feedback resistor
   */
  template <typename Input>
  static constexpr Output CalculateInvertingInputFromAmplifierOutput(
      const Input noninverting_node_value, const Input output_value,
      const Input inverting_node_resistor, const Input fb_resistor) {
    const auto Negative =
        (output_value * inverting_node_resistor) / fb_resistor;
    const auto Positive =
        (noninverting_node_value * (fb_resistor + inverting_node_resistor)) /
        fb_resistor;

    assert(std::numeric_limits<Output>::is_signed || (Positive >= Negative));

    const auto inverting_node_value = Positive - Negative;
    return Utilities::StaticCastQuickFail<Output>(inverting_node_value);
  }

  template <typename Input>
  static constexpr Output NonInvertingAmplifierGain(
      const Input inverting_node_resistor, const Input fb_resistor) {
    const auto numerator = (fb_resistor + inverting_node_resistor);
    const auto out = numerator / inverting_node_resistor;
    return Utilities::StaticCastQuickFail<Output>(out);
  }

  template <typename Input>
  static constexpr Output InvertingAmplifierGain(const Input resistor_in,
                                                 const Input fb_resistor) {
    assert(std::numeric_limits<Output>::is_signed);
    const auto out = -fb_resistor / resistor_in;
    return Utilities::StaticCastQuickFail<Output>(out);
  }
};
/*
static_assert(Calculator<uint64_t>::TranslateToMicro(3.3) ==
(uint64_t)(33*100*1000), "");
static_assert(Calculator<uint64_t>::TranslateToMicro(0) == 0, "");

static_assert(Calculator<uint64_t>::TranslateBitScale(103, 8, 6) == 26, "");
static_assert(Calculator<uint64_t>::TranslateBitScale(102, 8, 6) == 26, "");
static_assert(Calculator<uint64_t>::TranslateBitScale(101, 8, 6) == 25, "");
static_assert(Calculator<uint64_t>::TranslateBitScale(100, 8, 6) == 25, "");
static_assert(Calculator<uint64_t>::TranslateBitScale(101, 6, 8) == 404, "");
static_assert(Calculator<uint64_t>::ThreeNodeVoltageDivider<uint64_t, uint32_t,
double>(0.5*1000*1000, 0, 1e3, 1, 1e3, 1e3, 1, 1000*1000) == 0.75*1000*1000,
""); static_assert(ThreeNodeVoltageDivider<uint64_t, uint32_t,
double>(2.3*1000*1000, 0, 1e3, pull_up_value, const Input pull_up_resistor,
const Input input_resistor, const Output kScale = 1, const Output kValueScale =
1);
*/

#endif  //  CALCULATORS_CALCULATORBASE_H_
