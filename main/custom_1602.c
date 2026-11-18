#include <driver/gpio.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <math.h>
#define PIN_NUM_CONTROL 4
#include <c_driver/1602-LCD-Driver.h>

lcd_pin_t lcd_1602 = {23, 21, 22, 16, 17, 18, 19};


void app_main(void)
{
    ESP_LOGI("TESTING", "CHECKPOINT 1");
    // Using self writer driver 
    lcd_pin_init_config(lcd_1602);
    lcd_reset(lcd_1602);
    // Print template
    // 2x16 grid of available characters 
    //   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    // 0 V = # # # # m V | R A  W  #  #  #  #  
    // 1 P R O X : T R I G G E  R  E  D
    char row_1_buff[17];
    char row_2_buff[17];
    sprintf(row_1_buff, "V=####mV|RAW####");
    sprintf(row_2_buff, "PROX:      ");
    ddram_write(lcd_1602, row_1_buff, 0, 0);
    ddram_write(lcd_1602, row_2_buff, 1, 0);
    ESP_LOGI("TESTING", "CHECKPOINT 2");
    uint8_t count=0;
    char count_buffer[16];
    while(1){
        uint8_t count_index = 0;
        sprintf(count_buffer, "%u", count);
        while(count_buffer[count_index] != '\0'){
            ddram_write(lcd_1602, &(count_buffer[count_index]), 0, 2+count_index);
            count_index++;
        } // Should probably turn this into a function in the driver file, but im lazy.
        // ESP_LOGI("TESTING", "Task while loop");
        count++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
