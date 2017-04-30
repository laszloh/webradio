/*
 * LCD.c
 *
 * Created: 30.04.2017 19:00:52
 *  Author: Simon
 */

#include <avr/io.h>
#include <avr/pgmspace.h>


typedef struct segdef_ {
	uint8_t msg:2;
	uint8_t bit:6;
} segdef_t;

static const segdef_t segmap[] PROGMEM = {
	// character 1
	{.msg = 0, .bit = 7},			// a
	{.msg = 0, .bit = 14},			// b
	{.msg = 0, .bit = 13},			// c
	{.msg = 0, .bit = 12},			// d
	{.msg = 0, .bit = 0},			// e
	{.msg = 0, .bit = 3},			// f
	{.msg = 0, .bit = 1},			// g1
	{.msg = 0, .bit = 9},			// g2
	{.msg = 0, .bit = 2},			// h
	{.msg = 0, .bit = 6},			// i
	{.msg = 0, .bit = 10},			// j
	{.msg = 0, .bit = 4},			// k
	{.msg = 0, .bit = 5},			// l
	{.msg = 0, .bit = 8},			// m
	// character 2
	{.msg = 0, .bit = 23},			// a
	{.msg = 0, .bit = 30},			// b
	{.msg = 0, .bit = 29},			// c
	{.msg = 0, .bit = 28},			// d
	{.msg = 0, .bit = 16},			// e
	{.msg = 0, .bit = 19},			// f
	{.msg = 0, .bit = 17},			// g1
	{.msg = 0, .bit = 25},			// g2
	{.msg = 0, .bit = 18},			// h
	{.msg = 0, .bit = 22},			// i
	{.msg = 0, .bit = 26},			// j
	{.msg = 0, .bit = 20},			// k
	{.msg = 0, .bit = 21},			// l
	{.msg = 0, .bit = 24},			// m
	// character 3
	{.msg = 0, .bit = 39},			// a
	{.msg = 0, .bit = 46},			// b
	{.msg = 0, .bit = 45},			// c
	{.msg = 0, .bit = 44},			// d
	{.msg = 0, .bit = 32},			// e
	{.msg = 0, .bit = 35},			// f
	{.msg = 0, .bit = 33},			// g1
	{.msg = 0, .bit = 41},			// g2
	{.msg = 0, .bit = 34},			// h
	{.msg = 0, .bit = 38},			// i
	{.msg = 0, .bit = 42},			// j
	{.msg = 0, .bit = 36},			// k
	{.msg = 0, .bit = 37},			// l
	{.msg = 0, .bit = 40},			// m
	// character 4
	{.msg = 2, .bit = 3},			// a
	{.msg = 2, .bit = 10},			// b
	{.msg = 2, .bit = 9},			// c
	{.msg = 2, .bit = 8},			// d
	{.msg = 0, .bit = 48},			// e
	{.msg = 0, .bit = 51},			// f
	{.msg = 0, .bit = 49},			// g1
	{.msg = 2, .bit = 5},			// g2
	{.msg = 0, .bit = 50},			// h
	{.msg = 2, .bit = 2},			// i
	{.msg = 2, .bit = 6},			// j
	{.msg = 2, .bit = 0},			// k
	{.msg = 2, .bit = 1},			// l
	{.msg = 2, .bit = 4},			// m
	// character 5
	{.msg = 2, .bit = 51},			// a
	{.msg = 1, .bit = 2},			// b
	{.msg = 1, .bit = 1},			// c
	{.msg = 1, .bit = 0},			// d
	{.msg = 2, .bit = 44},			// e
	{.msg = 2, .bit = 47},			// f
	{.msg = 2, .bit = 45},			// g1
	{.msg = 2, .bit = 17},			// g2
	{.msg = 2, .bit = 46},			// h
	{.msg = 2, .bit = 50},			// i
	{.msg = 2, .bit = 18},			// j
	{.msg = 2, .bit = 48},			// k
	{.msg = 2, .bit = 49},			// l
	{.msg = 2, .bit = 16},			// m
	// character 6
	{.msg = 1, .bit = 11},			// a
	{.msg = 1, .bit = 14},			// b
	{.msg = 1, .bit = 13},			// c
	{.msg = 1, .bit = 12},			// d
	{.msg = 1, .bit = 4},			// e
	{.msg = 1, .bit = 7},			// f
	{.msg = 1, .bit = 5},			// g1
	{.msg = 2, .bit = 21},			// g2
	{.msg = 1, .bit = 6},			// h
	{.msg = 1, .bit = 10},			// i
	{.msg = 2, .bit = 22},			// j
	{.msg = 1, .bit = 8},			// k
	{.msg = 1, .bit = 9},			// l
	{.msg = 2, .bit = 20},			// m
	// character 7
	{.msg = 2, .bit = 23},			// a
	{.msg = 2, .bit = 26},			// b
	{.msg = 2, .bit = 25},			// c
	{.msg = 2, .bit = 24},			// d
	{.msg = 2, .bit = 16},			// e
	{.msg = 2, .bit = 19},			// f
	{.msg = 2, .bit = 17},			// g1
	{.msg = 1, .bit = 25},			// g2
	{.msg = 2, .bit = 18},			// h
	{.msg = 2, .bit = 22},			// i
	{.msg = 1, .bit = 26},			// j
	{.msg = 2, .bit = 20},			// k
	{.msg = 2, .bit = 21},			// l
	{.msg = 1, .bit = 24},			// m
	// character 8
	{.msg = 2, .bit = 35},			// a
	{.msg = 1, .bit = 34},			// b
	{.msg = 1, .bit = 33},			// c
	{.msg = 1, .bit = 32},			// d
	{.msg = 2, .bit = 28},			// e
	{.msg = 2, .bit = 31},			// f
	{.msg = 2, .bit = 29},			// g1
	{.msg = 1, .bit = 29},			// g2
	{.msg = 2, .bit = 30},			// h
	{.msg = 2, .bit = 34},			// i
	{.msg = 1, .bit = 30},			// j
	{.msg = 2, .bit = 32},			// k
	{.msg = 2, .bit = 33},			// l
	{.msg = 1, .bit = 28},			// m
	// special characters

};
