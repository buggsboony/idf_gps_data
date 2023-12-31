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
#include "esp_timer.h" //2023-07-27 10:06:16 - esp_timer_get_time()

const char * TAG ="GPS_DATA";

//#include "led_strip.h"
//#include "sdkconfig.h"
#include "esp_h/esp_console.h"
#include "esp_h/esp_iotools.h" //2023-06-24 22:46:55 : ln -s ../../../esp_headers/esp_h 

#include "GPS/gps_gns.cpp" //NMEA Parser
//portTICK_RATE_MS : is defined as "( ( portTickType ) 1000 / configTICK_RATE_HZ )", while configTICK_RATE_HZ is defined as 1000 also. So portTICK_RATE_MS value is 1. What is this? 1 ms per tick? 1 tick per ms?



#ifndef portTICK_RATE_MS
   #define portTICK_RATE_MS 1
#endif
//static const int RX_BUF_SIZE = 2048;
//static const int RX_BUF_SIZE = 4096;
static const int RX_BUF_SIZE = 1024;

//2023-06-24 22:01:16 - UART stuff
// PIN OUT for ESP32-c3 And adafruit ultimate GPS
// GPS TX is connected to Esp32-c3 RX (GPIO_NUM_2)
// GPS RX is connected to Esp32-c3 TX (GPIO_NUM_3)

const uart_port_t UART_PORT = (uart_port_t) 1;
#define ESP_RXD_PIN (GPIO_NUM_2)
#define ESP_TXD_PIN (GPIO_NUM_3)






#define GPS_UART_PORT UART_PORT
#define GPS_BAUD_RATE_INIT 9600

int CURRENT_BAUD_RATE = GPS_BAUD_RATE_INIT; //2023-07-27 11:22:25 - Will change
void changeGpsBaudRate() {      

    // Send command to change baud rate (replace "YOUR_GPS_COMMAND" with the actual command)
    //const char* command = "$PMTK251,115200*1F\r\n";   
    const char* command = ("$PMTK251,38400*27");  //set baud rate to 38400 
    int written = uart_write_bytes(GPS_UART_PORT, command, strlen(command));
 //int GPS_BAUD_RATE_NEW = 115200;
    int GPS_BAUD_RATE_NEW = 38400;
    printf("Writting command (written=%d) : [%s]",written,command);

    // Wait for the GPS module to process the command (adjust the delay as needed)
    vTaskDelay(pdMS_TO_TICKS(100));

    // Change the UART baud rate to match the new baud rate
    esp_err_t esp_err = uart_set_baudrate(GPS_UART_PORT, GPS_BAUD_RATE_NEW);
    if( esp_err == ESP_OK )
    {
        puts("uart_set_baudrate OK");
        uint32_t baudrate;
        esp_err = uart_get_baudrate(GPS_UART_PORT, &baudrate);
        cout<<"New baudrate is "<<baudrate<<endl;
    }
    else
    {

        //Display error without reboot
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_err);
    }

   CURRENT_BAUD_RATE = GPS_BAUD_RATE_NEW;
 // Wait for the GPS module to process the command (adjust the delay as needed)
    vTaskDelay(pdMS_TO_TICKS(2000));
}

//Main GPS LOOP
void uart_event_task(void *pvParameters)
{ 
   ESP_LOGI(pcTaskGetName(0), "Start  RX(%d) TX(%d)",ESP_RXD_PIN,ESP_TXD_PIN);
   esp_log_level_set(pcTaskGetName(0), ESP_LOG_WARN);


   ESP_LOGI(pcTaskGetName(0), "Initializing UART");
   /* Configure parameters of an UART driver,
    * communication pins and install the driver */
    uart_config_t uart_config = uart_config_t();
    uart_config.baud_rate = CURRENT_BAUD_RATE;
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
   uart_set_pin(UART_PORT, ESP_TXD_PIN, ESP_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
 
   //uart_driver_install(UART_PORT, RX_BUF_SIZE * 2, 0, 20, &uart0_queue, 0);
   uart_driver_install(UART_PORT, RX_BUF_SIZE * 2, 0, 20, NULL, 0);


   //Set uart pattern detect function.
   //uart_enable_pattern_det_intr(UART_PORT, 0x0a, 1, 10000, 10, 10); // pattern is LF
   
   //uart_enable_pattern_det_baud_intr(UART_PORT, 0x0a, 1, 9, 0, 0); // pattern is LF
   //Reset the pattern queue length to record at most 20 pattern positions.
   //uart_pattern_queue_reset(UART_PORT, 20); //2023-06-18 20:56:20 - Hangs
   ESP_LOGI(pcTaskGetName(0), "Initializing UART done");


    //2023-07-27 11:35:44 - CHange baudrate (trying)
    changeGpsBaudRate();
    
    if(false) //restart/reconnect
    {
                puts ("After changeGPSBaudRAte");
                esp_err_t esp_err = uart_driver_delete(UART_PORT);
                if( esp_err == ESP_OK )
                {
                    puts("uart_driver_delete OK");
                }
                else
                {
                    puts("uart_driver_delete FAILED");

                    //Display error without reboot
                    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_err);
                }
            /* Might be usefull to change to new BaudRAte */   
            cout<<"Reconnect with baudrate="<<CURRENT_BAUD_RATE<<endl;
                uart_config.baud_rate = CURRENT_BAUD_RATE;
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
            uart_set_pin(UART_PORT, ESP_TXD_PIN, ESP_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
            
            //uart_driver_install(UART_PORT, RX_BUF_SIZE * 2, 0, 20, &uart0_queue, 0);
          esp_err =  uart_driver_install(UART_PORT, RX_BUF_SIZE * 2, 0, 20, NULL, 0);
            
                if( esp_err == ESP_OK )
                {
                    puts("uart_driver_install again OK");
                }
                else
                {
                    puts("uart_driver_install again FAILED");

                    //Display error without reboot
                    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_err);
                }
    }




   uint8_t* data = (uint8_t*)malloc(RX_BUF_SIZE+1);
//    int msTimeout = 20; //TimeOut To wait Works With 2024 and 20ms
    int msTimeout = 630; //msTimeout/portTICK_RATE_MS TimeOut To wait Works With 2024 and 20ms
    //Pour msTimeout = 1180 et  RX_BUF_SIZE = 1024, we get 10 analyze loops in 18 secs.
  

    bool fix_found_first_time = false;
    long long int startTimer = esp_timer_get_time();
    long long int fixGPS_timer;
    float fixTimeMS = -1;
    double maxSpeed = 0;


   while(true)
   {
       const int rxbytes = uart_read_bytes(UART_PORT,data, RX_BUF_SIZE,pdMS_TO_TICKS(msTimeout) );
       printf("rxbytes=[%d]\n",rxbytes);
       if(rxbytes>0)
       {
           //printf("rxbytes=[%s]\n",data);
            vector<string> lines = explode( string((char*)data),'\n');
            printf("group line count = (%d)\n", lines.size());
            string GGA_sentence="", RMC_sentence="";

            //2023-06-25 12:26:15 - reinitialize GPS stuff
            GPS myGPS = GPS(); 
            myGPS.localOffsetHours=-4; //GMT-4           
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
                    if(!fix_found_first_time)
                    {
                        fix_found_first_time = true;                  
                        fixGPS_timer = esp_timer_get_time();//2023-07-27 10:04:05 - Get elapsed time to find a GPS Fix
                        float diffTime = (float) fixGPS_timer - startTimer;
                        fixTimeMS = (diffTime/1000);                                                                  
                    }
                    RMC_sentence = line;
                    println("RMC="+RMC_sentence,KYEL);
                }
             
                //printCoolLn("!["+line+"]!");
            }//next line in read group
          
            if(myGPS.GGA_valid)
            {
                ESP_LOGI(TAG,"GGA is OK");
                ESP_LOGW(TAG, "TIME TO GET A FIX: %f ms\n",fixTimeMS  );  
                ESP_LOGW(TAG,"maxSpeed : %f KM/H",maxSpeed); 
                ESP_LOGI(TAG,"myGPS Quality:%f",myGPS.quality);
                ESP_LOGI(TAG,"myGPS Sat count:%d",myGPS.numberSatellites);
                ESP_LOGI(TAG,"myGPS GPS_UTC=%s",myGPS.time.toString().c_str() );
                ESP_LOGI(TAG,"myGPS LAT:%f",myGPS.latitude);
                ESP_LOGI(TAG,"myGPS LNG:%f",myGPS.longitude);
                ESP_LOGW(TAG,"myGPS ALT:%f",myGPS.altitude);          
            }else
            {
                cout<<"No GGA valid found"<<endl;
                
   
            }

             if(myGPS.RMC_valid)
            {
                cout<<"**** RMC is OK ****"<<endl;
                ESP_LOGI(TAG,"RMC is OK");
                ESP_LOGI(TAG,"myGPS GPS_UTC=%s",myGPS.time.toString().c_str() );
                ESP_LOGI(TAG,"myGPS LAT:%f",myGPS.latitude);
                ESP_LOGI(TAG,"myGPS LNG:%f",myGPS.longitude); 
                ESP_LOGW(TAG,"myGPS SPD:%f KM/H",myGPS.speed); 
                if(myGPS.speed > maxSpeed ) maxSpeed = myGPS.speed; //2023-07-27 10:37:14 - Store max speed
                cout<<"************************RMC*****************"<<endl;

            }else
            {
                
                cout<<"No RMC valid found"<<endl;
            }
       }//rxbytes
       else
       {
         //2023-07-20 14:13:03 - no rx bytes :
           // ESP_LOGI(TAG,"-------------RxBytes:%d---------------",rxbytes);
       }
       int delayWait = 1;
       vTaskDelay(pdMS_TO_TICKS(delayWait) );
       //vTaskDelay(40/portTICK_PERIOD_MS);
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
    

 //vTaskDelay(pdMS_TO_TICKS(20111));
xTaskCreate(uart_event_task, "UART", 1024*4, NULL, 5, NULL);
 
   vTaskDelay(600*10);
   printf("END PROG ..............;;");



}
