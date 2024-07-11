/*
 * DataLoader.h
 *
 *  Created on: Jun 26, 2018
 *      Author: simon
 */

#ifndef INC_DATALOADER_H_
#define INC_DATALOADER_H_

#include <stdint.h>

#include <fstream>
#include <string>
#include <vector>

const unsigned int adcBits{14};
const int vectorsize = (1 << adcBits);

class DataLoader {
  // Take a file containing uint16_ts in flat binary format
  // For loading into detection algorithms
  std::ifstream din;
  int fsize{0};  // data measurements in file
 public:
  std::vector<int16_t> dstream{};

  static int16_t signExtend(const unsigned int& bits, const int16_t& val) {
    int16_t out = val;
    if (val & (1 << (bits - 1))) {
      out = static_cast<int16_t>((val & ((1 << (bits - 1)) - 1)) -
                                 (1 << (bits - 1)));
    }
    return out;
  }
  static int16_t makeData(const uint8_t top, const uint8_t bottom) {
    return signExtend(adcBits, static_cast<int16_t>(top << 8 | bottom));
  }

 public:
  void load(void) {
    for (auto i = 0; i < fsize; i++) {
      char bottom = static_cast<uint8_t>(din.get());
      char top = static_cast<uint8_t>(din.get());

      dstream.push_back(makeData(top, bottom));
    }
  }
  void set_din(const std::string& fname, const int fileSize) {
    if (din.is_open()) {
      din.close();
    }
    din.open(fname, std::ifstream::in);
    fsize = fileSize;
    dstream.reserve(fileSize);
  }

  DataLoader(const std::string& fname, const int fileSize) {
    set_din(fname, fileSize);
  }
  DataLoader() = default;
  ~DataLoader() { din.close(); }
};

#endif /* INC_DATALOADER_H_ */
