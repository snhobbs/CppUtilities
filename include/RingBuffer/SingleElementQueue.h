/*
 * Copyright 2020 ElectroOptical Innovations, LLC
 * SingleElementQueue.h
 *
 *  Created on: Nov 6, 2019
 *      Author: simon
 */

#pragma once
#ifndef RINGBUFFER_SINGLEELEMENTQUEUE_H_
#define RINGBUFFER_SINGLEELEMENTQUEUE_H_

#include <RingBuffer/RingBuffer.h>

template <typename T> class SingleElementQueueBase {
 public:
  void Reset(void) { empty_ = true; }
  bool isEmpty(void) const { return empty_; }
  bool isFull(void) const { return !empty_; }
  uint32_t pop(T &out) { return pop(&out, 1); }
  uint32_t pop(T *const out, const uint32_t) {
    if (!empty_) {
      empty_ = true;
      *out = data_;
      return 1;
    }
    return 0;
  }
  uint32_t insert(const T *const in, const uint32_t) {
    if (!empty_) {
      assert(empty_);
    }
    empty_ = false;
    data_ = *in;
    return 1;
  }

  uint32_t GetCount(void) const {
    return empty_ ? 0 : size();
  }
  uint32_t size(void) const { return 1; }

 private:
  bool empty_ = true;
  T data_{};
};

template <typename T> class SingleElementQueue : public Buffer<T> {
 public:
  virtual void Reset(void) { buffer_.reset(); }
  virtual bool isEmpty(void) const { return buffer_.isEmpty(); }
  virtual bool isFull(void) const { return buffer_.isFull(); }
  [[deprecated]] uint32_t pop(T &out) { return pop(&out, 1); }
  virtual uint32_t pop(T *const out, const uint32_t count) {
    return buffer_.pop(out, count);
  }
  virtual uint32_t insert(const T *const in, const uint32_t count) {
    return buffer_.insert(in, count);
  }
  [[deprecated]] virtual uint32_t insert(const T &in) { return insert(&in, 1); }
  virtual uint32_t GetCount(void) const {
    return buffer_.GetCount();
  }
  virtual uint32_t Size(void) const { return buffer_.size(); }

  constexpr SingleElementQueue(void) {}
  SingleElementQueue(const SingleElementQueue &) = delete;
  SingleElementQueue operator=(const SingleElementQueue &) = delete;

  virtual ~SingleElementQueue(void) {}

 private:
  SingleElementQueueBase<T> buffer_;
};

#endif // RINGBUFFER_SINGLEELEMENTQUEUE_H_
