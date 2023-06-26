#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
//#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "driver/uart.h"
//#include "led_strip.h"
//#include "sdkconfig.h"
#include "esp_h/esp_console.h"
#include "esp_h/esp_iotools.h" //2023-06-24 22:46:55 : ln -s ../../../esp_headers/esp_h 

#include "GPS/gps_gns.cpp" //NMEA Parser
//portTICK_RATE_MS : is defined as "( ( portTickType ) 1000 / configTICK_RATE_HZ )", while configTICK_RATE_HZ is defined as 1000 also. So portTICK_RATE_MS value is 1. What is this? 1 ms per tick? 1 tick per ms?


#ifndef portTICK_RATE_MS
   #define portTICK_RATE_MS 1
#endif
static const int RX_BUF_SIZE = 2024;


//2023-06-24 22:01:16 - UART stuff
const uart_port_t UART_PORT = (uart_port_t) 1;
#define TXD_PIN2 (GPIO_NUM_3)
#define RXD_PIN2 (GPIO_NUM_2)

void uart_event_task(void *pvParameters)
{ 
   ESP_LOGI(pcTaskGetName(0), "Start  RX(%d) TX(%d)",RXD_PIN2,TXD_PIN2);
   esp_log_level_set(pcTaskGetName(0), ESP_LOG_WARN);


   ESP_LOGI(pcTaskGetName(0), "Initializing UART");
   /* Configure parameters of an UART driver,
    * communication pins and install the driver */
    uart_config_t uart_config = uart_config_t();
    uart_config.baud_rate = 9600;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    #if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0))
        uart_config.source_clk = UART_SCLK_DEFAULT;
    #else
        uart_config.source_clk = UART_SCLK_APB;
    #endif
      
       
 
   uart_param_config(UART_PORT, &uart_config);


   //Set UART pins (using UART0 default pins ie no changes.)
   uart_set_pin(UART_PORT, TXD_PIN2, RXD_PIN2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
 
   //uart_driver_install(UART_PORT, RX_BUF_SIZE * 2, 0, 20, &uart0_queue, 0);
   uart_driver_install(UART_PORT, RX_BUF_SIZE * 2, 0, 20, NULL, 0);


   //Set uart pattern detect function.
   //uart_enable_pattern_det_intr(UART_PORT, 0x0a, 1, 10000, 10, 10); // pattern is LF
   
   //uart_enable_pattern_det_baud_intr(UART_PORT, 0x0a, 1, 9, 0, 0); // pattern is LF
   //Reset the pattern queue length to record at most 20 pattern positions.
   //uart_pattern_queue_reset(UART_PORT, 20); //2023-06-18 20:56:20 - Hangs
   ESP_LOGI(pcTaskGetName(0), "Initializing UART done");


     uint8_t* data = (uint8_t*)malloc(RX_BUF_SIZE+1);
    while(1)
   {
       const int rxbytes = uart_read_bytes(UART_PORT,data, RX_BUF_SIZE,1000/portTICK_RATE_MS);
       printf("rxbytes=[%d]\n",rxbytes);
       if(rxbytes>0)
       {
           //printf("rxbytes=[%s]\n",data);
            vector<string> lines = explode( string((char*)data),'\n');
            printf("group count = (%d)\n", lines.size());
            string GGA_sentence="", RMC_sentence="";

            //2023-06-25 12:26:15 - reinitialize GPS stuff
            GPS myGPS = GPS();            
            myGPS.GGA_valid=false;
            myGPS.RMC_valid=false;
            for(string line:lines)
            {            
                line = trimStr(line);
               
                if(myGPS.checkGGA(line,true) )
                {
                    GGA_sentence = line;
                    println("GGA="+GGA_sentence,KMAG);       
                }
                
                if(myGPS.checkRMC(line,true) )
                {          
                    RMC_sentence = line;
                    println("RMC="+RMC_sentence,KYEL);
                }
             
                //printCoolLn("!["+line+"]!");
            }//next line in read group
          
            if(myGPS.GGA_valid)
            {
                cout<<"GGA is OK"<<endl;
                cout<<"myGPS LAT:"<<myGPS.latitude<<endl;
                cout<<"myGPS LNG:"<<myGPS.longitude<<endl;
                cout<<"myGPS ALT:"<<myGPS.altitude<<endl;
                cout<<"myGPS SPD:"<<myGPS.speed<<endl;
            }else
            {
                cout<<"No GGA valid found"<<endl;
            }

             if(myGPS.RMC_valid)
            {
                cout<<"RMC is OK"<<endl;
                cout<<"myGPS LAT:"<<myGPS.latitude<<endl;
                cout<<"myGPS LNG:"<<myGPS.longitude<<endl;       
                cout<<"myGPS SPD:"<<myGPS.speed<<endl;
            }else
            {
                cout<<"No RMC valid found"<<endl;
            }
       }//rxbytes
       vTaskDelay(100/portTICK_PERIOD_MS);
   }//wend
   // never reach here
   free(data);
   data = NULL;
   vTaskDelete(NULL);
}//uart_event_task


extern "C" void app_main(void)
{


    printf("  REady  ---------------------------------------\n ");
    vTaskDelay(100); //wait a bit
 
   xTaskCreate(uart_event_task, "UART", 1024*4, NULL, 5, NULL);


   vTaskDelay(600*10); //wait a 60 sec
   printf("END PROG ..............;;");



}
