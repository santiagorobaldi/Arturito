#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

/* Memoria propia de cada tarea; el proveedor existente aporta la de Idle. */
#define TEST_STACK_WORDS 256U
static StaticTask_t task_100ms_tcb;
static StaticTask_t task_500ms_tcb;
static StackType_t task_100ms_stack[TEST_STACK_WORDS];
static StackType_t task_500ms_stack[TEST_STACK_WORDS];

static void task_100ms(void *parameter)
{
    TickType_t last_wake = xTaskGetTickCount();
    (void)parameter;

    for (;;) {
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(100));
        gpioToggle(LED1);
    }
}

static void task_500ms(void *parameter)
{
    TickType_t last_wake = xTaskGetTickCount();
    (void)parameter;

    for (;;) {
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(500));
        gpioToggle(LED2);
    }
}

void app_init(void)
{
    TaskHandle_t task_100ms_handle;
    TaskHandle_t task_500ms_handle;

    gpioWrite(LED1, OFF);
    gpioWrite(LED2, OFF);

    task_100ms_handle = xTaskCreateStatic(task_100ms, "test_100ms",
        TEST_STACK_WORDS, NULL, 1, task_100ms_stack, &task_100ms_tcb);
    configASSERT(task_100ms_handle != NULL);

    task_500ms_handle = xTaskCreateStatic(task_500ms, "test_500ms",
        TEST_STACK_WORDS, NULL, 1, task_500ms_stack, &task_500ms_tcb);
    configASSERT(task_500ms_handle != NULL);

    vTaskStartScheduler();

    /* Si retorna, no permitir que entry_educiaa.c llegue a app_update(). */
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

void app_update(void)
{
    /* Solo satisface la interfaz comun; app_init() nunca debe retornar. */
    configASSERT(0);
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;
    configASSERT(0);
}
