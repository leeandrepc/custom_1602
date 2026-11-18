#ifndef PIN_NUM_CONTROL
#define PIN_NUM_CONTROL 4
#endif

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
