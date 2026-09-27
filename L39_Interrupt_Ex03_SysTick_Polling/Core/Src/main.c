#include "main.h"

// Exercise 3 - SysTick Polling

void delay_ms(uint32_t ms)
{
    // 1 ms per SysTick tick
    SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;

    // Reset counter and COUNTFLAG
    SysTick->VAL = 0;

    // Enable SysTick, no interrupt
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    // Wait for each 1 ms tick
    for (uint32_t i = 0; i < ms; i++)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0)
        {
        }
    }

    // Disable SysTick
    SysTick->CTRL = 0;
}

int main(void)
{
    // Enable GPIOD clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    // PD12 output
    GPIOD->MODER &= ~(3U << (12 * 2));
    GPIOD->MODER |=  (1U << (12 * 2));

    while (1)
    {
        // Toggle LED
        GPIOD->ODR ^= (1U << 12);

        // 1 second delay
        delay_ms(1000);
    }
}
