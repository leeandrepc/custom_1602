#ifndef PIN_NUM_CONTROL
#define PIN_NUM_CONTROL 4
#endif

#define CGRAM_ADDR_1 0x00 // 00000000
#define CGRAM_ADDR_2 0x01 // 00000001
#define CGRAM_ADDR_3 0x02 // 00000010
#define CGRAM_ADDR_4 0x03 // 00000011
#define CGRAM_ADDR_5 0x04 // 00000100
#define CGRAM_ADDR_6 0x05 // 00000101
#define CGRAM_ADDR_7 0x06 // 00000110
#define CGRAM_ADDR_8 0x07 // 00000111

// header needs to define a value for PIN_NUM_CONTROL
typedef enum {
    pin_RS,
    pin_RW,
    pin_E,
    pin_DB7,
    pin_DB6,
    pin_DB5,
    pin_DB4,
    pin_DB3,
    pin_DB2,
    pin_DB1,
    pin_DB0,
}lcd_pin_enum;

typedef uint8_t lcd_pin_t[];

esp_err_t lcd_pin_init_config(lcd_pin_t lcd_handle);
esp_err_t lcd_enable_pin(lcd_pin_t lcd_handle); // Should not need to explicitly call this
esp_err_t lcd_reset(lcd_pin_t lcd_handle);
esp_err_t ddram_write(lcd_pin_t lcd_handle, char* word, uint8_t row, uint8_t col);
esp_err_t num_writer(lcd_pin_t lcd_handle, int num, uint8_t row, uint8_t col, uint8_t max_digits);
esp_err_t cgram_write(lcd_pin_t lcd_handle, uint8_t* charmat, char ddram_address);
esp_err_t ddram_write_custom(lcd_pin_t lcd_handle, char DDRAM_ADDR, uint8_t row, uint8_t col);