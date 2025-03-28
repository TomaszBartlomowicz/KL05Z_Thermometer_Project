#ifndef BACKEND_H
#define BACKEND_H

#include "MKL05Z4.h"
#include <stdbool.h>
#define S1_MASK (1U << 10) // Maska dla przycisku S2
#define S2_MASK (1U << 12)
#define S3_MASK (1U << 11)
#define RED_LED_PIN 8 
#define GREEN_LED_PIN 9
#define BLUE_LED_PIN 10
#define BUZZER_PIN 13

//Zmienne
extern volatile bool isCelsius;
extern volatile bool not_setting_low_alarm;
extern volatile bool not_setting_high_alarm;
extern volatile int iteration_count;
extern volatile int high_temp_alarm;
extern volatile int low_temp_alarm;


// Funkjce
void LED_Init(void);
void Buttons_Init(void);
void RED_LED_On(void);
void RED_LED_Off(void);
void BLUE_LED_On(void);
void BLUE_LED_Off(void);
void GREEN_LED_Off(void);
void GREEN_LED_On(void);
void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void PORTA_IRQHandler(void);
void debug_message(const char *message);


#endif 
