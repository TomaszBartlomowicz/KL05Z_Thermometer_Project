#include "ds18b20.h"
#include "lcd1602.h"


// Funkcja do opóŸnieñ w mikrosekundach

void delay_us(uint32_t us) {
    uint32_t ticks_per_us = SystemCoreClock / 1000000; // Iloœæ taktów zegara na µs
    uint32_t ticks = ticks_per_us * us;

    if (ticks > 0xFFFFFF) return; // Jeœli za du¿e opóŸnienie, ignoruj

    SysTick->LOAD = ticks - 1; // Ustaw wartoœæ pocz¹tkow¹
    SysTick->VAL = 0;         // Wyzeruj licznik
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)); // Czekaj na zakoñczenie odliczania

    SysTick->CTRL = 0; // Wy³¹cz licznik SysTick
}




void delay_us2(uint32_t us) {
    for (volatile uint32_t i = 0; i < us * 10; i++) { // Dla wiêkszych opóŸnien np. do debug message
        __asm("NOP"); // No Operation
    }
}


// Konfiguracja GPIO PTB6 jako linia OneWire

void GPIO_Init(void) {
    // W³¹cz portu B
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;

    // Skonfiguruj pin PTB6 jako GPIO z rezystorem podci¹gaj¹cym
    PORTB->PCR[6] = PORT_PCR_MUX(1)               // GPIO mode
                  //| PORT_PCR_ODE_MASK            // Open-drain enable
                  //| PORT_PCR_PE_MASK             // Pull resistor enable
                  | PORT_PCR_PS_MASK;            // Pull-up selected

    
    PTB->PDDR &= ~(1U << 6); // Domyœlnie wejœcie (wysoka impedancja)
}

void GPIO_SetLow(void) {
    PTB->PDDR |= (1U << 6);  // Ustaw pin jako wyjœcie
    PTB->PCOR = (1U << 6);   // Ustaw stan niski
}


void GPIO_SetHigh(void) {
    PTB->PDDR &= ~(1U << 6); // Ustaw pin jako wejœcie
}


// Odczytaj stan linii PTB6
int GPIO_Read(void) {
    return (PTB->PDIR & (1 << 6)) ? 1 : 0; // Odczytaj stan
}

// Funkcja inicjalizuj¹ca DS18B20
void ds18b20_init(void) {
    GPIO_Init();
}

// Reset magistrali 1-Wire
int onewire_reset(void) {
    GPIO_SetLow();
    delay_us(480); // Trzymanie linii nisko przez 480 µs
    GPIO_SetHigh();
    delay_us(70); // Oczekiwanie na odpowiedŸ

    int presence = GPIO_Read() == 0; // Sprawdzenie obecnoœci czujnika

    delay_us(410); // Czekaj na zakoñczenie resetu
    return presence ? 0 : -1;
}

// Odczyt bitu z magistrali 1-Wire
static int read_bit(void) {
    int bit;

    GPIO_SetLow();
    delay_us(6); 

    GPIO_SetHigh();
    delay_us(9);  

    bit = GPIO_Read();

    delay_us(55);

    return bit;
}

// Odczyt bajtu z magistrali 1-Wire
uint8_t onewire_read(void)
{
  uint8_t value = 0;
  int i;
  for (i = 0; i < 8; i++) {
    value >>= 1;
    if (read_bit())
      value |= 0x80;
  }
  return value;
}

// Zapis bitu do magistrali 1-Wire
void write_bit(int value) {
    if (value) {  // Wysy³anie logicznej jedynki
        GPIO_SetLow();
        delay_us2(6);
        GPIO_SetHigh();
        delay_us2(64);
    } else {  // Wysy³anie logicznego zera
        GPIO_SetLow();
        delay_us2(60);
        GPIO_SetHigh();
        delay_us2(10);
    }
    
}


// Zapis bajtu do magistrali 1-Wire
void onewire_write(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        write_bit(byte & 0x01); // Wyœlij najmniej znacz¹cy bit
        byte >>= 1;            // Przesuñ bajt o 1 bit w prawo
    }
}


// Uruchamianie pomiaru temperatury
int ds18b20_start_measure(void) {
    if (onewire_reset() != 0) return -1;

    onewire_write(DS18B20_SKIP_ROM);  // Pomijanie wyboru czujnika
    onewire_write(DS18B20_CONVERT_T); // Komenda rozpoczêcia pomiaru
    return 0;
}


uint8_t onewire_crc(const uint8_t *data, int len) {
    uint8_t crc = 0; // Inicjalizacja CRC na 0
    for (int i = 0; i < len; i++) {
        uint8_t byte = data[i];
        for (int j = 0; j < 8; j++) {
            uint8_t mix = (crc ^ byte) & 0x01;
            crc >>= 1;
            if (mix) {
                crc ^= 0x8C; // Wielomian: x^8 + x^5 + x^4 + 1
            }
            byte >>= 1;
        }
    }
    return crc;
}


// Odczyt temperatury z DS18B20
float ds18b20_get_temperature(void) {
    uint8_t temp_lsb, temp_msb;
    int16_t temp_raw;

    if (onewire_reset() != 0) return -100.0f; // B³¹d resetu

    onewire_write(DS18B20_SKIP_ROM);
    onewire_write(DS18B20_READ_SCRATCHPAD);

    temp_lsb = onewire_read();
    temp_msb = onewire_read();

		temp_raw = (int16_t)((uint16_t)temp_msb << 8 | temp_lsb);
    return temp_raw * 0.0625f; // Przeliczenie na stopnie Celsjusza
}
