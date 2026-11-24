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
    // Testing CGRAM custom character write:
    uint8_t hg_1[8][5] = {{1, 1, 1, 1, 1}, 
                            {1, 1, 1, 1, 1}, 
                            {0, 1, 1, 1, 0}, 
                            {0, 0, 1, 0, 0}, 
                            {0, 1, 0, 1, 0},
                            {1, 0, 0, 0, 1},
                            {1, 1, 1, 1, 1},
                            {0, 0, 0, 0, 0}};
    cgram_write(lcd_1602, &(hg_1[0][0]), CGRAM_ADDR_1);
    // ddram_write_custom(lcd_1602, CGRAM_ADDR_1, 1, 5);

    uint8_t hg_2[8][5] = {{1, 1, 1, 1, 1}, 
                            {1, 0, 0, 0, 1}, 
                            {0, 1, 1, 1, 0}, 
                            {0, 0, 1, 0, 0}, 
                            {0, 1, 0, 1, 0},
                            {1, 1, 1, 1, 1},
                            {1, 1, 1, 1, 1},
                            {0, 0, 0, 0, 0}};
    cgram_write(lcd_1602, &(hg_2[0][0]), CGRAM_ADDR_2);
    // ddram_write_custom(lcd_1602, CGRAM_ADDR_3, 1, 6);
    
    uint8_t hg_3[8][5] = {{1, 1, 1, 1, 1}, 
                            {1, 0, 0, 0, 1}, 
                            {0, 1, 0, 1, 0}, 
                            {0, 0, 1, 0, 0}, 
                            {0, 1, 1, 1, 0},
                            {1, 1, 1, 1, 1},
                            {1, 1, 1, 1, 1},
                            {0, 0, 0, 0, 0}};
    cgram_write(lcd_1602, &(hg_3[0][0]), CGRAM_ADDR_3);
    // ddram_write_custom(lcd_1602, CGRAM_ADDR_3, 1, 7);

    int count= 0;
    char count_buffer[16];
    char anim = 0;
    while(1){
        // uint8_t count_index = 0;
        sprintf(count_buffer, "%u", count);
        num_writer(lcd_1602, count, 0, 2, 4);
        ddram_write_custom(lcd_1602, anim, 1, 5);
        // printf("%x\n", anim);
        if (anim>=2){
            anim=0;
        }else anim++;

        // ESP_LOGI("TESTING", "Task while loop");
        count++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
