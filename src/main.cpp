#include <Arduino.h>
#include <Wire.h>                       //    เป็นคำสั่งเรียกใช้ Libary wire (wire เป็นคำสั่งที่ใน arduino)
#include <LiquidCrystal_I2C.h>          //    เป็นคำสั่งเรียกใช้ Libary ของ lcd i2c

LiquidCrystal_I2C lcd(0x27, 16, 2);     //    เป็นการตั้งค่า ของจอ Lcd (0*27 คือขนาดของจอ,16 ตัวอักษร ,2 บรรทัด)

void setup() {
}
  lcd.init();                           //    เป็นการเริ่มต้นของ lcd
  lcd.backlight();                      //    เป็นคำสั่งให้ backlight ของ lcd ติด
  

void loop() {
}