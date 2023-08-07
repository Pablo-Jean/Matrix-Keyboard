![Created for Ouroboros Embedded Education](imgs/logo-education.png)

Created for Ouroboros Embedded Education.

## Introduction

The Matrix Keyboard library is a simple and easy-to-use lib for implementing Matrix Keyboards. You can use any matrix with a configuration of 2x2 or greater.

### Main features

- No chipset dependency;
- Works with Inputs configured as Pull-Down or Pull-Up resistor configuration;
- Works with Outputs configured as Push-Pull or Open-Drain/Source;
- The function that check for pressed keys has any delay or latency;
- Can be called on: RTOS Task, Main Loop or even on a Timer Interrupt;
- Implements a Callback function, avoiding the use of polling (but you can still use it);
- By using instancing, you can have more than 1 matrix keyboard.

## Quick Start

### 1. Implement the platform functions

You need to implement only three things related with the chipset: 

- GPIO Write function
- GPIO Read function

I suggest you to start using the `mk_platform_blank.c` file to start, the `mk_platform_blank.h` won't need to be changed, but, if you need, you can make small changes.

The header file is described below:

```C
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
```

Let's take the STM32F411 as a example:

```C

#include "mk_platform.h"

/* Insert here the include of gpio driver of your chipset*/
#include <stm32f4xx.h>

void _mk_gpio_write(mk_gpio_t *gpio, mk_gpio_state_e val){
    GPIO_TypeDef *GPIO;
    uint32_t Pin;
    
    GPIO = (GPIO_TypeDef*)(gpio->GPIO);
    Pin = (uint32_t)(gpio->pin);
    if (val == MK_GPIO_LOW){
        // write low value on the gpio port
        HAL_GPIO_WritePin(GPIO, Pin, GPIO_PIN_RESET);
    }
    else{
        // write high value on the gpio port
        HAL_GPIO_WritePin(GPIO, Pin, GPIO_PIN_SET);
    }
}

mk_gpio_state_e _mk_gpio_read(mk_gpio_t *gpio){
    // check the gpio state and return the value:
    // MK_GPIO_LOW if the state is low
    // MK_GPIO_HIGH if the state was high
    GPIO_TypeDef *GPIO;
    uint32_t Pin;
    
    GPIO = (GPIO_TypeDef*)(gpio->GPIO);
    if (HAL_GPIO_ReadPin(GPIO, Pin) == GPIO_PIN_LOW){
        return MK_GPIO_LOW;
    }
    else{
        return MK_GPIO_HIGH;
    }
}
```

With this, the library can handle every gpio operation. 

### 2. Include in you project and Instance the Keypad

After adding the library on you project, include the `matrix_keyboard.h` file and instance your Keyboard.

```C
#include "matrix_keyboard.h"

mk_t Mk;
```

After instancing, this `mk_t` variable type, requires some configurations, like:

- Array containing the GPIOs and Pin numbers;
- Active level;
- The masking of events on Callback;
- Number of Columns and Rows.

```C
mk_t Mk;

mk_gpio_t ColGpios[4] = {
		{(uint32_t)(MBTN_C0_GPIO_Port), MBTN_C0_Pin},
		{(uint32_t)(MBTN_C1_GPIO_Port), MBTN_C1_Pin},
		{(uint32_t)(MBTN_C2_GPIO_Port), MBTN_C2_Pin},
		{(uint32_t)(MBTN_C3_GPIO_Port), MBTN_C3_Pin}
};
mk_gpio_t RowGpios[4] = {
		{(uint32_t)(MBTN_R0_GPIO_Port), MBTN_R0_Pin,
		{(uint32_t)(MBTN_R1_GPIO_Port), MBTN_R1_Pin,
		{(uint32_t)(MBTN_R2_GPIO_Port), MBTN_R2_Pin,
		{(uint32_t)(MBTN_R3_GPIO_Port), MBTN_R3_Pin
};

void main(){
    // ....... 
    // configure the active level of button as low.
    Mk.actLevel = MK_ACTIVE_LEVEL_LOW;
    // enable callback for Pressed and Released detection event
    Mk.eventMask = (MK_EVENT_PRESSED | MK_EVENT_RELEASED);
    // Configure Columns and Rows as 4x4
    Mk.nCols = 4;
    Mk.nRows = 4;
    // Insert the array containing the GPIOs handlers and Pin numbers
    Mk.gCols = ColGpios;
    Mk.gRows = RowGpios;

    if (mk_init(&Mk) != MK_STATUS_OK){
        // some think was wrong
        assert(0);
    }
    // ......
}
```

`... I will finish the documentation later`