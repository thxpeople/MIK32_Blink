/**
 * @file
 * Пример демонстрирует работу с прерываниями от подсистемы ввода/вывода общего назначения (GPIO).
 * В листинге приведен порядок настройки:
 * - подсистемы обработки прерываний (main) и объявления функции для обработки прерываний (trap_handler);
 * - предотвращения эффекта дребезга контактов при использовании прерываний;
 * - подсистемы тактирования и монитора частоты МК (systemClockConfig);
 * - выводов общего назначения (initGPIO);
 * - управление светодиодом на основе обработки прерывания при нажатии на кнопку (blinkLED)
 *
 * Ожидаемое поведение при выполнении приложения:
 * 1) светодиод выключен;
 * 2) быстрое нажатие на пользовательскую кнопку переключает режим работы светодиода (ВКЛ/ВЫКЛ);
 * 3) при удержании пользовательской кнопки светодиод светит.
 *
 * Примечания:
 * 1) обработчик прерываний trap_handler - это функция статически линкуемая (на этапе сборки приложения) к вектору прерываний EPIC.
 * 2) переменная flag должная иметь квалификатор volatile, так как изменяется внутри обработчика прерываний.
 */
#include "mik32_hal.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_irq.h"
#include "mik32_hal_pcc.h"

#define BOARD_NUKE_MIK32

void systemClockConfig();
void initGPIO();
void blinkLED();

volatile uint32_t flag = 0;

int main()
{
    // 1. Инициализировать подсистему аппаратной абстракции
    HAL_Init();
    // 2. Инициализировать подсистему тактирования и монитор частоты МК
    systemClockConfig();
    // 3. Инициализировать подсистему ввода/вывода общего назначения, настроить выводы для кнопки и светодиода(ов)
    initGPIO();
    // 4. Включить прерывания по уровню для линии EPIC GPIO_IRQ
    HAL_EPIC_MaskLevelSet(HAL_EPIC_GPIO_IRQ_MASK);
    // 5. Включить глобальные прерывания
    HAL_IRQ_EnableInterrupts();
    // 6. Запустить "бесконечный" цикл выполнения программы
    while (1) {
        // 7. Управлять уровнем сигнала вывода, подключеного к светодиоду
        blinkLED();
    }
}
/**
 * Управляет светодиодом на основе результатов обработки прерывания,
 * возникающего при нажатии на пользовательскую кнопку
 */
void blinkLED()
{
    if (flag) {
        // Отключить обработку запросов прерываний от кнопки
        // HAL_GPIO_DeInitInterruptLine(GPIO_MUX_PORT1_15_LINE_3);

        // Переключить уровень сигнала на выходе, подключенного к светодиоду на противоположный
        HAL_GPIO_TogglePin(GPIO_0, GPIO_PIN_9);

        // сбросить флаг нажатия пользовательской кнопки
        flag = 0;

        // Включить обработку запросов прерываний от кнопки
        // HAL_GPIO_InitInterruptLine(GPIO_MUX_PORT1_15_LINE_3, GPIO_INT_MODE_FALLING);
        HAL_GPIO_TogglePin(GPIO_0, GPIO_PIN_9);
        HAL_GPIO_TogglePin(GPIO_0, GPIO_PIN_9);
    }

    HAL_GPIO_TogglePin(GPIO_0, GPIO_PIN_10);

    // Запустить цикл ожидания, для устранения эффекта "дребезга" контактов кнопки
    for (volatile int i = 0; i < 500000; i++) {
    }
}
/**
 * Настраивает подсистему тактирования и монитор частоты МК
 */
void systemClockConfig(void)
{
    PCC_InitTypeDef PCC_OscInit          = {0};
    PCC_OscInit.OscillatorEnable         = PCC_OSCILLATORTYPE_ALL;
    PCC_OscInit.FreqMon.OscillatorSystem = PCC_OSCILLATORTYPE_OSC32M;
    PCC_OscInit.FreqMon.ForceOscSys      = PCC_FORCE_OSC_SYS_UNFIXED;
    PCC_OscInit.FreqMon.Force32KClk      = PCC_FREQ_MONITOR_SOURCE_OSC32K;
    PCC_OscInit.AHBDivider               = 0;
    PCC_OscInit.APBMDivider              = 0;
    PCC_OscInit.APBPDivider              = 0;
    PCC_OscInit.HSI32MCalibrationValue   = 128;
    PCC_OscInit.LSI32KCalibrationValue   = 8;
    PCC_OscInit.RTCClockSelection        = PCC_RTC_CLOCK_SOURCE_AUTO;
    PCC_OscInit.RTCClockCPUSelection     = PCC_CPU_RTC_CLOCK_SOURCE_OSC32K;
    HAL_PCC_Config(&PCC_OscInit);
}
/**
 * Инициализирует подсистему ввода/вывода общего назначения. Настраивает выводы.
 */
void initGPIO()
{
    __HAL_PCC_GPIO_IRQ_CLK_ENABLE();

    // Включить тактирование GPIO_0 и GPIO_1
    __HAL_PCC_GPIO_0_CLK_ENABLE();
    __HAL_PCC_GPIO_1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Настроить режим работы выхода, подключенного к светодиоду
    GPIO_InitStruct.Pin  = GPIO_PIN_9;
    GPIO_InitStruct.Mode = HAL_GPIO_MODE_GPIO_OUTPUT;
    GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;
    HAL_GPIO_Init(GPIO_0, &GPIO_InitStruct);

    // Установить высокий уровень на выходе, подлюченном к светодиоду (ВЫКЛ)
    HAL_GPIO_WritePin(GPIO_0, GPIO_PIN_9, GPIO_PIN_HIGH);

    // Настроить режим работы выхода, подключенного к светодиоду
    GPIO_InitStruct.Pin  = GPIO_PIN_10;
    GPIO_InitStruct.Mode = HAL_GPIO_MODE_GPIO_OUTPUT;
    GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;
    HAL_GPIO_Init(GPIO_0, &GPIO_InitStruct);

    // Настроить режим работы входа, подключенного к пользовательской кнопке
    GPIO_InitStruct.Pin  = GPIO_PIN_15;
    GPIO_InitStruct.Mode = HAL_GPIO_MODE_GPIO_INPUT;
    GPIO_InitStruct.Pull = HAL_GPIO_PULL_UP;
    HAL_GPIO_Init(GPIO_1, &GPIO_InitStruct);

    // Настроить обработку прерываний по спаду для входа, подключенного к пользовательской кнопке
    HAL_GPIO_InitInterruptLine(GPIO_MUX_PORT1_15_LINE_3, GPIO_INT_MODE_FALLING);
}

/**
 * Обрабатывает прерывания от подсистемы ввода/вывода общего назначения (GPIO).
 */
void trap_handler()
{
    // Если источник прерывания подсистема ввода/вывода общего назначения, то ..
    if (EPIC_CHECK_GPIO_IRQ()) {
        // Если источник прерывания линия 3, то ...
        if (HAL_GPIO_LineInterruptState(GPIO_LINE_3)) {
            // Установить флаг нажатия пользовательской кнопки
            flag = 1;
        }
        // Сбросить прерывания от подсистемы ввода/вывода общего назначения
        HAL_GPIO_ClearInterrupts();
    }
    // Сбросить все прерывания EPIC
    HAL_EPIC_Clear(0xFFFFFFFF);
}
