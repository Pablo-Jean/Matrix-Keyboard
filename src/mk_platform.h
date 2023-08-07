/*
 * mk_platform.h
 *
 *  Created on: Aug 6, 2023
 *      Author: pablo-jean
 */

#include <stdint.h>

/**
 * Typedefs
 */

typedef enum{
    MK_GPIO_LOW,
    MK_GPIO_HIGH
}mk_gpio_state_e;

// You can edit the below struct if you need
typedef struct{
	uint32_t *GPIO;
	uint32_t pin;
}mk_gpio_t;

/**
 * Publics
 */


void _mk_gpio_write(mk_gpio_t *gpio, mk_gpio_state_e val);

mk_gpio_state_e _mk_gpio_read(mk_gpio_t *gpio);