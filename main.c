#include "ds18b20.h"
#include "lcd1602.h"
#include "backend.h"
#include <stdbool.h>
#include <stdio.h>

volatile bool isCelsius = true;
volatile bool not_setting_high_alarm = true;
volatile bool not_setting_low_alarm = true;
volatile int iteration_count = 1;

int main(void) {
    uint8_t scratchpad[9];
    float temperature;
    char temp_msg[16];
	
	// Inicjalizacja podzespo³ów
    LCD1602_Init();
    LED_Init();
		Buzzer_Init();
    Buttons_Init();
    ds18b20_init();

    LCD1602_Print("Loading...");


    while (1) {

        if (onewire_reset() != 0) {
            debug_message("1-Wire Reset Fail");
            continue;
        }

        // Odczyt temperatury z DS18B20
        onewire_write(DS18B20_SKIP_ROM);
        onewire_write(DS18B20_CONVERT_T);
        delay_us(750000);
        
        if (onewire_reset() != 0) {
            debug_message("Reset failed");
            continue;
        }

        onewire_write(DS18B20_SKIP_ROM);
        onewire_write(DS18B20_READ_SCRATCHPAD);

        for (int i = 0; i < 9; i++) {
            scratchpad[i] = onewire_read();
        }

        if (onewire_crc(scratchpad, 8) != scratchpad[8]) {
            debug_message("CRC doesn't match");
            continue;
        }

        // Przetwarzanie temperatury
        int16_t temp_raw =((scratchpad[1] << 8) | scratchpad[0]);
        temperature = temp_raw * 0.0625f;
				
				// Celcjusz
        int temp_int = (int)temperature;
				int temp_frac = (int)(((temperature - (float)temp_int) * 100.0f) + 0.5f);

				
				// Kelwin
				float temp_kelvin = temperature + 273.15f;
        int temp_kelvin_int = (int)temp_kelvin;
				//int temp_kelvin_frac = (int)((temp_kelvin - (float)temp_kelvin_int) * 100.0f + 0.5f);
				
				if (!not_setting_high_alarm ){
						snprintf(temp_msg, sizeof(temp_msg), "Set High: %d\xDF""C", high_temp_alarm);
				}
				else if (!not_setting_low_alarm){
						snprintf(temp_msg, sizeof(temp_msg), "Set Low: %d\xDF""C", low_temp_alarm);
				}        
				
				// Wyœwietlanie temperatury w zale¿noœci od jednostki
        if (isCelsius && not_setting_high_alarm && not_setting_low_alarm) {
						snprintf(temp_msg, sizeof(temp_msg), "Temp:  %d.%02d\xDF""C", temp_int, temp_frac);
				}
				else if (!isCelsius && not_setting_high_alarm && not_setting_low_alarm) {
						snprintf(temp_msg, sizeof(temp_msg), "Temp:  %d.%02d\xDF""K", temp_kelvin_int, temp_frac);
				}


				
				//debug_message("debug here");
				// Poczekaj pierwsze dwie iteracje przed wyswietleniem temperatury (kalibracja czujnika) 
				if (iteration_count > 0) {  
					iteration_count--;  
				} 
				else {  
					LCD1602_ClearAll();  
					LCD1602_Print(temp_msg);  
				}
					
				if (temp_int >= high_temp_alarm) {
						RED_LED_On();
						BLUE_LED_Off();  // Gasi niebiesk¹ diodê, jeœli by³a w³¹czona
						Buzzer_On();
						delay_us2(50000);
				} 
				else if (temp_int <= low_temp_alarm) {
						BLUE_LED_On();
						RED_LED_Off();  // Gasi czerwon¹ diodê, jeœli by³a w³¹czona
						Buzzer_On();
						delay_us2(50000);
				} 
				else {
						// Normalna temperatura - wy³¹cz alarmy
						RED_LED_Off();
						BLUE_LED_Off();
						Buzzer_Off();
					}



						
				Buzzer_Off();


        // Pauza przed kolejnym pomiarem
        delay_us2(80000);
}
}
