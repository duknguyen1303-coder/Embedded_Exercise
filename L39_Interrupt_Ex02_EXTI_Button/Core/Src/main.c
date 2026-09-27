#include "main.h"

volatile uint8_t press_count = 0;

void EXTI0_IRQHandler(void)
{
	if (EXTI->PR & (1U << 0))
	{
		press_count++;

		/* // Output counter to PB0-PB7 */
		GPIOB->ODR = (GPIOB->ODR & ~0xFFU) | press_count;

		// Clear pending flag
		EXTI->PR |= (1U << 0);
	}
}

int main(void)
{
	// Enable GPIOA, GPIOB, SYSCFG clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	// PA0 input
	GPIOA->MODER &= ~(3U << 0);

	// PB0-PB7 output
	for (int pin = 0; pin < 8; pin++)
	{
		GPIOB->MODER &= ~(3U << (pin * 2));
		GPIOB->MODER |=  (1U << (pin * 2));
	}

	// EXTI0 -> PA0
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;

	// Rising edge
	EXTI->RTSR |= (1U << 0);
	EXTI->FTSR &= ~(1U << 0);

	// Unmask EXTI0
	EXTI->IMR |= (1U << 0);

	// Clear pending flag
	EXTI->PR = (1U << 0);

	//Enable EXTI0 IRQ
	NVIC->ISER[0] |= (1U << 6);
	NVIC->IP[EXTI0_IRQn] = (5U << 4);   // priority = 5 (tuy chon)

	while(1)
	{

	}
}
