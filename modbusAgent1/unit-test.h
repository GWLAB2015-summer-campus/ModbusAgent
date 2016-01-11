/* tests/unit-test.h.  Generated from unit-test.h.in by configure.  */
/*
 * Copyright © 2008-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

/* Constants defined by configure.ac */
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# ifndef _MSC_VER
# include <stdint.h>
# else
# include "stdint.h"
# endif
#endif

#define SERVER_ID         17
#define INVALID_SERVER_ID 18

const uint16_t UT_BITS_ADDRESS = 0x13;
const uint16_t UT_BITS_NB = 0x25;
const uint8_t UT_BITS_TAB[] = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B };

const uint16_t UT_INPUT_BITS_ADDRESS = 0xC4;
const uint16_t UT_INPUT_BITS_NB = 0x16;
const uint8_t UT_INPUT_BITS_TAB[] = { 0xAC, 0xDB, 0x35 };

const uint16_t UT_REGISTERS_ADDRESS = 0xFF;				// 0x6B
const uint16_t UT_REGISTERS_NB = 0xFF;					// 0x3

/* Raise a manual exception when this adress is used for the first byte */
const uint16_t UT_REGISTERS_ADDRESS_SPECIAL = 0x6C;
const uint16_t UT_REGISTERS_TAB[] = { 0x022B, 0x0001, 0x0064 };
/* If the following value is used, a bad response is sent.
   It's better to test with a lower value than
   UT_REGISTERS_NB_POINTS to try to raise a segfault. */
const uint16_t UT_REGISTERS_NB_SPECIAL = 0x0;			// 0x02

const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0x32;		// 0x08
const uint16_t UT_INPUT_REGISTERS_NB = 0x32;
const uint16_t UT_INPUT_REGISTERS_TAB[] = 
{ 0x0000, 0x0001, 0x0002, 0x0003, 0x0004
, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009
, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E
, 0x000F, 0x0010, 0x0011, 0x0012, 0x0013
, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018
, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D
, 0x001E, 0x001F, 0x0020, 0x0021, 0x0022
, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027
, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C
, 0x002D, 0x002E, 0x002F, 0x0030, 0x0031};

const float UT_REAL = 916.540649;
const uint32_t UT_IREAL = 0x4465229a;

const char memory_flag[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 	// 0x00~0x0F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 0x10~0x1F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 0x20~0x2F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 0x30~0x3F
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 0x40~0x4F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,     // 0x50~0x5F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 0x60~0x6F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 0x70~0x7F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,     // 0x80~0x8F
	0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 0x90~0x9F
	0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 0xA0~0xAF
	0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 0xB0~0xBF
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 0xC0~0xCF
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 0xD0~0xDF
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // 0xE0~0xEF
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,		// 0xF0~0xFF
};
// if value == 0, this register is RO
// if vallue != 0, this register is RW

#endif /* _UNIT_TEST_H_ */
