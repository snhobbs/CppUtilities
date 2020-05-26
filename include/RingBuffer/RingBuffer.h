/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * */
#pragma once
#ifndef RINGBUFFER_RINGBUFFER_H_
#define RINGBUFFER_RINGBUFFER_H_

#define ALIGNED(x) __attribute__((aligned(x)))
#include <ArrayView/ArrayView.h>
#include <array>
#include <assert.h>

template <typename T> class Buffer {
 public:
  virtual bool isEmpty(void) const = 0;
  virtual bool isFull(void) const = 0;
  virtual std::size_t pop(T *const out, const std::size_t count) = 0;
  virtual std::size_t insert(const T *const in, const std::size_t count) = 0;
  //  virtual std::size_t insert(const T& in) = 0;
  //  virtual std::size_t insert(T* const in, std::size_t count) {
  //  insert_const(in, count);
  virtual void Reset(void) = 0;
  virtual std::size_t GetCount(void) const = 0;
  virtual std::size_t Size(void) const = 0;

  Buffer(void) {}
  virtual ~Buffer(void) {}
};

template <typename T, std::size_t kElements>
class RingBuffer final : public Buffer<T> {
 private:
  static_assert((kElements > 0),
                "Size must be greater than 0 and a power of 2");
  static_assert(!(kElements & (kElements - 1)), "Size must be power of 2");
  //  static_assert(!(kElements*sizeof(T) % 4), "Size must be evenly distributed
  //  into 32bits to ensure alignment");
  std::array<T, kElements> buffer_ ALIGNED(32){};
  ArrayView<T> data{buffer_.size(), buffer_.data()};

  uint32_t head = 0;
  uint32_t tail = 0;

 protected:
  static uint32_t MaskIndex(const uint32_t Index) {
    uint32_t out = Index & (kElements - 1);
    return out;
  }

 public:
  uint32_t GetTail(void) const { return MaskIndex(tail); }
  uint32_t GetHead(void) const { return MaskIndex(head); }

  virtual bool isEmpty(void) const { return head == tail; }
  virtual bool isFull(void) const {
    return !isEmpty() && (GetHead() == GetTail());
  }

  virtual std::size_t pop(T *out) {
    if (!isEmpty()) {
      *out = data[GetTail()];
      tail += 1;
      return 1;
    }
    return 0;
  }

  [[deprecated]] virtual std::size_t pop(T &out) { return pop(&out); }

  virtual std::size_t pop(T *const out, const std::size_t count) {
    std::size_t popped = 0;
    for (std::size_t i = 0; i < count; i++) {
      popped += pop(&out[i]);
    }
    return popped;
  }

  virtual std::size_t insert(const T &in) {
    if (!isFull()) {
      data[GetHead()] = in;
      head += 1;
      return 1;
    }
    return 0;
  }

  virtual std::size_t insert(const T *const in, const std::size_t count) {
    std::size_t inserted = 0;
    for (std::size_t i = 0; i < count; i++) {
      T entry = in[i];
      inserted += insert(entry);
    }
    return inserted;
  }

#if 0
    virtual std::size_t insert(T* const in, std::size_t count) {
        return insert_const(in, count);
    }
#endif

  std::size_t insertOverwrite(const T &in) {
    if (isFull()) {
      tail += 1;
    }
    return insert(in);
  }
  void peek(T *out, const std::size_t pos) const {
    //  pos is the count backwards from head, pos = 0 is head, pos = count is
    //  tail
    std::size_t ArrayPos = 0;
    if (pos <= GetCount()) {
      if (pos <= GetHead()) {
        ArrayPos = GetHead() - pos;
      } else {
        ArrayPos = GetTail() + (GetCount() - pos);
      }
    }
    *out = data[ArrayPos];
  }

#if 0
  [[deprecated]] void peek(T &out, const std::size_t pos) const {
    peek(&out, pos);
  } 
#endif
  
  virtual void Reset(void) {
    head = 0;
    tail = 0;
  }
  void reset() { Reset(); }

  virtual std::size_t GetCount(void) const {
    std::size_t count = 0;
    if (tail <= head) {
      count = head - tail;
    } else {
      count = GetSize() - tail + head;
    }
    assert(count <= kElements);
    return count;
  }
  static constexpr std::size_t GetSize(void) { return kElements; }
  static constexpr std::size_t frameSize(void) { return GetSize(); }

  virtual std::size_t Size(void) const { return GetSize(); }
  std::size_t size(void) const { return GetSize(); }

  virtual ~RingBuffer(void) {}
  constexpr RingBuffer(void) {}
  RingBuffer(const RingBuffer &) = delete;
  RingBuffer operator=(const RingBuffer &) = delete;
};

#if 0
//  FIXME look into using std atomic here
template <typename T, std::size_t size>
class RingBufferThreadSafe : public Buffer<T> {
 private:
  RingBuffer<T, size> ring_buffer_;
  bool lock_ = false;

 public:
  void Lock(void) { lock_ = true; }
  void Release(void) { lock_ = false; }
  bool IsLocked(void) const { return lock_; }

  virtual bool isEmpty(void) const {
    if (!IsLocked()) {
      return ring_buffer_.isEmpty();
    }
    return false;
  }

  virtual bool isFull(void) const {
    if (!IsLocked()) {
      return ring_buffer_.isFull();
    }
    return true;
  }
  virtual std::size_t pop(T *const out, const std::size_t count) {
    if (!IsLocked()) {
      Lock();
      const auto ret = ring_buffer_.pop(out, count);
      Release();
      return ret;
    }
    return 0;
  }
  virtual std::size_t insert(const T *const in, const std::size_t count) {
    if (!IsLocked()) {
      Lock();
      const auto ret = ring_buffer_.insert(in, count);
      Release();
      return ret;
    }
    return 0;
  }

  virtual void Reset(void) { ring_buffer_.Reset(); }
  virtual std::size_t GetCount(void) const { return ring_buffer_.GetCount(); }
  virtual std::size_t Size(void) const { return ring_buffer_.Size(); }

  virtual ~RingBufferThreadSafe(void) {}
  constexpr RingBufferThreadSafe(void) {}
  RingBufferThreadSafe(const RingBufferThreadSafe &) = delete;
  RingBufferThreadSafe operator=(const RingBufferThreadSafe &) = delete;
};
#endif

#endif //  RINGBUFFER_RINGBUFFER_H_
