/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * */
#pragma once
#ifndef UTILITIES_CRC_H_
#define UTILITIES_CRC_H_
#include <array>
#include <cstdint>

namespace Utilities {

/*
 * lrc := 0
for each byte b in the buffer do
    lrc := (lrc + b) and 0xFF
lrc := (((lrc XOR 0xFF) + 1) and 0xFF)
 *
 * */
inline constexpr uint8_t LinearRedundancyCheck(const uint8_t *const buffer,
                                               const std::size_t data_length) {
  const uint8_t kMask = 0xff;
  uint8_t lrc = 0;
  for (std::size_t current_byte = 0; current_byte < data_length; current_byte++) {
    lrc = static_cast<uint16_t>((lrc + buffer[current_byte]) & kMask);
    lrc = static_cast<uint16_t>(((lrc ^ 0xFF) + 1) & kMask);
  }
  return lrc;
}

inline constexpr uint16_t crc16(const uint8_t *const buffer,
                                const std::size_t data_length) {
  const constexpr std::size_t crc16_polynomial = 0xA001;
  uint16_t crc = 0xffff;
  for (std::size_t current_byte = 0; current_byte < data_length; current_byte++) {
    crc = static_cast<uint16_t>(crc ^ buffer[current_byte]);
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = static_cast<uint16_t>((crc >> 1) ^ crc16_polynomial);
      } else {
        crc = static_cast<uint16_t>(crc >> 1);
      }
    }
  }
  return static_cast<uint16_t>(crc << 8 | crc >> 8);
}

template<typename T>
inline uint16_t crc16(const T &array, std::size_t length) {
  assert(length <= array.size());
  return Utilities::crc16(array.data(), length);
}

inline constexpr uint8_t crc_uint8(const uint8_t *const buffer,
                              const std::size_t data_length,
                              const uint8_t crc_polynomial = 0xff,
                              const uint8_t crc_initial_value = 0xff) {
  uint8_t crc = crc_initial_value;
  /* calculates 8-Bit checksum with given polynomial */
  for (std::size_t current_byte = 0; current_byte < data_length; current_byte++) {
    crc ^= (buffer[current_byte]);
    for (std::size_t crc_bit = 8; crc_bit > 0; --crc_bit) {
      if (crc & 0x80) {
        const auto crc_shift = static_cast<uint8_t>(crc << 1);
        crc = (crc_shift) ^ crc_polynomial;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

inline constexpr uint8_t crc8(const uint8_t *const buffer,
                              const std::size_t data_length,
                              const uint8_t crc_initial_value = 0xff) {
  const uint8_t crc8_polynomial = 0x31;  //  (x8 + x5 + x4 + 1)
  return crc_uint8(buffer, data_length, crc8_polynomial, crc_initial_value);
}

static_assert(crc8(std::array<uint8_t, 2>{0xBE, 0xEF}.data(), 2) == 0x92);
static_assert(crc8(std::array<uint8_t, 1>{0x00}.data(), 1) == 0xAC);


inline constexpr uint8_t crc4_new(const uint8_t *const buffer,
                              const std::size_t data_length,
                              const uint8_t crc_initial_value = 0) {
  const constexpr uint8_t crc4_polynomial = 0x03;
  const uint8_t crc = crc_uint8(buffer, data_length, crc4_polynomial, crc_initial_value);
  const uint8_t last4_bits = (0x000F & (crc >> 4)); // final 4-bit reminder is CRC code
  return 0xf & (last4_bits ^ 0x0);
}

inline constexpr uint8_t crc4(const uint8_t *const buffer,
                              const std::size_t data_length) {
  const constexpr uint16_t crc4_polynomial = 0x3000;
  uint16_t n_rem = 0; //  crc reminder

  for (std::size_t current_byte = 0; current_byte < data_length; current_byte++) {
    n_rem ^= buffer[current_byte];
    for (std::size_t n_bit = 8; n_bit > 0; n_bit--) {
      if (n_rem & (0x8000)) {
        n_rem = (n_rem << 1) ^ crc4_polynomial;
      } else {
        n_rem = (n_rem << 1);
      }
    }
  }
  n_rem = (0x000F & (n_rem >> 12)); // final 4-bit reminder is CRC code
  return 0xf & (n_rem ^ 0x0);
}
} //  namespace Utilities

#endif //  UTILITIES_CRC_H_
