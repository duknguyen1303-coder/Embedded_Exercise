/*
 * ad7731_fsm.c
 *
 *  Created on: Sep 16, 2026
 *      Author: ANH DUC
 */
#include "ad7731_fsm.h"

#include "ad7731.h"
#include "ad7731_config.h"


/* =========================================================
 * MAIN FSM STATES
 *
 * 5 MAIN STEPS:
 *
 * 1. FILTER
 * 2. MODE
 * 3. FULL-SCALE CALIBRATION
 * 4. ZERO-SCALE CALIBRATION
 * 5. READ CALIBRATION REGISTERS
 *
 * Each step has smaller states.
 * ========================================================= */

typedef enum
{
    AD7731_FSM_FILTER_START,
    AD7731_FSM_FILTER_WAIT_WRITE,
    AD7731_FSM_FILTER_START_READ,
    AD7731_FSM_FILTER_WAIT_READ,
    AD7731_FSM_FILTER_CHECK,

    AD7731_FSM_MODE_START,
    AD7731_FSM_MODE_WAIT_WRITE,
    AD7731_FSM_MODE_START_READ,
    AD7731_FSM_MODE_WAIT_READ,
    AD7731_FSM_MODE_CHECK,

    AD7731_FSM_FULL_CAL_START,
    AD7731_FSM_FULL_CAL_WAIT_WRITE,
    AD7731_FSM_FULL_CAL_WAIT_HIGH,
    AD7731_FSM_FULL_CAL_WAIT_LOW,

    AD7731_FSM_ZERO_CAL_START,
    AD7731_FSM_ZERO_CAL_WAIT_WRITE,
    AD7731_FSM_ZERO_CAL_WAIT_HIGH,
    AD7731_FSM_ZERO_CAL_WAIT_LOW,

    AD7731_FSM_READ_OFFSET_START,
    AD7731_FSM_READ_OFFSET_WAIT,
    AD7731_FSM_READ_GAIN_START,
    AD7731_FSM_READ_GAIN_WAIT,
    AD7731_FSM_VERIFY,

    AD7731_FSM_DONE,
    AD7731_FSM_ERROR
} AD7731_FSM_State_t;


/* =========================================================
 * INTERNAL VARIABLES
 * ========================================================= */
static AD7731_FSM_State_t fsm_state;
static uint8_t fsm_done = 0;
static uint8_t fsm_error = 0;

/* =========================================================
 * VALUES USED FOR VERIFICATION
 * ========================================================= */
static uint16_t filter_read_value = 0;
static uint16_t mode_read_value = 0;
static uint32_t offset_value = 0;
static uint32_t gain_value = 0;

/* =========================================================
 * RESET FSM
 * ========================================================= */
void AD7731_FSM_Reset(void)
{
    fsm_state = AD7731_FSM_FILTER_START;
    fsm_done = 0;
    fsm_error = 0;

    AD7731_SPI_ClearStatus();
    AD7731_ClearRDYEvent();
}

/* =========================================================
 * FSM PROCESS
 *
 * NON-BLOCKING
 *
 * Every state executes quickly and returns.
 * No HAL_Delay().
 * No while() waiting.
 * ========================================================= */

void AD7731_FSM_Process(void)
{
    switch (fsm_state)
    {
        /* =================================================
         * STEP 1
         * FILTER
         * ================================================= */
        case AD7731_FSM_FILTER_START:
        {
            HAL_StatusTypeDef status;
            status = AD7731_SPI_Write16Async(
                        AD7731_CMD_FILTER_WRITE,
                        AD7731_FILTER_VALUE);
            if (status == HAL_OK)
            {
                fsm_state = AD7731_FSM_FILTER_WAIT_WRITE;
            }
            else if (status != HAL_BUSY)
            {
                fsm_state = AD7731_FSM_ERROR;
            }
            break;
        }

        case AD7731_FSM_FILTER_WAIT_WRITE:

            if (AD7731_SPI_IsError())
            {
                fsm_state = AD7731_FSM_ERROR;
            }

            else if (AD7731_SPI_IsDone())
            {
                AD7731_SPI_ClearStatus();
                fsm_state = AD7731_FSM_FILTER_START_READ;
            }
            break;

        case AD7731_FSM_FILTER_START_READ:
        {
            HAL_StatusTypeDef status;
            status = AD7731_SPI_Read16Async(AD7731_CMD_FILTER_READ);
            if (status == HAL_OK)
            {
                fsm_state = AD7731_FSM_FILTER_WAIT_READ;
            }

            else if (status != HAL_BUSY)
            {
                fsm_state = AD7731_FSM_ERROR;
            }
            break;
        }


        case AD7731_FSM_FILTER_WAIT_READ:

            if (AD7731_SPI_IsError())
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            else if (AD7731_SPI_IsDone())
            {
                filter_read_value =
                    AD7731_SPI_GetRead16();

                AD7731_SPI_ClearStatus();

                fsm_state =
                    AD7731_FSM_FILTER_CHECK;
            }

            break;


        case AD7731_FSM_FILTER_CHECK:

            if (filter_read_value ==
                AD7731_FILTER_VALUE)
            {
                fsm_state =
                    AD7731_FSM_MODE_START;
            }
            else
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            break;


        /* =================================================
         * STEP 2
         * MODE
         * ================================================= */

        case AD7731_FSM_MODE_START:
        {
            HAL_StatusTypeDef status;


            status = AD7731_SPI_Write16Async(
                        AD7731_CMD_MODE_WRITE,
                        AD7731_MODE_VALUE);


            if (status == HAL_OK)
            {
                fsm_state =
                    AD7731_FSM_MODE_WAIT_WRITE;
            }

            else if (status != HAL_BUSY)
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            break;
        }


        case AD7731_FSM_MODE_WAIT_WRITE:

            if (AD7731_SPI_IsError())
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            else if (AD7731_SPI_IsDone())
            {
                AD7731_SPI_ClearStatus();

                fsm_state =
                    AD7731_FSM_MODE_START_READ;
            }

            break;


        case AD7731_FSM_MODE_START_READ:
        {
            HAL_StatusTypeDef status;


            status = AD7731_SPI_Read16Async(
                        AD7731_CMD_MODE_READ);


            if (status == HAL_OK)
            {
                fsm_state =
                    AD7731_FSM_MODE_WAIT_READ;
            }

            else if (status != HAL_BUSY)
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            break;
        }


        case AD7731_FSM_MODE_WAIT_READ:

            if (AD7731_SPI_IsError())
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            else if (AD7731_SPI_IsDone())
            {
                mode_read_value =
                    AD7731_SPI_GetRead16();

                AD7731_SPI_ClearStatus();

                fsm_state =
                    AD7731_FSM_MODE_CHECK;
            }

            break;


        case AD7731_FSM_MODE_CHECK:

            if (mode_read_value ==
                AD7731_MODE_VALUE)
            {
                fsm_state =
                    AD7731_FSM_FULL_CAL_START;
            }
            else
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            break;


        /* =================================================
         * STEP 3
         * FULL-SCALE CALIBRATION
         * ================================================= */

        case AD7731_FSM_FULL_CAL_START:
        {
            HAL_StatusTypeDef status;


            /*
             * Clear previous RDY event before starting
             * calibration.
             */

            AD7731_ClearRDYEvent();


            status = AD7731_SPI_Write16Async(
                        AD7731_CMD_MODE_WRITE,
                        AD7731_FULL_CAL_MODE);


            if (status == HAL_OK)
            {
                fsm_state =
                    AD7731_FSM_FULL_CAL_WAIT_WRITE;
            }

            else if (status != HAL_BUSY)
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            break;
        }


        case AD7731_FSM_FULL_CAL_WAIT_WRITE:

            if (AD7731_SPI_IsError())
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            else if (AD7731_SPI_IsDone())
            {
                AD7731_SPI_ClearStatus();

                /*
                 * Calibration should drive RDY HIGH first.
                 */

                fsm_state =
                    AD7731_FSM_FULL_CAL_WAIT_HIGH;
            }

            break;


        case AD7731_FSM_FULL_CAL_WAIT_HIGH:

            if (!AD7731_IsRDYLow())
            {
                /*
                 * RDY is HIGH.
                 * Now wait for LOW.
                 */

                AD7731_ClearRDYEvent();

                fsm_state =
                    AD7731_FSM_FULL_CAL_WAIT_LOW;
            }

            break;


        case AD7731_FSM_FULL_CAL_WAIT_LOW:

            if (AD7731_IsRDYEvent() ||
                AD7731_IsRDYLow())
            {
                AD7731_ClearRDYEvent();

                fsm_state =
                    AD7731_FSM_ZERO_CAL_START;
            }

            break;


        /* =================================================
         * STEP 4
         * ZERO-SCALE CALIBRATION
         * ================================================= */

        case AD7731_FSM_ZERO_CAL_START:
        {
            HAL_StatusTypeDef status;


            AD7731_ClearRDYEvent();


            status = AD7731_SPI_Write16Async(
                        AD7731_CMD_MODE_WRITE,
                        AD7731_ZERO_CAL_MODE);


            if (status == HAL_OK)
            {
                fsm_state =
                    AD7731_FSM_ZERO_CAL_WAIT_WRITE;
            }

            else if (status != HAL_BUSY)
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            break;
        }


        case AD7731_FSM_ZERO_CAL_WAIT_WRITE:

            if (AD7731_SPI_IsError())
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            else if (AD7731_SPI_IsDone())
            {
                AD7731_SPI_ClearStatus();

                fsm_state =
                    AD7731_FSM_ZERO_CAL_WAIT_HIGH;
            }

            break;


        case AD7731_FSM_ZERO_CAL_WAIT_HIGH:

            if (!AD7731_IsRDYLow())
            {
                AD7731_ClearRDYEvent();

                fsm_state =
                    AD7731_FSM_ZERO_CAL_WAIT_LOW;
            }

            break;


        case AD7731_FSM_ZERO_CAL_WAIT_LOW:

            if (AD7731_IsRDYEvent() ||
                AD7731_IsRDYLow())
            {
                AD7731_ClearRDYEvent();

                fsm_state =
                    AD7731_FSM_READ_OFFSET_START;
            }

            break;


        /* =================================================
         * STEP 5
         * READ OFFSET
         * ================================================= */

        case AD7731_FSM_READ_OFFSET_START:
        {
            HAL_StatusTypeDef status;


            status = AD7731_SPI_Read24Async(
                        AD7731_CMD_OFFSET_READ);


            if (status == HAL_OK)
            {
                fsm_state =
                    AD7731_FSM_READ_OFFSET_WAIT;
            }

            else if (status != HAL_BUSY)
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            break;
        }


        case AD7731_FSM_READ_OFFSET_WAIT:

            if (AD7731_SPI_IsError())
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            else if (AD7731_SPI_IsDone())
            {
                offset_value =
                    AD7731_SPI_GetRead24();

                AD7731_SPI_ClearStatus();

                fsm_state =
                    AD7731_FSM_READ_GAIN_START;
            }

            break;


        /* =================================================
         * READ GAIN
         * ================================================= */

        case AD7731_FSM_READ_GAIN_START:
        {
            HAL_StatusTypeDef status;


            status = AD7731_SPI_Read24Async(
                        AD7731_CMD_GAIN_READ);


            if (status == HAL_OK)
            {
                fsm_state =
                    AD7731_FSM_READ_GAIN_WAIT;
            }

            else if (status != HAL_BUSY)
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            break;
        }


        case AD7731_FSM_READ_GAIN_WAIT:

            if (AD7731_SPI_IsError())
            {
                fsm_state =
                    AD7731_FSM_ERROR;
            }

            else if (AD7731_SPI_IsDone())
            {
                gain_value =
                    AD7731_SPI_GetRead24();

                AD7731_SPI_ClearStatus();

                fsm_state =
                    AD7731_FSM_VERIFY;
            }

            break;


        /* =================================================
         * VERIFY
         * ================================================= */

        case AD7731_FSM_VERIFY:

            /*
             * At this stage:
             *
             * Filter register was written/read correctly.
             * Mode register was written/read correctly.
             * Full-scale calibration completed.
             * Zero-scale calibration completed.
             * Offset register was successfully read.
             * Gain register was successfully read.
             *
             * We DO NOT judge calibration validity merely
             * by offset_value != 0 or gain_value != 0.
             */

            fsm_state =
                AD7731_FSM_DONE;

            break;


        /* =================================================
         * DONE
         * ================================================= */

        case AD7731_FSM_DONE:

            fsm_done = 1;

            break;


        /* =================================================
         * ERROR
         * ================================================= */

        case AD7731_FSM_ERROR:

            fsm_error = 1;

            break;


        default:

            fsm_state =
                AD7731_FSM_ERROR;

            break;
    }
}


/* =========================================================
 * FSM STATUS
 * ========================================================= */

uint8_t AD7731_FSM_IsDone(void)
{
    return fsm_done;
}

uint8_t AD7731_FSM_IsError(void)
{
    return fsm_error;
}

uint8_t AD7731_FSM_GetState(void)
{
    return (uint8_t)fsm_state;
}

