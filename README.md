![Created for Ouroboros Embedded Education](imgs/logo-education.png)

Created for Ouroboros Embedded Education.

## Introduction

The Matrix Keyboard library is a simple and easy-to-use lib for implementing Matrix Keyboards. You can use any matrix with a configuration of 2x2 or greater.

### 🧮 Main features

- No chipset dependency;
- Works with Inputs configured as Pull-Down or Pull-Up resistor configuration;
- Works with Outputs configured as Push-Pull or Open-Drain/Source;
- The function that checks for pressed keys has any delay or latency;
- Can be called on: RTOS Task, Main Loop, or even on a Timer Interrupt;
- Implements a Callback function, avoiding the use of polling (but you can still use it);
- By using instancing, you can have more than 1 matrix keyboard.

## Quick Start

Before we discuss about implement and use this lib, let's understand quickly the File Structure:

```
matrix_keyboard
│   README.md   
└─── src
│   │   matrix_keyboard.c
│   │   matrix_keyboard.h
│   │	mk_platform.h
│   │	mk_platform_blank.c
```

- `matrix_keyboard.h` has the structs and function prototypes (I will explain later for you).
- `matrix_keyboard.c` contains the source code of the API.
- `mk_platform.h` has the prototypes of gpio basic functions, this will allow you to 'connect' to any chipset.
- `mk_platform_blank.c` is a blank file that you can use to implement the chipset routines for the GPIO read and write

### 1. Implement the platform functions

You need to implement only two things related to the chipset, on the mk_platform_xx.c: 

- **GPIO Write function**: write the High and Low levels on a Pin output;
- **GPIO Read function**: read the digital state of the input.

The `mk_platform_blank.h` header file is described below:

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
	uint32_t GPIO;
	uint32_t pin;
}mk_gpio_t;

/**
 * Publics
 */

void _mk_gpio_write(mk_gpio_t *gpio, mk_gpio_state_e val);

mk_gpio_state_e _mk_gpio_read(mk_gpio_t *gpio);
```

The `mk_gpio_t` can handle almost existing chipsets, because, you can have a GPIO handler and a pin number (STM32 devices, for example), or only the pin number (nRF52810, for example).

And the blank source file as the source code of the `_mk_gpio_write` and `_mk_gpio_read`, below, I'm showing you an example, for a STM32F411 device.

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

After instancing, this `mk_t` variable type, requires some configurations:

- Array containing the GPIOs and Pin numbers;
- Active level;
- The masking of events on Callback;
- Number of Columns and Rows.

Here's an implementation for a 4x4 matrix keyboard, where **KEY_Cx** refers to the Columns of the Keyboard, and **KEY_Rx** to the rows.

```C
mk_t Mk;

mk_gpio_t ColGpios[4] = {
		{(uint32_t)(KEY_C0_GPIO_Port), KEY_C0_Pin},
		{(uint32_t)(KEY_C1_GPIO_Port), KEY_C1_Pin},
		{(uint32_t)(KEY_C2_GPIO_Port), KEY_C2_Pin},
		{(uint32_t)(KEY_C3_GPIO_Port), KEY_C3_Pin}
};
mk_gpio_t RowGpios[4] = {
		{(uint32_t)(KEY_R0_GPIO_Port), KEY_R0_Pin},
		{(uint32_t)(KEY_R1_GPIO_Port), KEY_R1_Pin},
		{(uint32_t)(KEY_R2_GPIO_Port), KEY_R2_Pin},
		{(uint32_t)(KEY_R3_GPIO_Port), KEY_R3_Pin} 
};

void main(){
    mk_status_e err;
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

    err = mk_init(&Mk);
    if (err != MK_STATUS_OK){
        // something was wrong
        assert(0);
    }
    // ......
}
```

The GPIOs are previously configured on the CubeMX plugin, with Outputs, in this example, configured as Open-Drain, and the Inputs with the internal Pull-Up resistor. And, in this example, I've enabled the callback, that will be called for Pressed and Released events.

### 3. Call the Operation function periodically

After the initialization, the function `mk_DoOperation` must be called periodically, this will read a Column, check for the signal of the rows, turn on the next Column, and give back an event if anything was changed. You can do this in two ways:

#### Polling

You can call this function inside the main loop or in an RTOS task. Every time you call the function, you can check the event provided by the parameter. In this case, the event is independent of the eventMask.
