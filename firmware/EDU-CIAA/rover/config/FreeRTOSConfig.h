#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "chip.h"

/* boardConfig() actualiza SystemCoreClock antes de llamar a app_init(). */
#define configCPU_CLOCK_HZ                       (SystemCoreClock)
#define configTICK_RATE_HZ                       1000U /* 1 tick = 1 ms */
#define configUSE_PREEMPTION                     1
#define configMAX_PRIORITIES                     4
#define configMINIMAL_STACK_SIZE                 128U /* Palabras de 32 bits. */
#define configMAX_TASK_NAME_LEN                  16
#define configUSE_16_BIT_TICKS                   0

#define configSUPPORT_STATIC_ALLOCATION          1
#define configSUPPORT_DYNAMIC_ALLOCATION         0
#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0
#define configUSE_TIMERS                         0
#define configUSE_MUTEXES                        1
#define configCHECK_FOR_STACK_OVERFLOW           2

#define INCLUDE_vTaskDelayUntil                  1

/* LPC4337 M4: 3 bits, prioridades CMSIS de 0 (mayor) a 7 (menor).
 * Una ISR que use la API FromISR debe tener prioridad numerica >= 5. */
#define configPRIO_BITS                          __NVIC_PRIO_BITS
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY  7
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Nombres presentes en los vectores de armv7m_startup.c. */
#define vPortSVCHandler                          SVC_Handler
#define xPortPendSVHandler                       PendSV_Handler
#define xPortSysTickHandler                      SysTick_Handler

#define configASSERT(condition) do { \
    if (!(condition)) { \
        taskDISABLE_INTERRUPTS(); \
        for (;;) {} \
    } \
} while (0)

#endif /* FREERTOS_CONFIG_H */
