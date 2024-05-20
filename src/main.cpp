/*
    กฎการเขียนโปรแกรม C++
    1.
*/

// #include <แทนชื่อ library ที่อยากใช้.h>
#include <Arduino.h>

// #define [ชื่อตัวแปรที่ต้องการ] [ค่าเริ่มต้น]
// #define เปลี่ยนแปลงค่าทีหลังไม่ได้
#define LED_A_PIN 8
#define LED_B_PIN 9

// function ที่ สร้างเอง
/* 
    โครงสร้างการประกาศ function
    [ชนิดของค่าที่อยากให้return] [ชื่อของfunction] ([ชนิดของค่าที่อยากให้รับเข้า 1] [ชื่อตัวแปรรับค่าเข้า 1], [ชนิดของค่าที่อยากให้รับเข้า 2] [ชื่อตัวแปรรับค่าเข้า 2]) {
        [คําสั่งที่ต้องการทำงาน]
    }

    ตัวอย่างการประกาศ function
    int addNumber(int a, int b) { 
        return a + b; 
    }

    ตัวอย่างเรียกใช้ function ในการประกาศ function
    function 1
    int addNumber(int a, int b) { 
        return a + b;
    }

    function 2
    bool isPlusResultGreaterThanTen(int a, int b) {
        bool result = addNumber(a, b) > 10;
        return result;
    }

    ตัวอย่างการเรียกใช้ function
    void setup() {
        addNumber(1, 2);
    }

    ตัวอย่างการเรียกใช้ function และเก็บค่าที่ return ในตัวแปร
    void loop() {
        int result = addNumber(3, 4); // ค่าที่ return จาก function เก็บไว้ในตัวแปร result
    }
*/

// function ที่ run ครั้งเดียวเมื่อบอร์ดเริ่มทำงาน
void setup() {
}

// function ที่ run วนตลอดจนกว่าบอร์ดจะหยุด
void loop() {
}