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

### Instance Struct

```C
typedef struct{
// Parameters that must be provided by developer
	/**
	 * The columns must be configured as outputs
	 * That can be Push-Pull or Open-drain/collector
	 */
	mk_gpio_t *gCols;
	/**
	 * The Rows must be configured as inputs, the
	 * Pull resistors must be set according with the
	 * Columns config and Active Level
	 */
	mk_gpio_t *gRows;
	/**
	 * The absolute number of columns and rows
	 * Must be greater or equal 2.
	 * Yes, we do not accept 1 row/column keyboard
	 */
	uint32_t nCols;
	uint32_t nRows;
	/**
	 * The active level indicates the signal will
	 * indicate that the button was pressed.
	 * An example of a correctly actLevel is:
	 * - Columns: Configured as Open-Drain
	 * - Rows: Configured as Inputs with Pull-Ups
	 * - actLevel: Configured for Low level (MK_ACTIVE_LEVEL_LOW)
	 */
	mk_active_level_e actLevel;
	/**
	 * Select the events that must ne called on callback
	 * MK_EVENT_RELEASED: When detecting a releasing of button
	 * MK_EVENT_PRESSED: When detecting a pressing of a button
	 *
	 * You can select one or more events, using OR:
	 * eventMask = (MK_EVENT_RELEASED | MK_EVENT_PRESSED)
	 * or you can select all:
	 * eventMask = MK_EVENT_ALL
	 *
	 * If you leave this field with no event, the callback will
	 * never me called.
	 */
	uint8_t eventMask;

// the below values must not be changed by developer
	// is only for internal use
	struct{
		uint8_t _mkInit;
		uint32_t _actCol;
		uint32_t _actRow;
		uint32_t _prevColState;
		uint32_t _prevRowState;
		uint32_t _testCol;
		GPIO_PinState _onSignal;
		GPIO_PinState _offSignal;
	}_internal;
}mk_t;
```

### API

#### mk_status_e mk_init(mk_t *Mk)

This function initialized the `mk_t` variable with the provided parameters. The return STATUS can be:

- MK_STATUS_OK: Everything is fine
- MK_STATUS_INCORRET_PARAMS: some privided parameter is incorrect and not allowed

#### mk_status_e mk_get_button(mk_t *Mk, uint32_t *pPressedCol, uint32_t *pPressedRow)

Gives you the pressed button, based on Column and Row pressed, the values can be 1 to the max number of the row/column. If no key is detected, the values of pPressedCol and pPressedRow is `0`. Can provide the following returns;

- MK_STATUS_OK: Everything is fine
- MK_STATUS_INCORRET_PARAMS: some parameters was NULL
- MK_STATUS_NOT_INITIALIZED: the mk_init functions wasn't called for the `Mk` instance.

#### mk_status_e mk_DoOperation(mk_t *Mk, mk_event_e *Ev)

Must be called periodically, this function does the detection of Rows, and turning on/off the Columns. Is responsible to call the `callback`, if events are enabled. The `Ev` parameter can be NULL. Can provide the following returns;

- MK_STATUS_OK: Everything is fine
- MK_STATUS_INCORRET_PARAMS: `Mk` was NULL
- MK_STATUS_NOT_INITIALIZED: the `mk_init` functions wasn't called for the `Mk` instance.

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

After the initialization, the function `mk_DoOperation` must be called periodically, this will read a Column, check for the signal of the rows, turn on the next Column, and give back an event if anything was changed. Remember, every `DoOperation` command only check for one columns, the next will be readed on a new call, in other words, a full check of the keyboard requires the Number of Columns multiplied by the time period of the function calling. But, if a key was detected, the API doesn't check for the next columns, it will return to the next column, and give back the PRESSED event.

You can do this in two ways:

#### Polling

You can call this function inside the main loop or in an RTOS task. Every time you call the function, you can check the event provided by the parameter. In this case, the event is independent of the eventMask.

Below, an example of using the main loop to process the matrix;

```C
void main(){
    mk_event_e MkEvent;
    mk_error_e MkError;
    uint32_t BtnCol, BtnRow;
    // ....... 

    while (1){
        // ....... 

        MkError = mk_DoOperation(&Mk, &MkEvent);
        if (MkError != MK_STATUS_OK){
            assert(0);
        }
        // we detected a PRESSED event
        if (MkEvent == MK_EVENT_PRESSED){
            // So, you just need to call this function to get the Column and Row of
            // the pressed button
            // This event triggered only one time, while the key is pressed
            mk_get_button(&Mk, &BtnCol, &BtnRow);
        }
        else if (MkEvent == MK_EVENT_RELEASED){
            // the button was released, and no other button was pressed
        }

        // ....... 
        osDelay(50);
    }

    // ......
}
```

If you use an RTOS, you can do with a dedicated task.

```C
void xKeyTask(void *pvParams){
    mk_event_e MkEvent;
    mk_error_e MkError;
    // ....... 

    while (1){
        // ....... 

        MkError = mk_DoOperation(&Mk, &MkEvent);
        if (MkError != MK_STATUS_OK){
            assert(0);
        }
        // we detected a PRESSED event
        if (MkEvent == MK_EVENT_PRESSED){
            // you can trigger a semaphore to inform a main task that
            // a button is pressed
        }
        else if (MkEvent == MK_EVENT_RELEASED){
            // same as before
        }

        // ....... 
        vTaskSleep(pdMS_TO_TICKS(50));
    }

    // ......
}
```

#### Timer Interrupt

The `DoOperation` doens't have any delay inside, the function is builded to check the column previously enabled in last cycle, so, avoiding any use of delay to make sure that the signal is stable.

This bring the advantage to implement on a Periodic Timer Interrupt, the example below, hhow you how to use the a timer interrupt and the callback function.

```C
// In this example, the Timer 4 was configured to trigger the interrupt
// every 10 ms
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM4){
		mk_DoOperation(&Mk, NULL);
	}
}

// The callback of API
void mk_callback(mk_t *Mk, mk_event_e ev, uint32_t PressedCol, uint32_t PressedRow){
    if (ev == MK_EVENT_PRESSED){
        // load the PressedCol and PressedRow on you global variables and
        // inform the main that a PRESSED event was occoured
    }
    else if (ev == MK_EVENT_RELEASE){
        // inform the main that the key was released
    }
}

// The Main Loop
void main(){
    // ....... 
    HAL_TIM_Base_Start_IT(&htim4);
    while (1){
        // ......
    }

    // ......
}
```

Remember, the callback only will be called for the selected events on eventMask. 

## Have fun

If you followed the guide, the library must works perfectly. Any doubts, you can open an issue, or if you find any problem.