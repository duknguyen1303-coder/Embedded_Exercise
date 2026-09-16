/*
 * ad7731.c
 *
 *  Created on: Sep 16, 2026
 *      Author: ANH DUC
 */

#include "ad7731.h"

/* ============================================================
 * AD7731 SPI DRIVER
 * ------------------------------------------------------------
 * SPI1:
 *   PA5 -> SCLK
 *   PA6 -> DOUT (MISO)
 *   PA7 -> DIN  (MOSI)
 *
 * AD7731:
 *   CS tied LOW
 *   RDY -> PB14
 *
 * Driver sử dụng SPI interrupt:
 *   HAL_SPI_Transmit_IT()
 *   HAL_SPI_Receive_IT()
 *
 * Transaction:
 *   1. Write16:
 *      Send Communications Register command
 *      -> Send 16-bit data
 *
 *   2. Read16:
 *      Send Communications Register command
 *      -> Receive 16-bit data
 *
 *   3. Read24:
 *      Send Communications Register command
 *      -> Receive 24-bit data
 *
 * ============================================================ */


/* ============================================================
 * 1. INTERNAL SPI STATE
 * ============================================================ */

typedef enum
{
    AD7731_SPI_IDLE = 0,
    AD7731_SPI_TX_COMMAND,
    AD7731_SPI_TX_DATA,
    AD7731_SPI_RX_DATA,
    AD7731_SPI_ERROR

} AD7731_SPI_State_t;


/* ============================================================
 * 2. INTERNAL TRANSACTION TYPE
 * ============================================================
 *
 * Không được xác định READ/WRITE bằng command & 0x0F
 * vì register address có thể giống nhau giữa READ và WRITE.
 *
 * Ví dụ:
 *
 *   Write MODE  = 0x02
 *   Read MODE   = 0x12
 *
 * nhưng:
 *
 *   0x02 & 0x0F = 0x02
 *   0x12 & 0x0F = 0x02
 *
 * Vì vậy phải lưu loại transaction riêng.
 *
 * ============================================================ */
typedef enum
{
    AD7731_SPI_TRANSACTION_WRITE = 0,
    AD7731_SPI_TRANSACTION_READ
} AD7731_SPI_TransactionType_t;

/* ============================================================
 * 3. INTERNAL VARIABLES
 * ============================================================ */
static volatile AD7731_SPI_State_t spi_state = AD7731_SPI_IDLE;
static volatile AD7731_SPI_TransactionType_t spi_transaction = AD7731_SPI_TRANSACTION_WRITE;
/* SPI status */
static volatile uint8_t spi_busy  = 0;
static volatile uint8_t spi_done  = 0;
static volatile uint8_t spi_error = 0;
/* Current command */
static uint8_t spi_command = 0;
/* TX buffer */
static uint8_t spi_tx_data[2];
/* RX buffer */
static uint8_t spi_rx_data[3];
/* Number of bytes expected */
static uint8_t spi_rx_length = 0;
/* Received values */
static uint16_t spi_read16_value = 0;
static uint32_t spi_read24_value = 0;


/* ============================================================
 * 4. RDY EVENT
 * ============================================================ */

static volatile uint8_t ad7731_rdy_event = 0;

/* ============================================================
 * 5. WRITE 16-BIT REGISTER ASYNCHRONOUSLY
 * ============================================================
 * * Sequence:
 *
 *   IDLE
 *    |
 *    | Write16Async()
 *    v
 *   TX_COMMAND
 *    |
 *    | command transmitted
 *    v
 *   TX_DATA
 *    |
 *    | 2 data bytes transmitted
 *    v
 *   IDLE + DONE
 *
 * ============================================================ */

HAL_StatusTypeDef AD7731_SPI_Write16Async(
        uint8_t command,
        uint16_t value)
{
    HAL_StatusTypeDef status;

    /* ----------------------------------------
     * SPI đang bận
     * ---------------------------------------- */

    if (spi_busy)
    {
        return HAL_BUSY;
    }

    /* ----------------------------------------
     * Clear trạng thái cũ
     * ---------------------------------------- */
    spi_done  = 0;
    spi_error = 0;

    /* ----------------------------------------
     * Mark SPI busy
     * ---------------------------------------- */
    spi_busy = 1;

    /* ----------------------------------------
     * Save transaction information
     * ---------------------------------------- */
    spi_transaction = AD7731_SPI_TRANSACTION_WRITE;
    spi_command = command;

    /* ----------------------------------------
     * Convert 16-bit value -> 2 bytes
     *
     * AD7731 nhận MSB trước.
     * ---------------------------------------- */
    spi_tx_data[0] = (uint8_t)(value >> 8);
    spi_tx_data[1] = (uint8_t)(value & 0xFF);

    /* ----------------------------------------
     * Start with Communications Register
     * command
     * ---------------------------------------- */
    spi_state = AD7731_SPI_TX_COMMAND;
    status =
        HAL_SPI_Transmit_IT(
            &hspi1,
            &spi_command,
            1);

    /* ----------------------------------------
     * Start failed
     * ---------------------------------------- */

    if (status != HAL_OK)
    {
        spi_busy  = 0;
        spi_error = 1;

        spi_state =
            AD7731_SPI_ERROR;
    }
    return status;
}


/* ============================================================
 * 6. READ 16-BIT REGISTER ASYNCHRONOUSLY
 * ============================================================
 *
 * Sequence:
 *
 *   IDLE
 *    |
 *    | Read16Async()
 *    v
 *   TX_COMMAND
 *    |
 *    | command transmitted
 *    v
 *   RX_DATA
 *    |
 *    | 2 bytes received
 *    v
 *   IDLE + DONE
 *
 * ============================================================ */

HAL_StatusTypeDef AD7731_SPI_Read16Async(
        uint8_t command)
{
    HAL_StatusTypeDef status;


    /* ----------------------------------------
     * SPI đang bận
     * ---------------------------------------- */

    if (spi_busy)
    {
        return HAL_BUSY;
    }


    /* ----------------------------------------
     * Clear trạng thái cũ
     * ---------------------------------------- */

    spi_done  = 0;
    spi_error = 0;


    /* ----------------------------------------
     * Mark busy
     * ---------------------------------------- */

    spi_busy = 1;


    /* ----------------------------------------
     * Transaction = READ
     * ---------------------------------------- */

    spi_transaction =
            AD7731_SPI_TRANSACTION_READ;


    /* ----------------------------------------
     * Save command
     * ---------------------------------------- */

    spi_command = command;


    /* ----------------------------------------
     * Expect 2 bytes
     * ---------------------------------------- */

    spi_rx_length = 2;


    /* ----------------------------------------
     * Start Communications Register command
     * ---------------------------------------- */

    spi_state =
            AD7731_SPI_TX_COMMAND;


    status =
        HAL_SPI_Transmit_IT(
            &hspi1,
            &spi_command,
            1);


    /* ----------------------------------------
     * Start failed
     * ---------------------------------------- */

    if (status != HAL_OK)
    {
        spi_busy  = 0;
        spi_error = 1;

        spi_state =
            AD7731_SPI_ERROR;
    }


    return status;
}


/* ============================================================
 * 7. READ 24-BIT REGISTER ASYNCHRONOUSLY
 * ============================================================ */

HAL_StatusTypeDef AD7731_SPI_Read24Async(
        uint8_t command)
{
    HAL_StatusTypeDef status;


    /* ----------------------------------------
     * SPI đang bận
     * ---------------------------------------- */

    if (spi_busy)
    {
        return HAL_BUSY;
    }


    /* ----------------------------------------
     * Clear trạng thái cũ
     * ---------------------------------------- */

    spi_done  = 0;
    spi_error = 0;


    /* ----------------------------------------
     * Mark busy
     * ---------------------------------------- */

    spi_busy = 1;


    /* ----------------------------------------
     * Transaction = READ
     * ---------------------------------------- */

    spi_transaction =
            AD7731_SPI_TRANSACTION_READ;


    /* ----------------------------------------
     * Save command
     * ---------------------------------------- */

    spi_command = command;


    /* ----------------------------------------
     * Expect 3 bytes
     * ---------------------------------------- */

    spi_rx_length = 3;


    /* ----------------------------------------
     * Start Communications Register command
     * ---------------------------------------- */

    spi_state =
            AD7731_SPI_TX_COMMAND;


    status =
        HAL_SPI_Transmit_IT(
            &hspi1,
            &spi_command,
            1);


    /* ----------------------------------------
     * Start failed
     * ---------------------------------------- */

    if (status != HAL_OK)
    {
        spi_busy  = 0;
        spi_error = 1;

        spi_state =
            AD7731_SPI_ERROR;
    }


    return status;
}


/* ============================================================
 * 8. SPI TX COMPLETE CALLBACK
 * ============================================================
 *
 * Hàm này được gọi từ:
 *
 *   HAL_SPI_TxCpltCallback()
 *
 * trong main.c.
 *
 * ============================================================ */

void AD7731_SPI_TxCplt(void)
{
    HAL_StatusTypeDef status;


    /* ----------------------------------------
     * Nếu driver không busy thì bỏ qua
     * ---------------------------------------- */

    if (!spi_busy)
    {
        return;
    }


    /* ========================================================
     * CASE 1:
     * Communications Register command vừa gửi xong
     * ======================================================== */

    if (spi_state ==
            AD7731_SPI_TX_COMMAND)
    {

        /* ====================================================
         * READ TRANSACTION
         *
         * Sau command -> nhận data.
         * ==================================================== */

        if (spi_transaction ==
                AD7731_SPI_TRANSACTION_READ)
        {
            spi_state =
                    AD7731_SPI_RX_DATA;


            status =
                HAL_SPI_Receive_IT(
                    &hspi1,
                    spi_rx_data,
                    spi_rx_length);


            /* --------------------------------
             * Receive start failed
             * -------------------------------- */

            if (status != HAL_OK)
            {
                spi_busy  = 0;
                spi_error = 1;

                spi_state =
                    AD7731_SPI_ERROR;
            }


            return;
        }


        /* ====================================================
         * WRITE TRANSACTION
         *
         * Sau command -> gửi data.
         * ==================================================== */

        spi_state =
                AD7731_SPI_TX_DATA;


        status =
            HAL_SPI_Transmit_IT(
                &hspi1,
                spi_tx_data,
                2);


        /* --------------------------------
         * TX data start failed
         * -------------------------------- */

        if (status != HAL_OK)
        {
            spi_busy  = 0;
            spi_error = 1;

            spi_state =
                AD7731_SPI_ERROR;
        }


        return;
    }


    /* ========================================================
     * CASE 2:
     * 16-bit data hoặc 2-byte data đã gửi xong
     * ======================================================== */

    else if (spi_state ==
             AD7731_SPI_TX_DATA)
    {
        spi_busy = 0;

        spi_done = 1;

        spi_state =
            AD7731_SPI_IDLE;
    }
}


/* ============================================================
 * 9. SPI RX COMPLETE CALLBACK
 * ============================================================
 *
 * Hàm này được gọi từ:
 *
 *   HAL_SPI_RxCpltCallback()
 *
 * trong main.c.
 *
 * ============================================================ */

void AD7731_SPI_RxCplt(void)
{
    /* ----------------------------------------
     * Nếu driver không busy thì bỏ qua
     * ---------------------------------------- */

    if (!spi_busy)
    {
        return;
    }
    /* ========================================================
     * READ 16-BIT
     * ======================================================== */
    if (spi_rx_length == 2)
    {
        spi_read16_value =
                ((uint16_t)spi_rx_data[0] << 8)
                |
                ((uint16_t)spi_rx_data[1]);
    }
    /* ========================================================
     * READ 24-BIT
     * ======================================================== */
    else if (spi_rx_length == 3)
    {
        spi_read24_value =
                ((uint32_t)spi_rx_data[0] << 16)
                |
                ((uint32_t)spi_rx_data[1] << 8)
                |
                ((uint32_t)spi_rx_data[2]);
    }
    /* ----------------------------------------
     * Transaction completed
     * ---------------------------------------- */
    spi_busy = 0;

    spi_done = 1;

    spi_state =
        AD7731_SPI_IDLE;
}
/* ============================================================
 * 10. SPI ERROR CALLBACK
 * ============================================================ */
void AD7731_SPI_Error(void)
{
    spi_busy  = 0;
    spi_done  = 0;
    spi_error = 1;

    spi_state =
        AD7731_SPI_ERROR;
}
/* ============================================================
 * 11. SPI STATUS API
 * ============================================================ */
uint8_t AD7731_SPI_IsBusy(void)
{
    return spi_busy;
}
uint8_t AD7731_SPI_IsDone(void)
{
    return spi_done;
}
uint8_t AD7731_SPI_IsError(void)
{
    return spi_error;
}
/* ============================================================
 * 12. CLEAR SPI STATUS
 * ============================================================ */
void AD7731_SPI_ClearStatus(void)
{
    spi_done  = 0;
    spi_error = 0;
}

/* ============================================================
 * 13. GET RECEIVED 16-BIT VALUE
 * ============================================================ */

uint16_t AD7731_SPI_GetRead16(void)
{
    return spi_read16_value;
}

/* ============================================================
 * 14. GET RECEIVED 24-BIT VALUE
 * ============================================================ */

uint32_t AD7731_SPI_GetRead24(void)
{
    return spi_read24_value;
}

/* ============================================================
 * 15. RDY INTERRUPT CALLBACK
 * ============================================================
 *
 * PB14 = AD7731 RDY
 *
 * Khi RDY tạo falling edge:
 *
 *   EXTI
 *     ↓
 *   HAL_GPIO_EXTI_Callback()
 *     ↓
 *   AD7731_RDY_Callback()
 *     ↓
 *   ad7731_rdy_event = 1
 *
 * FSM sẽ kiểm tra event này.
 *
 * ============================================================ */

void AD7731_RDY_Callback(void)
{
    ad7731_rdy_event = 1;
}

/* ============================================================
 * 16. CHECK RDY EVENT
 * ============================================================ */
uint8_t AD7731_IsRDYEvent(void)
{
    return ad7731_rdy_event;
}

/* ============================================================
 * 17. CLEAR RDY EVENT
 * ============================================================ */
void AD7731_ClearRDYEvent(void)
{
    ad7731_rdy_event = 0;
}

/* ============================================================
 * 18. READ PHYSICAL RDY PIN
 * ============================================================ */
uint8_t AD7731_IsRDYLow(void)
{
    return
        (HAL_GPIO_ReadPin(
            GPIOB,
            GPIO_PIN_14)
        == GPIO_PIN_RESET);
}
