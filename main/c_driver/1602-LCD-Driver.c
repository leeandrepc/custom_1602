#include <stdio.h>
#include <unistd.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>
#include <string.h>
#include "1602-LCD-Driver.h"
#include "esp_rom_sys.h"
/* 
    Initial Driver config
    - Choose 8 pin or 4 pin control
    - Choose GPIO pins to use: 8 + 3 or 4 + 3

*/

// 4 pin basic implementation tested working,
// 8 pin basic implementation is there but untested

#ifndef PIN_NUM_CONTROL
#define PIN_NUM_CONTROL 4 // 4 pin control for the 1602 LCD
#endif

#if !(PIN_NUM_CONTROL == 4 || PIN_NUM_CONTROL == 8)
#error "PIN_NUM_CONTROL must be 4 or 8"
#endif

// typedef enum {
//     pin_RS,
//     pin_RW,
//     pin_E,
//     pin_DB7,
//     pin_DB6,
//     pin_DB5,
//     pin_DB4,
//     pin_DB3,
//     pin_DB2,
//     pin_DB1,
//     pin_DB0,
// }lcd_pin_enum;

// typedef uint8_t lcd_pin_t[];

esp_err_t lcd_pin_init_config(lcd_pin_t lcd_handle){
    // ESP_LOGI("TESTING", "DRIVER CHECKPOINT 1");
    gpio_config_t pin_conf;
    esp_err_t ret;
    for (uint8_t i=0; i<PIN_NUM_CONTROL+3; i++){
        pin_conf.pin_bit_mask = (1 << lcd_handle[i]);
        pin_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        pin_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        pin_conf.intr_type = GPIO_INTR_DISABLE;
        pin_conf.mode = GPIO_MODE_INPUT_OUTPUT;
        ret = gpio_config(&pin_conf);
        if (ret!=ESP_OK){
            return ret;
        }
    }
    return ESP_OK;
}

esp_err_t lcd_enable_pin(lcd_pin_t lcd_handle){
    if (gpio_get_level(lcd_handle[pin_E]) != 0){
        ESP_ERROR_CHECK(gpio_set_level(lcd_handle[pin_E], 0));
    }
    ESP_ERROR_CHECK(gpio_set_level(lcd_handle[pin_E], 1));
    usleep(40); // min wait time is 230 ns, no max wait time required
    ESP_ERROR_CHECK(gpio_set_level(lcd_handle[pin_E], 0));
    usleep(40); // min complete cycle time is 500 ns, no max wait time required
    return ESP_OK;
}

#if (PIN_NUM_CONTROL==8)
// GPIO pin setup function
// rs = 0 for lcd instruction register commands, 1 for data register commands
// rw = 0 for writes to display, 1 for reads from display
esp_err_t lcd_pin_data_write(lcd_pin_t* lcd_handle, bool rs, bool rw, char conf_as_hex, bool reset_flag){
    ESP_ERROR_CHECK(gpio_set_level(lcd_handle[pin_RS], rs));
    ESP_ERROR_CHECK(gpio_set_level(lcd_handle[pin_RW], rw));
    for (uint8_t i=pin_DB7; i<PIN_NUM_CONTROL+3; i++){
            ESP_ERROR_CHECK(gpio_set_level(lcd_handle[i], (conf_as_hex>>(7-(i-3)))&0x01));
    }
    lcd_enable_pin(lcd_handle);
    return ESP_OK;
}


#elif (PIN_NUM_CONTROL==4)
// GPIO pin setup function
// rs = 0 for lcd instruction register commands, 1 for data register commands
// rw = 0 for writes to display, 1 for reads from display
esp_err_t lcd_pin_data_write(lcd_pin_t lcd_handle, bool rs, bool rw, char conf_as_hex, bool reset_flag){
    ESP_ERROR_CHECK(gpio_set_level(lcd_handle[pin_RS], rs));
    ESP_ERROR_CHECK(gpio_set_level(lcd_handle[pin_RW], rw));
    // First: send upper bits
    bool level;
    for (uint8_t i=pin_DB7; i<PIN_NUM_CONTROL+3; i++){
        level = (conf_as_hex>>(7-(i-3)))&0x01;
        ESP_ERROR_CHECK(gpio_set_level(lcd_handle[i], level));
    }
    lcd_enable_pin(lcd_handle);
    usleep(40); // Wait 40us just in case
    // Second: send lower bits
    if (reset_flag==0){
        for (uint8_t i=pin_DB7; i<PIN_NUM_CONTROL+3; i++){
            level = (conf_as_hex>>(3-(i-3)))&0x01;
            ESP_ERROR_CHECK(gpio_set_level(lcd_handle[i], level));
        }
        lcd_enable_pin(lcd_handle);
    }
    return ESP_OK;
}

#endif


// LCD Required inits - Seems to be this sequence, try and see
/*    
      Instruction: RS R/W DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
 - Reset LCD       0  0   0   0   1   1   -   -   -   -   - Send this 3 times (check documentation for wait time)
 - Function Set    0  0   0   0   1   DL  N   F   -   -   - Min wait time 39us 
 - Display off     0  0   0   0   0   0   1   0   0   0   
 - Display Clear   0  0   0   0   0   0   0   0   0   1   - Min wait time 1.52ms
 - Entry Mode      0  0   0   0   0   0   0   1   I/D S   - Min wait time 39us
    Settings  
*/

// Maybe build dictionary of hex functions?

esp_err_t lcd_reset(lcd_pin_t lcd_handle){
    vTaskDelay(pdMS_TO_TICKS(40)); // Wait 40ms for LCD hardware initialisation

    // Reset LCD 0x30 (00110000) - 3 signals 
    lcd_pin_data_write(lcd_handle, 0, 0, 0x30, 1);    
    vTaskDelay(pdMS_TO_TICKS(5)); // Wait for more than 4.1 ms
    lcd_pin_data_write(lcd_handle, 0, 0, 0x30, 1);    
    usleep(100); // Wait for more than 100 μs
    lcd_pin_data_write(lcd_handle, 0, 0, 0x30, 1);    

    // Function Set 0x28 (00101000)
    lcd_pin_data_write(lcd_handle, 0, 0, 0x28, 1);
    usleep(40);

    //  Function Set 0x28 (00101000)
    lcd_pin_data_write(lcd_handle, 0, 0, 0x28, 0);
    usleep(40);

    // Display Off 0x08 (00001000)
    lcd_pin_data_write(lcd_handle, 0, 0, 0x08, 0);
    usleep(40); 

    // Display Clear 0x01 (00000001)
    lcd_pin_data_write(lcd_handle, 0, 0, 0x01, 0);
    vTaskDelay(pdMS_TO_TICKS(10)); // This clear takes a long time ~10ms, shorter intervals
                                   // may not work
    // Entry Mode Settings 0x06 (00000110)
    lcd_pin_data_write(lcd_handle, 0, 0, 0x06, 0);
    usleep(40);

    // Display on, cursor off, no blinking 0x0c (00001100)
    lcd_pin_data_write(lcd_handle, 0, 0, 0x0c, 0);
    usleep(40);

    return ESP_OK;
}

// How to write to screen:
// 1602 has 80bytes of display data ram
// 40 for top row, 40 for bottom row
// Display only prints first 16 bytes of top row, and first 16 bytes of bottom row
// This means the display can scroll text left and right
// To write to display: choose address of DDRAM and write

// DDRAM ADDRESS LIST                               OUTSIDE OF DISPLAY
//  0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 ... 38 39
//  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ... 26 27
//  40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F ... 66 67
//  64 -                                                   103
esp_err_t ddram_write(lcd_pin_t lcd_handle, char* word, uint8_t row, uint8_t col){
    // Set DDRAM ADDRESS
    char addr = col;
    // ESP_LOGI("DEBUG", "Word Length %d", strlen(word));
    if (addr > 39 || strlen(word) > 40){
        return ESP_ERR_INVALID_ARG;
    }
    if (row==0){
        lcd_pin_data_write(lcd_handle, 0, 0, (addr+128), 0);
    } else if (row==1){
        addr = col + 64;
        lcd_pin_data_write(lcd_handle, 0, 0, (addr+128), 0);
    } else return ESP_ERR_INVALID_ARG;
    // printf("Address Set at: %d\n", addr);
    usleep(40);
    
    // Write to LCD starting at address
    for (uint8_t i=0; i<strlen(word); i++){
        // printf("Character: %c\n", word[i]);
        lcd_pin_data_write(lcd_handle, 1, 0, word[i], 0);
        usleep(45);
    }
    return ESP_OK;
}

// Turn int to string so it can be written by ddram_write onto lcd
esp_err_t num_writer(lcd_pin_t lcd_handle, int num, uint8_t row, uint8_t col, uint8_t max_digits){
    char buffer[max_digits+1];
    int length = sprintf(buffer, "%*d", max_digits, num);
    // manual error checking just in case
    if (length < 0){
        return ESP_ERR_NOT_FINISHED;
    }
    esp_err_t err = ddram_write(lcd_handle, buffer, row, col);
    return err;
}


// CGRAM can store:
//  - up to 8 8x5 character patterns from 0000*000 to 0000*111 where * is either 1 or 0
//  - up to 4 10x5 character patterns from 0000*00* to 0000*11* where * is either 1 or 0
// currently only 8x5 characters are implemented
/* 
    CGRAM write procedure:
        - Store desired custom character as a 8x5 matrix of 0 and 1s
        - Set CGRAM address
        - Write matrix into CGRAM address row by row (8 total)
        - CGRAM address: 01XXXYYY - X is 3 bit address that corresponds to DDRAM last 3 bits 

*/
esp_err_t cgram_write(lcd_pin_t lcd_handle, uint8_t* charmat, char ddram_address){

    // Convert DDRAM address last 3 bits to CGRAM address AC5-AC3 (DDRAM<<3, shift left 3)
    // for 01XXXYYY: 64+(DDRAM<<3)+y; where y is 0 to 7 for lines 1 to 8
    for (uint8_t row = 0; row < 8; row++){
        lcd_pin_data_write(lcd_handle, 0, 0, 64+(ddram_address<<3)+row, 0);
        // Convert cols of row to hex/data
        char line_dat;
        for (uint8_t col = 0; col < 5; col++){
            line_dat += (charmat[row*5+col]<<(4-col));
        }
        usleep(40);
        lcd_pin_data_write(lcd_handle, 1, 0, line_dat, 0);
        usleep(45);
        line_dat = 0;
    }
    return ESP_OK;
}

esp_err_t ddram_write_custom(lcd_pin_t lcd_handle, char DDRAM_ADDR, uint8_t row, uint8_t col){
    // Set DDRAM ADDRESS
    char addr = col;
    if (row==0){
        lcd_pin_data_write(lcd_handle, 0, 0, (addr+128), 0);
    } else if (row==1){
        addr = col + 64;
        lcd_pin_data_write(lcd_handle, 0, 0, (addr+128), 0);
    } else return ESP_ERR_INVALID_ARG;
    usleep(40);

    // Write to LCD starting at address
    lcd_pin_data_write(lcd_handle, 1, 0, DDRAM_ADDR, 0);
    usleep(45);
    return ESP_OK;
}

// TODO: Write other functionality/instructions