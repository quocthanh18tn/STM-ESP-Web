/**
   BasicHTTPClient.ino
   Created on: 01.02.2019
   Author: Phan Quoc Thanh
   Project: Communicate between STM + ESP + Webbrowser
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
#include "Eeprom24C32_64.h"

//function user define
void formattime(void);
// end function user define

#define EEPROM_ADDRESS 0x50       //address EEPROM when you use ESP, if you use stm32 => address must be 0xA0
#define FORMATPRODUCT  30       //Number 20, date and time ( Month, day, hour, min, second) 10 
static Eeprom24C32_64 eeprom(EEPROM_ADDRESS);

RTC_DS1307 RTC;
const char * ssid="Baodeptrai";
const char * pass="thanhdeptrai";
const char * url="http://192.168.137.1/test.php";
bool FlagCompareTime=false;

String postData="",station="",payload="";
HTTPClient http;
int httpCode;
byte BufferOutput[32];
byte BufferInput[5]={'0','0','0','0','0'};
word IndexBufferOutput=0;
int  Counts=0;   
void setup() 
{
          FlagCompareTime=false;
         
          Wire.begin(4, 5); // Chan 4,5 I2C cua ESP8266
          RTC.begin();      // Khoi dong RTC
          Serial.begin(115200);                 //Serial connection
          //WiFi.persistent( false );
          WiFi.begin(ssid, pass);   //WiFi connection          
        //  eeprom.writeBytes(0,2 , BufferInput);
          delay(1000);
          
          // readbytes format: address, numerous, pointer.
          eeprom.readBytes(0,2 , BufferOutput);
          // BufferOutput contain number of products in eeprom
          for (IndexBufferOutput=0;IndexBufferOutput<2 ;IndexBufferOutput++)
          {      
          //Serial.println(Counts);
          Counts+=BufferOutput[IndexBufferOutput]-48;
          //Serial.println(BufferOutput[IndexBufferOutput]);
          }
          delay(1000);          
          Serial.println("..........");
          Serial.println(Counts);
}
 
void loop() 
{
           delay(1000);
           station="";
           Serial.println("..........");
           if (!FlagCompareTime && (WiFi.status() == WL_CONNECTED))
              {
                  Serial.println("sync time");
                  FlagCompareTime=true;
                  http.begin( url);
                  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
                  // 1 la send de update time
                  station="1";
                  // test la variable tren server to receive value from esp
                  postData = "test="+station;
                  httpCode = http.POST(postData);   //Send the request
                  payload = http.getString();                  //Get the response payload
                  //Serial.println("data:"+payload);    // print to test format date time from server
                  http.end();  //Close connection 
                  String Year,Month,Day,Hour,Min,Second;
                  Year=(payload.substring(0,4));
                  Month=(payload.substring(5,7));
                  Day=(payload.substring(8,10));
                  Hour=(payload.substring(11,13));
                  Min=(payload.substring(14,16));
                  Second=(payload.substring(17));
                 // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
                  RTC.adjust(DateTime(Year.toInt(),Month.toInt(),Day.toInt(),Hour.toInt(),Min.toInt(),Second.toInt())); 
                  Serial.println(" end sync time");   
             }

          if (Counts>0 &&(WiFi.status() == WL_CONNECTED))
            {
                 Serial.println("syn eeprom");
                 //Serial.println(Counts);
                 //Serial.println(Counts*32);
                 station="";
                 eeprom.readBytes(Counts*32, FORMATPRODUCT+1, BufferOutput);
                 if( BufferOutput[0]!='0')
                 {
                  for (IndexBufferOutput=1;IndexBufferOutput<=FORMATPRODUCT+1;IndexBufferOutput++)
                      {
                        if ((BufferOutput[IndexBufferOutput]-48)>=0)
                             station+=BufferOutput[IndexBufferOutput]-48;    
                         //Serial.println("data to send:"+station);
                      }
                  Serial.println("data to send:"+station);   
                  http.begin( url);
                  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
                  postData = "test=" + station ;
                  httpCode = http.POST(postData);   //Send the request
                  payload = http.getString();                  //Get the response payload
                  if (payload =="1")                    //return from server indicate success if = 1 or 404 : error
                  Serial.println("data:"+payload);
                  http.end();  //Close connection
                  eeprom.writeBytes(Counts*32, 5 , BufferInput);     
                  //delay(1000);
                 }
                  Serial.println("end syn eeprom");    
                  Counts--;   
                  if (Counts==0)
                  {
                  eeprom.writeBytes(0, 5 , BufferInput);
                  Serial.println(Counts);    
                  }
          }
      
  //uart receive data from another hardware
          if ((Serial.available() > 0)&&(WiFi.status() == WL_CONNECTED)) 
           {
              Serial.println("send data connected");
              formattime();
              station+=Serial.readStringUntil('*');
              Serial.println("data is sned:"+station);           
              // wait for WiFi connection   
              http.begin( url);
              http.addHeader("Content-Type", "application/x-www-form-urlencoded");
              postData = "test=" + station ;
              httpCode = http.POST(postData);   //Send the request
              payload = http.getString();                  //Get the response payload
              Serial.println("data:"+payload);
              http.end();  //Close connection 
              Serial.println("end send data connected");       
           }
          else if ((WiFi.status() != WL_CONNECTED))
           {
            if (Serial.available() > 0)
            {
               Serial.println("write eeprom");   
               station="1";
               formattime();
               station+=Serial.readStringUntil('*');
               Serial.println("data to write:"+station);
               //write eeprom
               Counts++;
               Serial.println(Counts);
              byte bufferwrite[32],index;
              for (index=0;index<32;index++)
              bufferwrite[index]=station[index];
              eeprom.writeBytes(Counts*32,FORMATPRODUCT+1 , bufferwrite);
              if ( Counts>=256)
                  bufferwrite[1]=Counts-256+48;
              else
                  bufferwrite[1]=48;      
              bufferwrite[0]=Counts+48;        
              eeprom.writeBytes(0,2 , bufferwrite);
              Serial.println("end write eeprom");
            }
            else
            {
              //turn on led to warning disconect to reset mcu
                Serial.println("just disconnected switch LPM");
                delay(500);
            }   
           }
        }
        
        void formattime(void)
        {
               DateTime now = RTC.now(); // Thoi gian = thoi gian RTC hien tai
            //  station+=now.year();
              if(now.month()>9)
              station+=now.month();
              else
              {
              station+="0";
              station+=now.month();  
              }
              if(now.day()>9)
              station+=now.day();
              else
              {
              station+="0";
              station+=now.day();  
              }
              if(now.hour()>9)
              station+=now.hour();
              else
              {
              station+="0";
              station+=now.hour();  
              }      
              if(now.minute()>9)
              station+=now.minute();
              else
              {
              station+="0";
              station+=now.minute();  
              }
              if(now.second()>9)
              station+=now.second();
              else
              {
              station+="0";
              station+=now.second();  
              }
             
}
