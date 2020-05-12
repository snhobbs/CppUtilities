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

template <typename T> class SingleElementQueue : public Buffer<T> {
  bool empty_ = true;
  T data_{};

 public:
  virtual void Reset(void) { empty_ = true; }
  virtual bool isEmpty(void) const { return empty_; }
  virtual bool isFull(void) const { return !empty_; }
  uint32_t pop(T &out) { return pop(&out, 1); }
  virtual uint32_t pop(T *const out, const uint32_t) {
    if (!empty_) {
      empty_ = true;
      *out = data_;
      return 1;
    }
    return 0;
  }
  virtual uint32_t insert(const T *const in, const uint32_t) {
    if (!empty_) {
      assert(empty_);
    }
    empty_ = false;
    data_ = *in;
    return 1;
  }

  virtual uint32_t insert(const T &in) { return insert(&in, 1); }
  virtual uint32_t GetCount(void) const {
    if (empty_)
      return 0;
    else
      return 1;
  }
  virtual uint32_t Size(void) const { return 1; }

  constexpr SingleElementQueue(void) {}
  SingleElementQueue(const SingleElementQueue &) = delete;
  SingleElementQueue operator=(const SingleElementQueue &) = delete;

  virtual ~SingleElementQueue(void) {}
};

#endif // RINGBUFFER_SINGLEELEMENTQUEUE_H_
