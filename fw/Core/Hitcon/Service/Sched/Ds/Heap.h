/*
 * Heap.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_SERVICE_SCHED_DS_HEAP_H_
#define HITCON_SERVICE_SCHED_DS_HEAP_H_

#include <Common.h>

namespace hitcon {
namespace service {
namespace sched {

namespace heap {

inline unsigned ParentIdx(unsigned i) {
	return (i-1)/2;
}

inline unsigned ChildIdx1(unsigned i) {
	return (i*2)+1;
}

inline unsigned ChildIdx2(unsigned i) {
	return (i*2)+2;
}

} /* namespace heap */

template<class T, unsigned capacity>
class Heap {
	unsigned sz;
	T *storage[capacity];
private:
	unsigned GetIdx(T &t) {
		for (unsigned i = 0; i < sz; ++i) {
			if (t == *storage[i])
				return i;
		}
		return sz;
	}

	void Heapify(unsigned i) {
		while (i > 0) {
			unsigned parIdx = heap::ParentIdx(i);
			T **cur = &storage[i], **par = &storage[parIdx];
			if (**cur < **par)
				swap(*cur, *par);
			i = parIdx;
		}
	}

	void ReverseHeapify(unsigned i) {
		while (heap::ChildIdx1(i) < sz) {
			unsigned i1 = heap::ChildIdx1(i);
			unsigned i2 = heap::ChildIdx2(i);
			unsigned smallest = i;
			if (*storage[i1] < *storage[i])
				smallest = i1;
			if (i2 < sz && *storage[i2] < *storage[smallest])
				smallest = i2;
			if (i == smallest)
				break;
			swap(*storage[i], *storage[smallest]);
			i = smallest;
		}
	}
public:
	Heap() {}

	virtual ~Heap() {}

	bool Add(T *t) {
		if (sz >= capacity)
			return false;
		storage[sz++] = t;
		Heapify(sz-1);
		return true;
	}

	bool Remove(T *t) {
		unsigned idx = GetIdx(*t);
		if (idx == sz)
			return false;
		storage[idx] = storage[--sz];
		storage[sz] = nullptr;
		ReverseHeapify(idx);
		return true;
	}

	T &Top() {
		return *storage[0];
	}

	unsigned size() {
		return sz;
	}
};

} /* namespace sched */
} /* namespace service */
} /* namespace hitcon */

#endif /* HITCON_SERVICE_SCHED_DS_HEAP_H_ */
