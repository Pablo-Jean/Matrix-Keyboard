/*
 * matrix_keyboard.c
 *
 *  Created on: Aug 6, 2023
 *      Author: pablo-jean
 */

/**
 * Includes
 */

#include "matrix_keyboard.h"
#include <cmsis_gcc.h>

/**
 * Macros
 */

#define _DISABLE_COLUMN(mk, col)  	_mk_gpio_write(&mk->gCols[mk->_testCol], Mk->_offSignal)
#define _ENABLE_COLUMN(mk, col)  	_mk_gpio_write(&mk->gCols[mk->_testCol], Mk->_onSignal)


/**
 * Privates
 */

#define _MK_INIT_MASK	0x3A



/**
 * Publics
 */

mk_status_e mk_init(mk_t *Mk){
	uint32_t i;
	if (Mk == NULL)
		return MK_STATUS_INCORRET_PARAMS;
	if (Mk->gCols == NULL || Mk->gRows == NULL)
		return MK_STATUS_INCORRET_PARAMS;
	if (Mk->nCols < 2 || Mk->nRows < 2)
		return MK_STATUS_INCORRET_PARAMS;

	if (Mk->actLevel == MK_ACTIVE_LEVEL_LOW){
		Mk->_onSignal = MK_GPIO_LOW;
		Mk->_offSignal = MK_GPIO_HIGH;
	}
	else{
		Mk->_onSignal = MK_GPIO_HIGH;
		Mk->_offSignal = MK_GPIO_LOW;
	}
	for (i=0 ; i<Mk->nCols ; i++){
		_DISABLE_COLUMN(Mk, i);
	}
	Mk->_actCol = 0;
	Mk->_actRow = 0;
	Mk->_prevColState = 0;
	Mk->_prevRowState = 0;
	Mk->_testCol = 0;
	_ENABLE_COLUMN(Mk, Mk->_testCol);

	Mk->_mkInit = _MK_INIT_MASK;
	return MK_STATUS_OK;
}

mk_status_e mk_get_button(mk_t *Mk, uint32_t *pPressedCol, uint32_t *pPressedRow){
	if (Mk == NULL || pPressedCol == NULL || pPressedRow == NULL)
		return MK_STATUS_INCORRET_PARAMS;
	if (Mk->_mkInit != _MK_INIT_MASK){
		return MK_STATUS_NOT_INITIALIZED;
	}

	*pPressedCol = Mk->_actCol;
	*pPressedRow = Mk->_actRow;

	return MK_STATUS_OK;
}

mk_status_e mk_DoOperation(mk_t *Mk, mk_event_e *Ev){
	uint32_t i;
	uint8_t detectedBtn = 0;
	mk_event_e ev = MK_EVENT_NONE;

	if (Mk == NULL)
		return MK_STATUS_INCORRET_PARAMS;
	if (Mk->_mkInit != _MK_INIT_MASK){
		return MK_STATUS_NOT_INITIALIZED;
	}

	if (Mk->_testCol > Mk->nCols){
		Mk->_testCol = 0;
		_ENABLE_COLUMN(Mk, 0);

		return MK_STATUS_OK;
	}

	for (i=0 ; i<Mk->nRows ; i++){
		if (_mk_gpio_read(&Mk->gRows[i]) == Mk->_onSignal){
			Mk->_prevColState = Mk->_actCol;
			Mk->_prevRowState = Mk->_actRow;
			Mk->_actCol = (Mk->_testCol+1);
			Mk->_actRow = (i+1);
			detectedBtn = 1;
			break;
		}
	}

	_DISABLE_COLUMN(Mk, Mk->_testCol);

	if (detectedBtn == 1){
		Mk->_testCol = 0;
		if (Mk->_actCol != Mk->_prevColState || Mk->_actRow != Mk->_prevRowState){
			ev = MK_EVENT_PRESSED;
		}
	}
	else{
		Mk->_testCol++;
		if (Mk->_testCol == Mk->nCols){
			Mk->_prevColState = Mk->_actCol;
			Mk->_prevRowState = Mk->_actRow;
			Mk->_actCol = 0;
			Mk->_actRow = 0;
			if (Mk->_prevColState != 0 || Mk->_prevRowState != 0){
				ev = MK_EVENT_RELEASED;
			}
			Mk->_testCol = 0;
		}
	}

	_ENABLE_COLUMN(Mk, Mk->_testCol);

	if (ev & Mk->eventMask){
		mk_callback(Mk, ev, Mk->_actCol, Mk->_actRow);
	}

	if (Ev != NULL){
		*Ev = ev;
	}

	return MK_STATUS_OK;
}

// Callback

void __WEAK mk_callback(mk_t *Mk, mk_event_e ev, uint32_t PressedCol, uint32_t PressedRow){
	// Do not
	// The source code must be implemented by the developer, if needed
}
