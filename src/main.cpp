#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <I2CKeyPad.h> 
#include <Keypad.h>

I2CKeyPad keypad(0x20);             //  เป็นคำสั่งเก็บค่า address ของ keypad address = 0x20
LiquidCrystal_I2C lcd(0x27, 16, 2); //  เป็นการตั้งค่า ของจอ Lcd (0*27 คือขนาดของจอ,16 ตัวอักษร ,2 บรรทัด)

char keymap[19] = "123A456B789C*0#DNF"; //  เป็นคำสั่งใช้ตัวแปร char โดยชื่อ keymap เป็นตัวเก็บจำนวนไว้ที่ตัวแปร ของ array

                                       // สร้างออบเจ็ค Keypad_I2C

String inputTime = "";

void setup() {                        // เริ่มต้นการทำงานของ I2C
 
  Wire.begin();
                                      // เริ่มต้นการทำงานของ LCD
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Enter Time:");
  lcd.setCursor(0, 1);
      
  keypad.begin();                     // เริ่มต้นการทำงานของ Keypad
}

char getkeypadPressed()
{
    if (keypad.isPressed())
    {
        char ch = keypad.getChar();
        return ch;
    }
    else
    {
        return ' ';
    }

void loop() {
  char re = getkeypadPressed();
  
  if (re) {
    if (re == '#') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time:");
      lcd.setCursor(0, 1);
      lcd.print(inputTime);
      inputTime = "";
    } else if (re == '*') {
      inputTime = "";String inputTime = "";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Time:");
      lcd.setCursor(0, 1);
    } else {
      inputTime += re;
      lcd.setCursor(0, 1);
      lcd.print(inputTime);
    }
  }
}

