/*
 * Array
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHED_DS_ARRAY_H_
#define HITCON_SERVICE_SCHED_DS_ARRAY_H_

#include <Common.h>

namespace hitcon {
namespace service {
namespace sched {

template <class T, unsigned capacity>
class Array {
  unsigned sz;
  T *storage[capacity];

 private:
  unsigned GetIdx(T &t) {
    for (unsigned i = 0; i < sz; ++i) {
      if (t == *storage[i]) return i;
    }
    return sz;
  }

 public:
  bool Add(T *t) {
    if (sz >= capacity) return false;
    storage[sz++] = t;
    return true;
  }

  bool Remove(T *t) {
    unsigned idx = GetIdx(*t);
    if (idx == sz) return false;
    storage[idx] = storage[--sz];
    return true;
  }
};

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_DS_ARRAY_H_ */
