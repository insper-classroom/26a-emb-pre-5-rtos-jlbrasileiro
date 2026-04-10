/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BTN_PIN_R 28
#define BTN_PIN_Y 21

#define LED_PIN_R 5
#define LED_PIN_Y 10

#define BLINK_DELAY_MS 100

SemaphoreHandle_t xSemaphoreBtnR;
SemaphoreHandle_t xSemaphoreBtnY;

void btn_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ((events & GPIO_IRQ_EDGE_FALL) == 0) {
        return;
    }

    if (gpio == BTN_PIN_R) {
        xSemaphoreGiveFromISR(xSemaphoreBtnR, &xHigherPriorityTaskWoken);
    }

    if (gpio == BTN_PIN_Y) {
        xSemaphoreGiveFromISR(xSemaphoreBtnY, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void led_r_task(void *p) {
    bool is_blinking = false;
    bool led_state = false;

    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    while (true) {
        if (xSemaphoreTake(xSemaphoreBtnR, 0) == pdTRUE) {
            is_blinking = !is_blinking;
            if (!is_blinking) {
                led_state = false;
                gpio_put(LED_PIN_R, 0);
            }
        }

        if (is_blinking) {
            led_state = !led_state;
            gpio_put(LED_PIN_R, led_state);
            vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void led_y_task(void *p) {
    bool is_blinking = false;
    bool led_state = false;

    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_put(LED_PIN_Y, 0);

    while (true) {
        if (xSemaphoreTake(xSemaphoreBtnY, 0) == pdTRUE) {
            is_blinking = !is_blinking;
            if (!is_blinking) {
                led_state = false;
                gpio_put(LED_PIN_Y, 0);
            }
        }

        if (is_blinking) {
            led_state = !led_state;
            gpio_put(LED_PIN_Y, led_state);
            vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

int main() {
    stdio_init_all();

    xSemaphoreBtnR = xSemaphoreCreateBinary();
    xSemaphoreBtnY = xSemaphoreCreateBinary();

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(led_r_task, "LED_R_Task", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED_Y_Task", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) {
    }

    return 0;
}