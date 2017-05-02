/*
 * IncFile1.h
 *
 * Created: 25.03.2017 22:12:51
 *  Author: Simon
 */ 


#ifndef BACKLIGHT_H_
#define BACKLIGHT_H_

#include "gpio.h"

#define BACKLIGHT					gpio_sfr(E, 6)

#define backlight_Init()			gpio_direction(BACKLIGHT, GPIO_OUTPUT)

#define backlight_change(state)		gpio_write(BACKLIGHT, !state)

#define backlight_toggle()			gpio_toggle(BACKLIGHT)

#define backlight_set()				backlight_change(true)
#define backlight_clear()			backlight_change(false)

#endif /* BACKLIGHT_H_ */