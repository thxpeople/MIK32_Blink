#include <mik32_memory_map.h>
#include <pad_config.h>
#include <gpio.h>
#include <power_manager.h>
#include <wakeup.h>

/*
 * Данный пример демонстрирует работу с GPIO и PAD_CONFIG.
 * В примере настраивается вывод, который подключенный к светодиоду, в режим GPIO.
 */

#define PIN_LED1 9    // Светодиод управляется выводом PORT_0_9
#define PIN_LED2 10   // Светодиод управляется выводом PORT_0_10
#define PIN_BUTTON 15 // Кнопка на PORT_1_15


void InitClock()
{
    PM->CLK_APB_P_SET |= PM_CLOCK_APB_P_UART_0_M | PM_CLOCK_APB_P_GPIO_0_M | PM_CLOCK_APB_P_GPIO_1_M | PM_CLOCK_APB_P_GPIO_2_M; // включение тактирования GPIO
    PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;                               // включение тактирования блока для смены режима выводов
}

void ledBlink()
{
    GPIO_0->OUTPUT ^= 1 << PIN_LED2; // Установка сигнала вывода 3 порта 1 в противоположный уровень
    for (volatile int i = 0; i < 100000; i++) {
        (void)i;
    }
}




void ledButton()
{
    if (GPIO_1->STATE & (1 << PIN_BUTTON)) {
        GPIO_0->OUTPUT |= 1 << PIN_LED1; // Установка сигнала вывода 7 порта 2 в высокий уровень
    }
    else {
        GPIO_0->OUTPUT &= ~(1 << PIN_LED1); // Установка сигнала вывода 7 порта в низкий уровень
    }
}


int main()
{
    InitClock(); // Включение тактирования GPIO

    PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * PIN_LED1));   // Установка вывода 9 порта 0 в режим GPIO
    PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * PIN_LED2));   // Установка вывода 10 порта 0 в режим GPIO
    PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * PIN_BUTTON)); // Установка вывода 15 порта 1 в режим GPIO

    GPIO_0->DIRECTION_OUT = 1 << PIN_LED1;  // Установка направления вывода 9 порта 0 на выход
    GPIO_0->DIRECTION_OUT = 1 << PIN_LED2;  // Установка направления вывода 10 порта 0 на выход
    GPIO_1->DIRECTION_IN = 1 << PIN_BUTTON; // Установка направления вывода 15 порта 1 на вход

    while (1) {
        ledBlink(); /* Светодиод мигает */
        ledButton(); /* Светодиод зажигается при нажатой кнопке */
    }
}
