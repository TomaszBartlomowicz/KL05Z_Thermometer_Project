#ifndef DS18B20_H
#define DS18B20_H
#include "MKL05Z4.h"

#define DS18B20_ROM_CODE_SIZE 8
#define DS18B20_SCRATCHPAD_SIZE 9
#define DS18B20_READ_ROM 0x33
#define DS18B20_MATCH_ROM 0x55
#define DS18B20_SKIP_ROM 0xCC
#define DS18B20_CONVERT_T 0x44
#define DS18B20_READ_SCRATCHPAD 0xBE
//#define PORT_PCR_ODE_MASK  (1U << 10)  // Open-Drain Enable

// Prototypy funkcji
void delay_us(uint32_t us);
void delay_us2(uint32_t us);
void ds18b20_init(void);
int ds18b20_read_address(uint8_t *rom_code);
int ds18b20_start_measure(void);
float ds18b20_get_temperature(void);
int onewire_reset(void);
uint8_t onewire_read(void);
void onewire_write(uint8_t byte);
uint8_t onewire_crc(const uint8_t *data, int len);
void write_bit(int value);
// Prototypy funkcji GPIO
void GPIO_Init(void);
void GPIO_SetHigh(void);
void GPIO_SetLow(void);
int GPIO_Read(void);


#endif // DS18B20_H
