#include <Arduino.h>
#include <Wire.h>                       //    เป็นคำสั่งเรียกใช้ Libary wire (wire เป็นคำสั่งที่ใน arduino)
#include <LiquidCrystal_I2C.h>          //    เป็นคำสั่งเรียกใช้ Libary ของ lcd i2c
#include <I2CKeyPad.h>                  //    เป็นคำสั่งเรียกใช้ libary keypad i2c


LiquidCrystal_I2C lcd(0x27, 16, 2);      //    เป็นการตั้งค่า ของจอ Lcd (0*27 คือขนาดของจอ,16 ตัวอักษร ,2 บรรทัด)
I2CKeyPad keypad(0x20);                  //    เป็นคำสั่งเก็บค่า address ของ keypad address = 0x20
char keymap[19] = "123A456B789C*0#DNF";  //    เป็นคำสั่งใช้ตัวแปร char โดยชื่อ keymap เป็นตัวเก็บจำนวนไว้ที่ตัวแปร ของ array

void setup() {

  Wire.begin();                          //    เป็นการเริ่มใช้งาน Libary wire
  Wire.setClock(400000);                 //    เป็นคำสั่งตั้งค่าความเร็วในการสื่อสาร (400000 fast mode )
  lcd.init();                            //    เป็นการเริ่มต้นของ lcd
  lcd.backlight();                       //    เป็นคำสั่งให้ backlight ของ lcd ติด 
      if (keypad.begin() == false) {     //    ถ้า (keypad.begin เป็นการตรวจสอบว่าสื่อสารกันได้) keypad เป็น เท็จ  
      lcd.print ("\nkeypadError");         
      while (1)  
  }
  keypad.loadKeyMap(keymap);             //    เป็นการตั้งค่า layout ของ keypad เป็นการดึงค่าจาก keymap มา                         
}



void loop() {
}