#ifdef CIRCULAR_QUEUE_TEST

// g++ -I./ -Wall -pedantic -O0 Util/CircularQueue.cc Util/CircularQueueTest.cc
// -o Util/CircularQueueTest -DCIRCULAR_QUEUE_TEST

#include <Util/CircularQueue.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <deque>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>

// Test basic initialization and state properties.
void test_initial_state() {
  std::cout << "Running test_initial_state..." << std::endl;

  // Test with uint32_t and capacity 10
  hitcon::CircularQueue<uint32_t, 10> cq1;
  assert(cq1.IsEmpty());
  assert(!cq1.IsFull());
  assert(cq1.Size() == 0);
  assert(cq1.Capacity() == 10);

  // Test with int8_t and capacity 5
  hitcon::CircularQueue<int8_t, 5> cq2;
  assert(cq2.IsEmpty());
  assert(!cq2.IsFull());
  assert(cq2.Size() == 0);
  assert(cq2.Capacity() == 5);

  // Test with int64_t and capacity 128
  hitcon::CircularQueue<int64_t, 128> cq3;
  assert(cq3.IsEmpty());
  assert(!cq3.IsFull());
  assert(cq3.Size() == 0);
  assert(cq3.Capacity() == 128);

  // Edge case: capacity 2
  hitcon::CircularQueue<int, 2> cq4;
  assert(cq4.IsEmpty());
  assert(!cq4.IsFull());
  assert(cq4.Size() == 0);
  assert(cq4.Capacity() == 2);

  std::cout << "test_initial_state PASSED." << std::endl;
}

// Test PushBack, Front, Back, and IsFull.
void test_push_back() {
  std::cout << "Running test_push_back..." << std::endl;
  const unsigned capacity = 16;
  hitcon::CircularQueue<int, capacity> cq;

  // Push items to the back
  for (unsigned i = 0; i < capacity - 1; ++i) {
    assert(cq.PushBack(static_cast<int>(i)));
    assert(cq.Size() == i + 1);
    assert(cq.Front() == 0);
    assert(cq.Back() == static_cast<int>(i));
    if (i != capacity - 2) assert(!cq.IsFull());
  }

  // Queue is now full
  assert(cq.IsFull());
  assert(cq.Size() == capacity - 1);

  // Try to push to a full queue
  assert(!cq.PushBack(99));
  assert(cq.Size() == capacity - 1);  // Size should not change
  assert(cq.Back() ==
         static_cast<int>(capacity - 2));  // Back should not change

  std::cout << "test_push_back PASSED." << std::endl;
}

// Test PushFront, Front, Back, and IsFull.
void test_push_front() {
  std::cout << "Running test_push_front..." << std::endl;
  const unsigned capacity = 31;
  hitcon::CircularQueue<int, capacity> cq;

  // Push items to the front
  for (unsigned i = 0; i < capacity - 1; ++i) {
    assert(cq.PushFront(static_cast<int>(i)));
    assert(cq.Size() == i + 1);
    assert(cq.Front() == static_cast<int>(i));
    assert(cq.Back() == 0);
    if (i != capacity - 2) assert(!cq.IsFull());
  }

  // Queue is now full
  assert(cq.IsFull());
  assert(cq.Size() == capacity - 1);

  // Try to push to a full queue
  assert(!cq.PushFront(99));
  assert(cq.Size() == capacity - 1);  // Size should not change
  assert(cq.Front() ==
         static_cast<int>(capacity - 2));  // Front should not change

  std::cout << "test_push_front PASSED." << std::endl;
}

// Test PopFront operation.
void test_pop_front() {
  std::cout << "Running test_pop_front..." << std::endl;
  const unsigned capacity = 8;
  hitcon::CircularQueue<int, capacity> cq;

  for (unsigned i = 0; i < capacity - 1; ++i) {
    cq.PushBack(static_cast<int>(i));
  }

  for (unsigned i = 0; i < capacity - 1; ++i) {
    assert(cq.Size() == (capacity - 1) - i);
    assert(cq.Front() == static_cast<int>(i));
    cq.PopFront();
  }

  assert(cq.IsEmpty());
  assert(cq.Size() == 0);

  // Try to pop from an empty queue
  cq.PopFront();
  assert(cq.IsEmpty());

  std::cout << "test_pop_front PASSED." << std::endl;
}

// Test PopBack operation.
void test_pop_back() {
  std::cout << "Running test_pop_back..." << std::endl;
  const unsigned capacity = 8;
  hitcon::CircularQueue<int, capacity> cq;

  for (unsigned i = 0; i < capacity - 1; ++i) {
    cq.PushBack(static_cast<int>(i));
  }

  for (unsigned i = 0; i < capacity - 1; ++i) {
    assert(cq.Size() == (capacity - 1) - i);
    assert(cq.Back() == static_cast<int>((capacity - 2) - i));
    cq.PopBack();
  }

  assert(cq.IsEmpty());
  assert(cq.Size() == 0);

  // Try to pop from an empty queue
  cq.PopBack();
  assert(cq.IsEmpty());

  std::cout << "test_pop_back PASSED." << std::endl;
}

// Test wrap-around behavior.
void test_wrap_around() {
  std::cout << "Running test_wrap_around..." << std::endl;
  const unsigned capacity = 5;
  hitcon::CircularQueue<int, capacity> cq;

  // Fill the queue
  for (int i = 0; i < 4; ++i) cq.PushBack(i);  // 0, 1, 2, 3

  // Pop two elements
  cq.PopFront();  // 1, 2, 3
  cq.PopFront();  // 2, 3
  assert(cq.Front() == 2);
  assert(cq.Size() == 2);

  // Push two more elements to force wrap-around
  cq.PushBack(4);  // 2, 3, 4
  cq.PushBack(5);  // 2, 3, 4, 5
  assert(cq.IsFull());
  assert(cq.Front() == 2);
  assert(cq.Back() == 5);
  assert(cq.Size() == 4);

  // Pop all elements to check correctness
  assert(cq.Front() == 2);
  cq.PopFront();
  assert(cq.Front() == 3);
  cq.PopFront();
  assert(cq.Front() == 4);
  cq.PopFront();
  assert(cq.Front() == 5);
  cq.PopFront();
  assert(cq.IsEmpty());

  // Test PushFront wrap-around
  cq.Clear();
  for (int i = 0; i < 4; ++i) cq.PushFront(i);  // 3, 2, 1, 0
  assert(cq.Front() == 3);
  assert(cq.Back() == 0);

  cq.PopBack();  // 3, 2, 1
  cq.PopBack();  // 3, 2
  assert(cq.Back() == 2);
  assert(cq.Size() == 2);

  cq.PushFront(4);  // 4, 3, 2
  cq.PushFront(5);  // 5, 4, 3, 2
  assert(cq.IsFull());
  assert(cq.Front() == 5);
  assert(cq.Back() == 2);

  std::cout << "test_wrap_around PASSED." << std::endl;
}

// Test Clear operation.
void test_clear() {
  std::cout << "Running test_clear..." << std::endl;
  hitcon::CircularQueue<int, 5> cq;

  cq.PushBack(1);
  cq.PushBack(2);
  cq.Clear();

  assert(cq.IsEmpty());
  assert(cq.Size() == 0);

  // Should be able to use the queue normally after clearing
  assert(cq.PushBack(10));
  assert(cq.PushBack(20));
  assert(cq.Size() == 2);
  assert(cq.Front() == 10);
  assert(cq.Back() == 20);

  std::cout << "test_clear PASSED." << std::endl;
}

// Test PeekSegment operation.
void test_peek_segment() {
  std::cout << "Running test_peek_segment..." << std::endl;
  const unsigned capacity = 10;
  hitcon::CircularQueue<int, capacity> cq;
  int buffer[capacity];

  // Fill queue: 0, 1, 2, 3, 4, 5, 6, 7, 8
  for (int i = 0; i < 9; ++i) cq.PushBack(i);

  // Test 1: Simple peek from the start
  assert(cq.PeekSegment(buffer, 3, 0));
  for (int i = 0; i < 3; ++i) assert(buffer[i] == i);

  // Test 2: Peek from the middle
  assert(cq.PeekSegment(buffer, 4, 2));
  for (int i = 0; i < 4; ++i) assert(buffer[i] == i + 2);

  // Test 3: Peek until the end
  assert(cq.PeekSegment(buffer, 5, 4));
  for (int i = 0; i < 5; ++i) assert(buffer[i] == i + 4);

  // Test 4: Peek past the end (should fail)
  assert(!cq.PeekSegment(buffer, 5, 5));
  assert(!cq.PeekSegment(buffer, 1, 9));

  // Test 5: Peek with wrap-around
  cq.PopFront();    // 1, 2, 3, 4, 5, 6, 7, 8
  cq.PopFront();    // 2, 3, 4, 5, 6, 7, 8
  cq.PushBack(9);   // 2, 3, 4, 5, 6, 7, 8, 9
  cq.PushBack(10);  // 2, 3, 4, 5, 6, 7, 8, 9, 10 (full)
  // Internal array state (example): [10, _, 2, 3, 4, 5, 6, 7, 8, 9]
  // m_front_ is at index 2, m_back_ is at index 1

  assert(cq.Size() == 9);
  assert(cq.Front() == 2);
  assert(cq.Back() == 10);

  // Peek a segment that wraps
  assert(cq.PeekSegment(buffer, 5, 4));  // Peeking 6, 7, 8, 9, 10
  int expected1[] = {6, 7, 8, 9, 10};
  for (int i = 0; i < 5; ++i) assert(buffer[i] == expected1[i]);

  // Peek the whole wrapped queue
  assert(cq.PeekSegment(buffer, 9, 0));
  int expected2[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
  for (int i = 0; i < 9; ++i) assert(buffer[i] == expected2[i]);

  std::cout << "test_peek_segment PASSED." << std::endl;
}

// Test RemoveFrontMulti and RemoveBackMulti operations.
void test_remove_multi() {
  std::cout << "Running test_remove_multi..." << std::endl;
  const unsigned capacity = 10;
  hitcon::CircularQueue<int, capacity> cq;

  // --- Test RemoveFrontMulti ---
  // Fill queue: 0, 1, 2, 3, 4, 5, 6, 7, 8 (size 9)
  for (int i = 0; i < 9; ++i) cq.PushBack(i);

  // Test 1: Remove more than available
  assert(!cq.RemoveFrontMulti(10));
  assert(cq.Size() == 9);
  assert(cq.Front() == 0);

  // Test 2: Remove 0 elements
  assert(cq.RemoveFrontMulti(0));
  assert(cq.Size() == 9);

  // Test 3: Remove 3 elements
  assert(cq.RemoveFrontMulti(3));  // 3, 4, 5, 6, 7, 8
  assert(cq.Size() == 6);
  assert(cq.Front() == 3);

  // Test 4: Remove all remaining elements
  assert(cq.RemoveFrontMulti(6));
  assert(cq.IsEmpty());

  // Test 5: Remove from empty queue
  assert(!cq.RemoveFrontMulti(1));
  assert(cq.RemoveFrontMulti(0));

  // Test 6: Wrap-around case
  cq.Clear();
  for (int i = 0; i < 8; ++i) cq.PushBack(i);  // 0, 1, 2, 3, 4, 5, 6, 7
  cq.PopFront();                               // 1, 2, 3, 4, 5, 6, 7
  cq.PopFront();                               // 2, 3, 4, 5, 6, 7
  cq.PushBack(8);                              // 2, 3, 4, 5, 6, 7, 8
  cq.PushBack(9);                              // 2, 3, 4, 5, 6, 7, 8, 9
  // front is at 2, back is at 0. size is 8.
  assert(cq.Size() == 8);
  assert(cq.Front() == 2);
  assert(cq.RemoveFrontMulti(4));  // 6, 7, 8, 9
  assert(cq.Size() == 4);
  assert(cq.Front() == 6);
  cq.RemoveFrontMulti(4);
  assert(cq.IsEmpty());

  // --- Test RemoveBackMulti ---
  cq.Clear();
  // Fill queue: 0, 1, 2, 3, 4, 5, 6, 7, 8 (size 9)
  for (int i = 0; i < 9; ++i) cq.PushBack(i);

  // Test 1: Remove more than available
  assert(!cq.RemoveBackMulti(10));
  assert(cq.Size() == 9);
  assert(cq.Back() == 8);

  // Test 2: Remove 0 elements
  assert(cq.RemoveBackMulti(0));
  assert(cq.Size() == 9);

  // Test 3: Remove 3 elements
  assert(cq.RemoveBackMulti(3));  // 0, 1, 2, 3, 4, 5
  assert(cq.Size() == 6);
  assert(cq.Back() == 5);

  // Test 4: Remove all remaining elements
  assert(cq.RemoveBackMulti(6));
  assert(cq.IsEmpty());

  // Test 5: Remove from empty queue
  assert(!cq.RemoveBackMulti(1));
  assert(cq.RemoveBackMulti(0));

  // Test 6: Wrap-around case
  cq.Clear();
  for (int i = 0; i < 8; ++i) cq.PushFront(i);  // 7, 6, 5, 4, 3, 2, 1, 0
  cq.PopBack();                                 // 7, 6, 5, 4, 3, 2, 1
  cq.PopBack();                                 // 7, 6, 5, 4, 3, 2
  cq.PushFront(8);                              // 8, 7, 6, 5, 4, 3, 2
  cq.PushFront(9);                              // 9, 8, 7, 6, 5, 4, 3, 2
  // back is at 2, front is at some wrapped around value. size is 8.
  assert(cq.Size() == 8);
  assert(cq.Back() == 2);
  assert(cq.RemoveBackMulti(4));  // 9, 8, 7, 6
  assert(cq.Size() == 4);
  assert(cq.Back() == 6);
  cq.RemoveBackMulti(4);
  assert(cq.IsEmpty());

  std::cout << "test_remove_multi PASSED." << std::endl;
}

// Comprehensive test comparing CircularQueue against std::deque.
template <typename T, unsigned capacity>
void test_with_std_deque() {
  std::cout << "Running test_with_std_deque for type " << typeid(T).name()
            << " with capacity " << capacity << "..." << std::endl;

  hitcon::CircularQueue<T, capacity> cq;
  std::deque<T> dq;

  auto compare_states = [&]() {
    assert(cq.Size() == dq.size());
    assert(cq.IsEmpty() == dq.empty());
    if (!dq.empty()) {
      assert(cq.Front() == dq.front());
      assert(cq.Back() == dq.back());

      // Test PeekSegment against deque content
      if (cq.Size() > 0) {
        std::vector<T> cq_buffer(dq.size());
        std::vector<T> dq_buffer(dq.begin(), dq.end());
        assert(cq.PeekSegment(cq_buffer.data(), cq.Size(), 0));
        assert(cq_buffer == dq_buffer);
      }
    }
  };

  // A mix of operations
  for (int i = 0; i < 20000; ++i) {
    int op = rand() % 8;
    T val = static_cast<T>(rand());

    switch (op) {
      case 0:  // PushBack
        if (!cq.IsFull()) {
          assert(cq.PushBack(val));
          dq.push_back(val);
        }
        break;
      case 1:  // PushFront
        if (!cq.IsFull()) {
          assert(cq.PushFront(val));
          dq.push_front(val);
        }
        break;
      case 2:  // PopFront
        if (!cq.IsEmpty()) {
          cq.PopFront();
          dq.pop_front();
        }
        break;
      case 3:  // PopBack
        if (!cq.IsEmpty()) {
          cq.PopBack();
          dq.pop_back();
        }
        break;
      case 4:                                       // Clear
        if (cq.Size() > 0 && (rand() % 20 == 0)) {  // Clear occasionally
          cq.Clear();
          dq.clear();
        }
        break;
      case 5:  // PeekSegment (verified inside compare_states)
        if (cq.Size() > 1) {
          size_t offset = rand() % (cq.Size() - 1);
          size_t count = rand() % (cq.Size() - offset);
          if (count == 0) count = 1;

          std::vector<T> cq_peek_buffer(count);
          std::vector<T> dq_peek_buffer;
          assert(cq.PeekSegment(cq_peek_buffer.data(), count, offset));
          std::copy(dq.begin() + offset, dq.begin() + offset + count,
                    std::back_inserter(dq_peek_buffer));
          assert(cq_peek_buffer == dq_peek_buffer);
        }
        break;
      case 6:  // RemoveFrontMulti
        if (cq.Size() > 0) {
          size_t count = rand() % (cq.Size() + 1);  // Can remove 0 up to Size()
          assert(cq.RemoveFrontMulti(count));
          for (size_t k = 0; k < count; ++k) dq.pop_front();
        } else {
          assert(!cq.RemoveFrontMulti(1));  // Should fail on empty
          assert(cq.RemoveFrontMulti(0));   // Should succeed on empty
        }
        break;
      case 7:  // RemoveBackMulti
        if (cq.Size() > 0) {
          size_t count = rand() % (cq.Size() + 1);
          assert(cq.RemoveBackMulti(count));
          for (size_t k = 0; k < count; ++k) dq.pop_back();
        } else {
          assert(!cq.RemoveBackMulti(1));  // Should fail on empty
          assert(cq.RemoveBackMulti(0));   // Should succeed on empty
        }
        break;
    }
    compare_states();
  }

  std::cout << "test_with_std_deque for type " << typeid(T).name() << " PASSED."
            << std::endl;
}

int main() {
  srand(time(0));  // Seed for random operations in deque test

  std::cout << "--- Starting CircularQueue Tests ---" << std::endl;

  test_initial_state();
  test_push_back();
  test_push_front();
  test_pop_front();
  test_pop_back();
  test_wrap_around();
  test_clear();
  test_peek_segment();
  test_remove_multi();

  // Run comprehensive deque comparison test for various types and sizes
  test_with_std_deque<uint32_t, 50>();
  test_with_std_deque<int8_t, 20>();
  test_with_std_deque<int64_t, 100>();
  test_with_std_deque<int, 128>();  // Another size

  std::cout << "--- All CircularQueue Tests Passed! ---" << std::endl;

  return 0;
}

#endif  // #ifdef CIRCULAR_QUEUE_TEST