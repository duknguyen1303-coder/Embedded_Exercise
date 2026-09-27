/* ---- Exercise 1: Enable EXTI0_IRQn(6), TIM2_IRQn(28), USART1_IRQn(37) ----
 * Truy cap truc tiep NVIC->ISER[], KHONG dung NVIC_EnableIRQ() */

#include "main.h"

int main(void)
{

	/* EXTI0_IRQn = 6  -> ISER index = 6/32  = 0, bit = 6%32  = 6  */
	NVIC->ISER[0] |= (1U << 6);

	/* TIM2_IRQn  = 28 -> ISER index = 28/32 = 0, bit = 28%32 = 28 */
	NVIC->ISER[0] |= (1U << 28);

	/* USART1_IRQn = 37 -> ISER index = 37/32 = 1, bit = 37%32 = 5
	 * (37 >= 32 nen IRQ nay nam o ISER[1], khong con o ISER[0]) */
	NVIC->ISER[1] |= (1U << 5);

	while(1)
	{

	}
}
