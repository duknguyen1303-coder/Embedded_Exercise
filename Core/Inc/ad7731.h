/*
 * ad7731.h
 *
 *  Created on: Sep 16, 2026
 *      Author: ANH DUC
 */

#ifndef INC_AD7731_H_
#define INC_AD7731_H_

#include "stm32f1xx_hal.h"


/* hspi1 is defined in main.c */
extern SPI_HandleTypeDef hspi1;


/* =========================================================
 * SPI ASYNCHRONOUS DRIVER
 * ========================================================= */

/*
 * Start asynchronous 16-bit register write.
 *
 * Example:
 * AD7731_SPI_Write16Async(
 *      AD7731_CMD_FILTER_WRITE,
 *      AD7731_FILTER_VALUE
 * );
 */
HAL_StatusTypeDef AD7731_SPI_Write16Async(uint8_t command,
                                          uint16_t value);


/*
 * Start asynchronous 16-bit register read.
 */
HAL_StatusTypeDef AD7731_SPI_Read16Async(uint8_t command);


/*
 * Start asynchronous 24-bit register read.
 */
HAL_StatusTypeDef AD7731_SPI_Read24Async(uint8_t command);


/* =========================================================
 * SPI TRANSACTION STATUS
 * ========================================================= */

uint8_t AD7731_SPI_IsBusy(void);

uint8_t AD7731_SPI_IsDone(void);

uint8_t AD7731_SPI_IsError(void);

void AD7731_SPI_ClearStatus(void);


/* =========================================================
 * READ DATA
 * ========================================================= */

uint16_t AD7731_SPI_GetRead16(void);

uint32_t AD7731_SPI_GetRead24(void);


/* =========================================================
 * HAL CALLBACK BRIDGE
 * These are called from main.c HAL callbacks.
 * ========================================================= */

void AD7731_SPI_TxCplt(void);

void AD7731_SPI_RxCplt(void);

void AD7731_SPI_Error(void);


/* =========================================================
 * RDY EVENT
 * ========================================================= */

void AD7731_RDY_Callback(void);

uint8_t AD7731_IsRDYEvent(void);

void AD7731_ClearRDYEvent(void);

uint8_t AD7731_IsRDYLow(void);


#endif /* INC_AD7731_H_ */
