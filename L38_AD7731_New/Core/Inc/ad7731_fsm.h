/*
 * ad7731_fsm.h
 *
 *  Created on: Sep 16, 2026
 *      Author: ANH DUC
 */

#ifndef INC_AD7731_FSM_H_
#define INC_AD7731_FSM_H_


/* =========================================================
 * FSM API
 * ========================================================= */

void AD7731_FSM_Reset(void);
void AD7731_FSM_Process(void);
uint8_t AD7731_FSM_IsDone(void);
uint8_t AD7731_FSM_IsError(void);


/* =========================================================
 * OPTIONAL DEBUG
 * ========================================================= */

uint8_t AD7731_FSM_GetState(void);

#endif /* INC_AD7731_FSM_H_ */
