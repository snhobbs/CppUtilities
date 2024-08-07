#include <ArrayView/ArrayView.h>
#include <Utilities/CommonTypes.h>
#include <Utilities/TypeConversion.h>

#include <cstdint>

template <std::size_t kElements>
class BitField {
  static const std::size_t kByteSize = 8;
  static const constexpr std::size_t kLog2Of8 = 3;
  static_assert(1 << kLog2Of8 == 8);
  // static const constexpr std::size_t kDataBytes = kSize / 8 + 1;
  static const constexpr std::size_t kByteCount =
      Utilities::CalcNumberOfBytesToFitBits(kElements);

  std::array<uint8_t, kByteCount> data_store_
      __attribute__((aligned(sizeof(std::size_t))));

 public:
  static std::size_t size(void) { return GetSize(); }
  static constexpr std::size_t GetSize(void) { return kElements; }
  static constexpr std::size_t GetDataArrayIndex(std::size_t address) {
    return address >> kLog2Of8;
  }
  static constexpr std::size_t GetDataByteIndex(std::size_t address) {
    return static_cast<std::size_t>(address & ((1 << kLog2Of8) - 1));
  }
  static constexpr uint8_t GetByteMask(std::size_t address) {
    return static_cast<uint8_t>(1 << GetDataByteIndex(address));
  }
  bool ReadElement(uint16_t address) const {
    return data_store_[GetDataArrayIndex(address)] & GetByteMask(address);
  }

  void WriteElement(uint16_t address, bool state) {
    if (state) {
      data_store_[GetDataArrayIndex(address)] |= (GetByteMask(address));
    } else {
      data_store_[GetDataArrayIndex(address)] &= ~(GetByteMask(address));
    }
  }
  void ReadElementsToBytes(const uint16_t starting_address,
                           const uint16_t element_count,
                           ArrayView<uint8_t> *response_data) const {
    const std::size_t num_data_bytes =
        Utilities::CalcNumberOfBytesToFitBits(element_count);
    std::size_t address = starting_address;
    for (std::size_t byte_number = 0; byte_number < num_data_bytes;
         byte_number++) {
      uint8_t byte_value = 0;
      for (std::size_t byte_index = 0; byte_index < kByteSize; byte_index++) {
        const bool state = ReadElement(address++);
        if (state) {
          byte_value |= static_cast<uint8_t>(1 << byte_index);
        }
      }
      response_data->operator[](byte_number) = byte_value;
    }
  }
  void WriteMultipleElementsFromBytes(const uint16_t starting_address,
                                      const uint16_t element_count,
                                      const ArrayView<const uint8_t> data) {
    const std::size_t num_data_bytes =
        Utilities::CalcNumberOfBytesToFitBits(element_count);
    assert(data.size() >= num_data_bytes);
    std::size_t address = starting_address;
    for (std::size_t byte_number = 0; byte_number < num_data_bytes;
         byte_number++) {
      for (std::size_t byte_index = 0; byte_index < kByteSize; byte_index++) {
        const bool flag_value = (data[byte_number] & (1 << byte_index)) != 0;
        WriteElement(address++, flag_value);
      }
    }
  }
};
