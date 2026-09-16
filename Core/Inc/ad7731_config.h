/*
 * ad7731_config.h
 *
 * Created on: Sep 16, 2026
 * Author: ANH DUC
 */

#ifndef INC_AD7731_CONFIG_H_
#define INC_AD7731_CONFIG_H_

/* =========================================================
 * AD7731 COMMUNICATION REGISTER COMMAND
 * ========================================================= */

/* Read commands */
#define AD7731_CMD_STATUS_READ       0x10
#define AD7731_CMD_DATA_READ         0x11
#define AD7731_CMD_MODE_READ         0x12
#define AD7731_CMD_FILTER_READ       0x13
#define AD7731_CMD_OFFSET_READ       0x15
#define AD7731_CMD_GAIN_READ         0x16

/* Write commands */
#define AD7731_CMD_MODE_WRITE        0x02
#define AD7731_CMD_FILTER_WRITE      0x03
#define AD7731_CMD_OFFSET_WRITE      0x05
#define AD7731_CMD_GAIN_WRITE        0x06


/* =========================================================
 * AD7731 CONFIGURATION VALUES
 * ========================================================= */

#define AD7731_FILTER_VALUE           0x3424
#define AD7731_MODE_VALUE             0x0194

/* Calibration modes */
#define AD7731_FULL_CAL_MODE          0xA1B4
#define AD7731_ZERO_CAL_MODE          0x8194


#endif /* INC_AD7731_CONFIG_H_ */
