#include "backend.h"
#include "lcd1602.h"
#include "ds18b20.h"


volatile int low_temp_alarm = 0;
volatile int high_temp_alarm = 40;


void debug_message(const char *message) {
    LCD1602_ClearAll();
    LCD1602_Print(message);
    delay_us2(1000000);
}

void Buzzer_Init(void) {
		SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;  // W³¹czenie zegara dla PORTB
		PORTB->PCR[BUZZER_PIN] = PORT_PCR_MUX(1);
		PTB->PDDR |= (1 << BUZZER_PIN);  // Ustawienie pinu jako wyjœcie
		PTB->PCOR = (1 << BUZZER_PIN);
}	

void Buzzer_On(void){
		PTB->PCOR = (1 << BUZZER_PIN);

}

void Buzzer_Off(void) {
    PTB->PSOR = (1 << BUZZER_PIN);
}

void LED_Init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;  // W³¹czenie zegara dla PORTB
    PORTB->PCR[RED_LED_PIN] = PORT_PCR_MUX(1);  // GPIO
    PORTB->PCR[BLUE_LED_PIN] = PORT_PCR_MUX(1);
		PORTB->PCR[GREEN_LED_PIN] = PORT_PCR_MUX(1);
		PTB->PDDR |= (1 << RED_LED_PIN);  // Ustawienie pinu jako wyjœcie
    PTB->PSOR = (1 << RED_LED_PIN);  // WY£¥CZENIE diody na start (aktywny stan niski)
		PTB->PDDR |= (1 << BLUE_LED_PIN);  // Ustawienie pinu jako wyjœcie
    PTB->PSOR = (1 << BLUE_LED_PIN);  // WY£¥CZENIE diody na start (aktywny stan niski)
		PTB->PDDR |= (1 << GREEN_LED_PIN);  // Ustawienie pinu jako wyjœcie
    PTB->PSOR = (1 << GREEN_LED_PIN);  // WY£¥CZENIE diody na start (aktywny stan niski)

}



void GREEN_LED_On(void) {
    PTB->PSOR = (1 << GREEN_LED_PIN);
}

void GREEN_LED_Off(void){
		PTB->PCOR = (1 << GREEN_LED_PIN);
}
void RED_LED_On(void) {
    PTB->PCOR = (1 << RED_LED_PIN);
}

void RED_LED_Off(void) {
    PTB->PSOR = (1 << RED_LED_PIN);
}

void BLUE_LED_On(void) {
    PTB->PCOR = (1 << BLUE_LED_PIN);
}

void BLUE_LED_Off(void) {
    PTB->PSOR = (1 << BLUE_LED_PIN);
}

void Buttons_Init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;  // W³¹czenie zegara dla PORTA

    // Konfiguracja pinu S1 jako wejœcia z Pull-Up i przerwaniem na zbocze opadaj¹ce
    PORTA->PCR[10] = PORT_PCR_MUX(1UL) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(0xA);
    PORTA->PCR[12] = PORT_PCR_MUX(1UL) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(0xA);
    PORTA->PCR[11] = PORT_PCR_MUX(1UL) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(0xA);
		
	// Ustawienie jako wejœcia
    PTA->PDDR &= ~S1_MASK;  
		PTA->PDDR &= ~S2_MASK;
		PTA->PDDR &= ~S2_MASK;
    // Zerowanie ewentualnych starych przerwañ
    PORTA->ISFR |= S1_MASK;
		PORTA->ISFR |= S2_MASK;
    // W³¹czenie przerwañ w NVIC
    NVIC_SetPriority(PORTA_IRQn, 3);  
    NVIC_ClearPendingIRQ(PORTA_IRQn);
    NVIC_EnableIRQ(PORTA_IRQn);

	
}




// Obs³uga przerwañ dla PORTA
void PORTA_IRQHandler(void) {
    delay_us2(40000);  // Debounce (20ms)

    // Sprawdzanie, czy naciœniêto przycisk S1 (zmiana jednostek lub zwiêkszanie wartoœci w menu)
    if (PORTA->ISFR & S1_MASK) {  
        if (!not_setting_high_alarm) {  
            high_temp_alarm++;
        } 
        else if (!not_setting_low_alarm) {  
            low_temp_alarm++;
        } 
        else {  
            isCelsius = !isCelsius;  
            LCD1602_ClearAll();      
            LCD1602_Print(isCelsius ? "Switching to C." : "Switching to K.");
            delay_us2(1000000);
        }
        PORTA->ISFR = S1_MASK;  
    }

    // Sprawdzanie, czy naciœniêto przycisk S2 (wejœcie w ustawienia high alarm lub zmniejszanie wartoœci)
    if (PORTA->ISFR & S2_MASK) {  
        if (!not_setting_low_alarm) {  
            low_temp_alarm--;  // Zmniejszanie progu dolnego alarmu
        } 
        else if (!not_setting_high_alarm) {
							high_temp_alarm--; 
        } 
        else {  
            not_setting_high_alarm = false;
            LCD1602_ClearAll();
            LCD1602_Print("Set High Alarm");
            delay_us2(1000000);
        }
        PORTA->ISFR = S2_MASK;  
    }

    // Sprawdzanie, czy naciœniêto przycisk S3 (wejœcie w ustawienia low alarm lub zapisanie alarmu)
    if (PORTA->ISFR & S3_MASK) {  
        if (!not_setting_high_alarm) {  
            not_setting_high_alarm = true;
            LCD1602_ClearAll();
            LCD1602_Print("High Alarm saved");
            delay_us2(1000000);
        }
        else if (!not_setting_low_alarm) {  
            not_setting_low_alarm = true;
            LCD1602_ClearAll();
            LCD1602_Print("Low Alarm saved");
            delay_us2(1000000);
        }
        else {  
            not_setting_low_alarm = false;
            LCD1602_ClearAll();
            LCD1602_Print("Set Low Alarm");
            delay_us2(1000000);
        }
        PORTA->ISFR = S3_MASK;  
    }
}
