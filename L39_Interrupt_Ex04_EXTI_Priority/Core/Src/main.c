#include "main.h"

// Exercise 4 - Interrupt Priority & Nesting
// Button 1: PA0 -> EXTI0 -> Red LED (PD14)
// Button 2: PA1 -> EXTI1 -> Green LED (PD12)

#define SWAP_PRIORITY 0

void delay_ms(uint32_t ms)
{
    // SysTick polling delay
    SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    for (uint32_t i = 0; i < ms; i++)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
    }

    SysTick->CTRL = 0;
}

// EXTI0: long ISR
void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & (1U << 0))
    {
        GPIOD->ODR |= (1U << 14);     // Turn on red LED
        delay_ms(3000);               // Keep ISR busy for 3 seconds
        GPIOD->ODR &= ~(1U << 14);    // Turn off red LED

        EXTI->PR |= (1U << 0);        // Clear pending flag
    }
}

// EXTI1: short ISR
void EXTI1_IRQHandler(void)
{
    if (EXTI->PR & (1U << 1))
    {
        GPIOD->ODR ^= (1U << 12);     // Toggle green LED
        EXTI->PR |= (1U << 1);        // Clear pending flag
    }
}

int main(void)
{
    // Enable GPIOA, GPIOD and SYSCFG clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // PA0 input
    GPIOA->MODER &= ~(3U << (0 * 2));

    // PA1 input with pull-down
    GPIOA->MODER &= ~(3U << (1 * 2));
    GPIOA->PUPDR &= ~(3U << (1 * 2));
    GPIOA->PUPDR |=  (2U << (1 * 2));

    // PD12 and PD14 output
    GPIOD->MODER &= ~(3U << (12 * 2));
    GPIOD->MODER |=  (1U << (12 * 2));

    GPIOD->MODER &= ~(3U << (14 * 2));
    GPIOD->MODER |=  (1U << (14 * 2));

    // PA0 -> EXTI0, PA1 -> EXTI1
    SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI0 |
                           SYSCFG_EXTICR1_EXTI1);

    // Rising edge for both buttons
    EXTI->RTSR |= (1U << 0) | (1U << 1);
    EXTI->FTSR &= ~((1U << 0) | (1U << 1));

    // Enable EXTI0 and EXTI1
    EXTI->IMR |= (1U << 0) | (1U << 1);

    // Clear pending flags
    EXTI->PR = (1U << 0) | (1U << 1);

    // Set priority grouping
    SCB->AIRCR = (0x5FAU << SCB_AIRCR_VECTKEY_Pos) |
                 (3U << SCB_AIRCR_PRIGROUP_Pos);

    // Set interrupt priorities
#if SWAP_PRIORITY
    NVIC->IP[EXTI0_IRQn] = (0U << 4);
    NVIC->IP[EXTI1_IRQn] = (1U << 4);
#else
    NVIC->IP[EXTI0_IRQn] = (1U << 4);
    NVIC->IP[EXTI1_IRQn] = (0U << 4);
#endif

    // Enable EXTI0 (IRQ6) and EXTI1 (IRQ7)
    NVIC->ISER[0] |= (1U << 6) | (1U << 7);

    while (1)
    {
    }
}
