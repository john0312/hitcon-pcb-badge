/*
 * Common.h
 *
 *  Created on: May 19, 2024
 *      Author: aoaaceai
 */

#ifndef HITCON_COMMON_H_
#define HITCON_COMMON_H_

namespace hitcon {

template<class T>
void swap(T &a, T &b) {
	T tmp = a;
	a = b;
	b = tmp;
}

} /* namespace hitcon */

#endif /* HITCON_COMMON_H_ */
