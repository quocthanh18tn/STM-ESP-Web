#include <Wire.h>
#include <SPI.h>

#include "Eeprom24C32_64.h"
#include "RTClib.h"

RTC_DS1307 RTC;
#define EEPROM_ADDRESS  0x50
static Eeprom24C32_64 eeprom(EEPROM_ADDRESS);

void setup () {
 eeprom.initialize();
  Wire.begin(4, 5); // Chan 4,5 I2C cua ESP8266
  RTC.begin();      // Khoi dong RTC
  delay(500);
  // Dong bo thoi gian voi may tinh
const  char* date= "Dec 26 2009";
 const  char* time1 = "12:34:56";
  RTC.adjust(DateTime(date,time1)); 
  delay(1000);
  Serial.begin(115200); // Khoi dong serial port de lay du lieu

}
void loop() {
  DateTime now = RTC.now(); // Thoi gian = thoi gian RTC hien tai
  // In thời gian
  Serial.print(now.year(), DEC); // Năm
  Serial.print('/');
  Serial.print(now.month(), DEC); // Tháng
  Serial.print('/');
  Serial.print(now.day(), DEC); // Ngày
  Serial.print(' ');
  Serial.print(now.hour(), DEC); // Giờ
  Serial.print(':');
  Serial.print(now.minute(), DEC); // Phút
  Serial.print(':');
  Serial.print(now.second(), DEC); // Giây
  Serial.println();
  delay(1000); // Delay
     
      const word address = 0;
    
    // Write a byte at address 0 in EEPROM memory.
    Serial.println("Write byte to EEPROM memory...");
    eeprom.writeByte(address, 0xAA);
    
    // Write cycle time (tWR). See EEPROM memory datasheet for more details.
    delay(10);
    
    // Read a byte at address 0 in EEPROM memory.
    Serial.println("Read byte from EEPROM memory...");
    byte data = eeprom.readByte(address);
    
    // Print read byte.
    Serial.print("Read byte = 0x");
    Serial.print(data, HEX);
    Serial.println("");
    delay(1000); // Delay
  
}
