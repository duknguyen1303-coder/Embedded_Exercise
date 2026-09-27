#include "main.h"

// Exercise 3 - SysTick Interrupt

volatile uint32_t ms_counter = 0;

void SysTick_Handler(void)
{
    ms_counter++;

    // Toggle LED every 1 second
    if (ms_counter >= 1000)
    {
        ms_counter = 0;
        GPIOD->ODR ^= (1U << 12);
    }
}

int main(void)
{
    // Enable GPIOD clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    // PD12 output
    GPIOD->MODER &= ~(3U << (12 * 2));
    GPIOD->MODER |=  (1U << (12 * 2));

    // 1 ms SysTick period
    SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;
    SysTick->VAL = 0;

    // Enable SysTick interrupt
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk
                  | SysTick_CTRL_TICKINT_Msk
                  | SysTick_CTRL_ENABLE_Msk;

    while (1)
    {
    }
}
