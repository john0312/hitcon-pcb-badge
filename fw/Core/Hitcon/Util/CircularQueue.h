#ifndef UTIL_CIRCULAR_QUEUE_DOT_H_
#define UTIL_CIRCULAR_QUEUE_DOT_H_

#include <cstddef>

namespace hitcon {

// A fixed-capacity circualr queue implemented in an embedded environment.
// The circular queue prevents the use of any Expandable STL collections
// to maintain efficiency and to minimize heap fragmentation.
//
// NOTE: This class does not handle any exceptions for add or remove failures
// due to being empty or full, instead returns a boolean value indicating
// success or failure.
template <class T, unsigned capacity>
class CircularQueue {
 private:
  // Array to hold the elements of the circular queue
  T elements_[capacity];

  // Index of the front of the circular queue
  // This generally points to a valid element when the queue is not empty.
  size_t m_front_;

  // Index of the back of the circular queue
  // This generally points to an empty/invalid element as this position is where
  // the next pushed item will be inserted.
  size_t m_back_;

 public:
  // Constructor: Initializes the circular queue.
  CircularQueue() : m_front_(0), m_back_(0) {}

  // Returns the maximum capacity of the circular queue.
  size_t Capacity() { return capacity; }

  // Returns true if the circular queue is empty.
  bool IsEmpty() { return m_front_ == m_back_; }

  // Returns true if the circular queue is full.
  bool IsFull() { return ((m_back_ + 1) % capacity) == m_front_; }

  // Returns a reference to the front element in the circular queue.
  T& Front() { return elements_[m_front_]; }

  // Returns a reference to the back element in the circular queue.
  T& Back() { return elements_[(m_back_ - 1 + capacity) % capacity]; }

  // Removes the front element from the circular queue.
  void PopFront() {
    if (!IsEmpty()) {
      m_front_ = (m_front_ + 1) % capacity;
    }
  }

  // Removes the back element from the circular queue.
  void PopBack() {
    if (!IsEmpty()) {
      m_back_ = (m_back_ - 1 + capacity) % capacity;
    }
  }

  // Adds an element to the back of the circular queue.
  // Returns true if the operation succeeded, false if the queue was full.
  bool PushBack(T item) {
    if (!IsFull()) {
      elements_[m_back_] = item;
      m_back_ = (m_back_ + 1) % capacity;
      return true;
    }
    return false;
  }

  // Adds an element to the front of the circular queue.
  // Returns true if the operation succeeded, false if the queue was full.
  bool PushFront(T item) {
    if (!IsFull()) {
      m_front_ = (m_front_ - 1 + capacity) % capacity;
      elements_[m_front_] = item;
      return true;
    }
    return false;
  }
};

}  // namespace hitcon

#endif  // UTIL_CIRCULAR_QUEUE_DOT_H_
