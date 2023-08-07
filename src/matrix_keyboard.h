/*
 * matrix_keyboard.h
 *
 *  Created on: Aug 6, 2023
 *      Author: pablo-jean
 */

#ifndef MATRIX_KEYBOARD_H_
#define MATRIX_KEYBOARD_H_

#include <stdint.h>
#include <stddef.h>

#include "mk_platform.h"

/**
 * Macros
 */


/**
 * Enumerates
 */

typedef enum{
	MK_EVENT_NONE = 0x0,
	MK_EVENT_RELEASED = 0x1,
	MK_EVENT_PRESSED = 0x2,

	MK_EVENT_ALL = 0xF
}mk_event_e;

typedef enum{
	MK_ACTIVE_LEVEL_LOW,
	MK_ACTIVE_LEVEL_HIGH
}mk_active_level_e;

typedef enum{
	MK_STATUS_OK,
	MK_STATUS_FAIL,
	MK_STATUS_INCORRET_PARAMS,
	MK_STATUS_NOT_INITIALIZED,

	MK_STATUS_UNKNOWN = 0xFF
}mk_status_e;

/**
 * Typedefs
 */

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
	uint8_t _mkInit;
	uint32_t _actCol;
	uint32_t _actRow;
	uint32_t _prevColState;
	uint32_t _prevRowState;
	uint32_t _testCol;
	mk_gpio_state_e _onSignal;
	mk_gpio_state_e _offSignal;
}mk_t;

/**
 * Publics
 */

mk_status_e mk_init(mk_t *Mk);

mk_status_e mk_get_button(mk_t *Mk, uint32_t *pPressedCol, uint32_t *pPressedRow);

mk_status_e mk_DoOperation(mk_t *Mk, mk_event_e *Ev);

// Callback

void mk_callback(mk_t *Mk, mk_event_e ev, uint32_t PressedCol, uint32_t PressedRow);

#endif /* MATRIX_KEYBOARD_H_ */
