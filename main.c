#include <stdint.h>
#include "tm4c123gh6pm.h"

// LED pins
#define RED_LED   (1U<<1)   // PF1
#define BLUE_LED  (1U<<2)   // PF2
#define GREEN_LED (1U<<3)   // PF3

// Switch pins
#define SW2       (1U<<0)   // PF0
#define SW1       (1U<<4)   // PF4

// SysTick Registers
#define STCTRL    (*((volatile uint32_t *)0xE000E010))
#define STRELOAD  (*((volatile uint32_t *)0xE000E014))
#define STCURRENT (*((volatile uint32_t *)0xE000E018))

// Function Prototypes
void PortF_Init(void);
void SysTick_Init(uint32_t reloadVal);
void GPIOF_Handler(void);

// ------------------------------------------------------------
// Initialize GPIO Port F
// ------------------------------------------------------------
void PortF_Init(void)
{
    volatile unsigned long delay;

    SYSCTL_RCGCGPIO_R |= 0x20;        // Enable clock for Port F
    delay = SYSCTL_RCGCGPIO_R;        // Allow time for clock to start

    // Unlock PF0 for SW2 (it is locked by default)
    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R |= (SW1 | SW2);
    GPIO_PORTF_LOCK_R = 0;

    // Set direction: LEDs output, Switches input
    GPIO_PORTF_DIR_R |= (RED_LED | BLUE_LED | GREEN_LED);
    GPIO_PORTF_DIR_R &= ~(SW1 | SW2);

    // Enable digital function
    GPIO_PORTF_DEN_R |= (RED_LED | BLUE_LED | GREEN_LED | SW1 | SW2);

    // Enable pull-ups for switches
    GPIO_PORTF_PUR_R |= (SW1 | SW2);

    // Configure interrupts for switches
    // Configure interrupts for switches
        GPIO_PORTF_IS_R &= ~(SW1 | SW2);   // Edge sensitive
        GPIO_PORTF_IBE_R &= ~(SW1 | SW2);  // Not both edges
        GPIO_PORTF_IEV_R |= (SW1 | SW2);  // Falling edge
        GPIO_PORTF_ICR_R |= (SW1 | SW2);   // Clear any prior interrupt
        GPIO_PORTF_IM_R  |= (SW1 | SW2);   // Unmask interrupt




    // Enable Port F interrupt in NVIC
    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF3FFFFF) | (3<<21); // priority 3
    NVIC_EN0_R |= (1<<30);    // Enable IRQ30 (Port F)
}

// ------------------------------------------------------------
// SysTick Initialization
// ------------------------------------------------------------
void SysTick_Init(uint32_t reloadVal)
{
    STCTRL = 0;                // Disable SysTick during setup
    STRELOAD = reloadVal - 1;  // Reload value
    STCURRENT = 0;             // Clear current
    STCTRL = 0x07;             // Enable SysTick with core clock and interrupt
}

// ------------------------------------------------------------
// SysTick Handler - Blink Green LED
// ------------------------------------------------------------
void SysTick_Handler(void)
{
    GPIO_PORTF_DATA_R ^= GREEN_LED;
}

// ------------------------------------------------------------
// GPIOF Interrupt Handler - Toggle Blue/Red LEDs
// ------------------------------------------------------------
void GPIOF_Handler(void)
{
    if (GPIO_PORTF_MIS_R & SW1)   // SW1 pressed
    {
        GPIO_PORTF_DATA_R ^= BLUE_LED;  // Toggle Blue LED
        GPIO_PORTF_ICR_R = SW1;         // Clear interrupt
    }
    if (GPIO_PORTF_MIS_R & SW2)   // SW2 pressed
    {
        GPIO_PORTF_DATA_R ^= RED_LED;   // Toggle Red LED
        GPIO_PORTF_ICR_R = SW2;         // Clear interrupt
    }
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main(void)
{
    PortF_Init();
    SysTick_Init(16000000/2);  // Blink green every ~0.5s (SysClk=16MHz)

    while (1)
    {
        // CPU idle, waiting for interrupts
    }
}
