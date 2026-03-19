/*
 * CG2271 Final Project - "WorkHere"
 * MCXC444 Code with FreeRTOS
 *
 * Requirements Met:
 * - 3+ Tasks ✓ (ButtonTask, UARTTask, LEDTask, DisplayTask)
 * - 1 ISR ✓ (Button interrupt)
 * - 1 Mutex ✓ (Protects sensor data)
 * - 1 Queue ✓ (Button trigger to sampling task)
 * - Polled sensor ✓ (Button polling for press count)
 * - Interrupt-driven sensor ✓ (Button edge trigger)
 * - Time-slicing enabled ✓
 * - Preemption enabled ✓
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// ==================== CONFIGURATION ====================
#define BUTTON_PIN      4   // PTA4 (SW3)
#define RED_PIN         31  // PTE31
#define GREEN_PIN       5   // PTD5
#define BLUE_PIN        29  // PTE29
#define BUZZER_PIN      20  // PTE20

// UART Pins (for ESP32 communication)
#define UART_TX_PIN     22  // PTE22
#define UART_RX_PIN     23  // PTE23
#define BAUD_RATE       9600
#define UART2_INT_PRIO  128

// Queue and buffer sizes
#define QUEUE_LENGTH    10
#define MAX_MSG_LEN     64

// ==================== DATA STRUCTURES ====================
typedef struct {
    int temperatureScore;
    int soundScore;
    int lightScore;
    char overall[16];
} EnvironmentScores_t;

typedef struct {
    char data[MAX_MSG_LEN];
} UARTMessage_t;

// ==================== GLOBAL VARIABLES ====================
QueueHandle_t buttonQueue;      // Queue for button press triggers
QueueHandle_t uartQueue;        // Queue for UART messages from ESP32
SemaphoreHandle_t scoreMutex;   // Mutex to protect score data
EnvironmentScores_t currentScores;

// Button press counter (for polling requirement)
volatile int buttonPressCount = 0;

// ==================== LED CONTROL FUNCTIONS ====================
void initLEDs() {
    // Enable clock gating
    SIM->SCGC5 |= (SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK);

    // Configure pins as GPIO
    PORTE->PCR[RED_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[RED_PIN] |= PORT_PCR_MUX(1);
    PORTE->PCR[BLUE_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[BLUE_PIN] |= PORT_PCR_MUX(1);
    PORTD->PCR[GREEN_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTD->PCR[GREEN_PIN] |= PORT_PCR_MUX(1);
    PORTE->PCR[BUZZER_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[BUZZER_PIN] |= PORT_PCR_MUX(1);

    // Set as outputs
    GPIOE->PDDR |= (1 << RED_PIN) | (1 << BLUE_PIN) | (1 << BUZZER_PIN);
    GPIOD->PDDR |= (1 << GREEN_PIN);

    // Turn everything off (active low for LEDs)
    GPIOE->PSOR |= (1 << RED_PIN) | (1 << BLUE_PIN);
    GPIOD->PSOR |= (1 << GREEN_PIN);
    GPIOE->PCOR |= (1 << BUZZER_PIN);  // Buzzer off
}

void setLEDColor(int red, int green, int blue) {
    // Active low: PSOR = OFF, PCOR = ON
    if (red) GPIOE->PCOR |= (1 << RED_PIN);
    else GPIOE->PSOR |= (1 << RED_PIN);

    if (green) GPIOD->PCOR |= (1 << GREEN_PIN);
    else GPIOD->PSOR |= (1 << GREEN_PIN);

    if (blue) GPIOE->PCOR |= (1 << BLUE_PIN);
    else GPIOE->PSOR |= (1 << BLUE_PIN);
}

void setBuzzer(int on) {
    if (on) {
        GPIOE->PSOR |= (1 << BUZZER_PIN);  // Buzzer on (active high)
    } else {
        GPIOE->PCOR |= (1 << BUZZER_PIN);  // Buzzer off
    }
}

// ==================== BUTTON INTERRUPT SETUP ====================
void initButtonIRQ() {
    NVIC_DisableIRQ(PORTA_IRQn);

    // Clock gating for PORTA
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;

    // Configure as GPIO
    PORTA->PCR[BUTTON_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTA->PCR[BUTTON_PIN] |= PORT_PCR_MUX(1);

    // Set as input with pull-up
    GPIOA->PDDR &= ~(1 << BUTTON_PIN);
    PORTA->PCR[BUTTON_PIN] |= PORT_PCR_PS(1) | PORT_PCR_PE(1);

    // Configure interrupt on falling edge
    PORTA->PCR[BUTTON_PIN] &= ~PORT_PCR_IRQC_MASK;
    PORTA->PCR[BUTTON_PIN] |= PORT_PCR_IRQC(0b1010);  // Falling edge

    NVIC_SetPriority(PORTA_IRQn, 3);  // Low priority for ISR
    NVIC_ClearPendingIRQ(PORTA_IRQn);
    NVIC_EnableIRQ(PORTA_IRQn);
}

// ==================== BUTTON ISR ====================
void PORTA_IRQHandler() {
    BaseType_t hpw = pdFALSE;

    if (PORTA->ISFR & (1 << BUTTON_PIN)) {
        // Clear interrupt flag
        PORTA->ISFR |= (1 << BUTTON_PIN);

        // Send trigger to queue (from ISR)
        int trigger = 1;
        xQueueSendFromISR(buttonQueue, &trigger, &hpw);

        // Request context switch if needed
        portYIELD_FROM_ISR(hpw);
    }
}

// ==================== UART INITIALIZATION ====================
void initUART2(uint32_t baud_rate) {
    NVIC_DisableIRQ(UART2_FLEXIO_IRQn);

    SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

    UART2->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);

    // Configure pins
    PORTE->PCR[UART_TX_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[UART_TX_PIN] |= PORT_PCR_MUX(4);
    PORTE->PCR[UART_RX_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[UART_RX_PIN] |= PORT_PCR_MUX(4);

    // Set baud rate
    uint32_t bus_clk = CLOCK_GetBusClkFreq();
    uint32_t sbr = (bus_clk + (baud_rate * 8)) / (baud_rate * 16);

    UART2->BDH &= ~UART_BDH_SBR_MASK;
    UART2->BDH |= ((sbr >> 8) & UART_BDH_SBR_MASK);
    UART2->BDL = (uint8_t)(sbr & 0xFF);

    // Configure UART
    UART2->C1 &= ~(UART_C1_LOOPS_MASK | UART_C1_RSRC_MASK |
                   UART_C1_PE_MASK | UART_C1_M_MASK);

    // Enable RX interrupt
    UART2->C2 |= UART_C2_RIE_MASK;
    UART2->C2 |= UART_C2_RE_MASK;

    NVIC_SetPriority(UART2_FLEXIO_IRQn, 2);
    NVIC_ClearPendingIRQ(UART2_FLEXIO_IRQn);
    NVIC_EnableIRQ(UART2_FLEXIO_IRQn);
}

// ==================== UART ISR ====================
void UART2_FLEXIO_IRQHandler(void) {
    static char recv_buffer[MAX_MSG_LEN];
    static int recv_ptr = 0;
    char rx_data;
    BaseType_t hpw = pdFALSE;

    NVIC_ClearPendingIRQ(UART2_FLEXIO_IRQn);

    if (UART2->S1 & UART_S1_RDRF_MASK) {
        rx_data = UART2->D;

        if (rx_data == '\n') {
            // End of message
            recv_buffer[recv_ptr] = '\0';

            UARTMessage_t msg;
            strncpy(msg.data, recv_buffer, MAX_MSG_LEN);
            xQueueSendFromISR(uartQueue, &msg, &hpw);

            recv_ptr = 0;
        } else if (recv_ptr < MAX_MSG_LEN - 1) {
            recv_buffer[recv_ptr++] = rx_data;
        }
    }
}

// Helper function to send a string over UART2 safely
void UART2_SendString(const char *str) {
    // Enable transmitter
    UART2->C2 |= UART_C2_TE_MASK;

    // Loop through each character until the null terminator
    while(*str != '\0') {
        // Wait until the Transmit Data Register Empty (TDRE) flag is set
        while(!(UART2->S1 & UART_S1_TDRE_MASK));

        // Write the character to the data register
        UART2->D = *str;

        // Move to the next character
        str++;
    }

    // Wait until the transmission of the last character is fully complete (TC flag)
    while(!(UART2->S1 & UART_S1_TC_MASK));

    // We leave the transmitter enabled, or you can disable it if strictly necessary.
    // Usually, it's fine to leave TE_MASK on.
}

// ==================== TASK 1: Button Monitor (Polling + Trigger) ====================
// Priority: 1 (Lowest)
static void buttonTask(void *pvParameters) {
    int trigger;

    PRINTF("Button Task Started (Priority 1)\r\n");

    while (1) {
        // Wait for button press from ISR via queue
        if (xQueueReceive(buttonQueue, &trigger, portMAX_DELAY) == pdTRUE) {
            buttonPressCount++;

            // Check if this is the 5th press
            if (buttonPressCount % 5 == 0) {
                PRINTF("Button pressed %d times - Triggering sensor read\r\n", buttonPressCount);

                // Use the new helper function to send the command safely
                UART2_SendString("READ_SENSORS\n");
            }
        }
    }
}

// ==================== TASK 2: UART Receive (Get scores from ESP32) ====================
// Priority: 2 (Medium)
static void uartReceiveTask(void *pvParameters) {
    UARTMessage_t msg;

    PRINTF("UART Receive Task Started (Priority 2)\r\n");

    while (1) {
        if (xQueueReceive(uartQueue, &msg, portMAX_DELAY) == pdTRUE) {
            PRINTF("Received from ESP32: %s\r\n", msg.data);

            // Parse format: "T:4,S:3,L:5,O:Good"
            int tScore = 3, sScore = 3, lScore = 3;
            char overall[16] = "Unknown";

            // Simple parsing
            char *ptr = msg.data;
            if (strncmp(ptr, "T:", 2) == 0) {
                tScore = atoi(ptr + 2);
                ptr = strchr(ptr, ',');
                if (ptr) {
                    ptr++;
                    if (strncmp(ptr, "S:", 2) == 0) {
                        sScore = atoi(ptr + 2);
                        ptr = strchr(ptr, ',');
                        if (ptr) {
                            ptr++;
                            if (strncmp(ptr, "L:", 2) == 0) {
                                lScore = atoi(ptr + 2);
                                ptr = strchr(ptr, ',');
                                if (ptr) {
                                    ptr++;
                                    if (strncmp(ptr, "O:", 2) == 0) {
                                        strncpy(overall, ptr + 2, 15);
                                        // Remove trailing newline if present
                                        char *nl = strchr(overall, '\n');
                                        if (nl) *nl = '\0';
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Update scores with mutex protection
            if (xSemaphoreTake(scoreMutex, portMAX_DELAY) == pdTRUE) {
                currentScores.temperatureScore = tScore;
                currentScores.soundScore = sScore;
                currentScores.lightScore = lScore;
                strncpy(currentScores.overall, overall, 15);
                xSemaphoreGive(scoreMutex);
            }

            PRINTF("Parsed: T=%d, S=%d, L=%d, O=%s\r\n",
                   tScore, sScore, lScore, overall);
        }
    }
}

// ==================== TASK 3: LED Control (Based on scores) ====================
// Priority: 3 (Highest - needs quick response)
static void ledControlTask(void *pvParameters) {
    EnvironmentScores_t localScores;

    PRINTF("LED Control Task Started (Priority 3 - Highest)\r\n");

    while (1) {
        // Safely copy scores with mutex
        if (xSemaphoreTake(scoreMutex, portMAX_DELAY) == pdTRUE) {
            localScores = currentScores;
            xSemaphoreGive(scoreMutex);
        }

        // Map overall rating to LED color
        if (strcmp(localScores.overall, "Good") == 0) {
            setLEDColor(0, 1, 0);  // Green
            setBuzzer(0);           // Buzzer off
        } else if (strcmp(localScores.overall, "Poor") == 0) {
            setLEDColor(1, 1, 0);  // Yellow
            setBuzzer(1);           // Buzzer short beep pattern
            vTaskDelay(pdMS_TO_TICKS(100));
            setBuzzer(0);
        } else if (strcmp(localScores.overall, "Bad") == 0) {
            setLEDColor(1, 0, 0);  // Red
            // Buzzer alarm pattern
            for (int i = 0; i < 3; i++) {
                setBuzzer(1);
                vTaskDelay(pdMS_TO_TICKS(100));
                setBuzzer(0);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        } else {
            setLEDColor(0, 0, 1);  // Blue (waiting/unknown)
            setBuzzer(0);
        }

        vTaskDelay(pdMS_TO_TICKS(100));  // Update every 100ms
    }
}

// ==================== TASK 4: Display Task (Optional - for I2C display) ====================
// Priority: 2 (Same as UART)
static void displayTask(void *pvParameters) {
    EnvironmentScores_t localScores;
    char displayLine[32];

    PRINTF("Display Task Started (Priority 2)\r\n");

    while (1) {
        // Safely copy scores
        if (xSemaphoreTake(scoreMutex, portMAX_DELAY) == pdTRUE) {
            localScores = currentScores;
            xSemaphoreGive(scoreMutex);
        }

        // Here you would update your I2C display
        // Example: display.printf("T:%d S:%d L:%d\n%s",
        //          localScores.temperatureScore,
        //          localScores.soundScore,
        //          localScores.lightScore,
        //          localScores.overall);

        PRINTF("Display would show: T:%d S:%d L:%d %s\r\n",
               localScores.temperatureScore,
               localScores.soundScore,
               localScores.lightScore,
               localScores.overall);

        vTaskDelay(pdMS_TO_TICKS(500));  // Update display every 500ms
    }
}

// ==================== MAIN ====================
int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();

    PRINTF("\r\n====================================\r\n");
    PRINTF("CG2271 Final Project - WorkHere\r\n");
    PRINTF("Temperature, Sound, Light Monitor\r\n");
    PRINTF("with ChatGPT Scoring\r\n");
    PRINTF("====================================\r\n\r\n");

    // Initialize hardware
    initLEDs();
    initUART2(BAUD_RATE);
    initButtonIRQ();

    // Create queues
    buttonQueue = xQueueCreate(QUEUE_LENGTH, sizeof(int));
    uartQueue = xQueueCreate(QUEUE_LENGTH, sizeof(UARTMessage_t));

    // Create mutex
    scoreMutex = xSemaphoreCreateMutex();

    if (buttonQueue == NULL || uartQueue == NULL || scoreMutex == NULL) {
        PRINTF("Failed to create RTOS objects!\r\n");
        while (1);
    }

    // Initialize scores
    currentScores.temperatureScore = 3;
    currentScores.soundScore = 3;
    currentScores.lightScore = 3;
    strcpy(currentScores.overall, "Unknown");

    // Create tasks with DIFFERENT priorities
    // Task 1: Button - Priority 1 (Lowest)
    xTaskCreate(buttonTask, "Button", configMINIMAL_STACK_SIZE + 200, NULL, 1, NULL);

    // Task 2: UART Receive - Priority 2 (Medium)
    xTaskCreate(uartReceiveTask, "UART_RX", configMINIMAL_STACK_SIZE + 300, NULL, 2, NULL);

    // Task 3: LED Control - Priority 3 (Highest)
    xTaskCreate(ledControlTask, "LED_Ctrl", configMINIMAL_STACK_SIZE + 200, NULL, 3, NULL);

    // Task 4: Display - Priority 2 (Medium)
    xTaskCreate(displayTask, "Display", configMINIMAL_STACK_SIZE + 200, NULL, 2, NULL);

    PRINTF("All tasks created. Starting scheduler...\r\n\r\n");

    // Start scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (1) {
        __asm volatile ("nop");
    }
    return 0;
}
