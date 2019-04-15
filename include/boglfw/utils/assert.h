/*
 * assert.h
 *
 *  Created on: Jan 28, 2015
 *      Author: bogdan
 */

#ifndef UTILS_ASSERT_H_
#define UTILS_ASSERT_H_

#include <cassert>

#ifdef __GNUC__
#define FUNC_NOT_USED_NO_WARN __attribute__ ((unused))
#else
#define FUNC_NOT_USED_NO_WARN
#endif

#ifdef DEBUG
#define ASSERTDBG_ENABLE
#endif

#ifdef ASSERTDBG_ENABLE
static void FUNC_NOT_USED_NO_WARN assertDbg(bool e) {
	if (!e) {
		__builtin_trap();
	}
}
#else
#define assertDbg assert
#endif

#endif /* UTILS_ASSERT_H_ */
