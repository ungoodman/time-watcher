/*
    กฎการเขียนโปรแกรม C++
    0. โปรแกรมจะทำตามลําดับจากซ้ายไปขวา บนลงล่าง
    1. ใส่ ; ทุกบรรทัดที่เป็น function หรือ ตัวแปร
    2. เรียก function หรือ ตัวแปร จากข้างนอก {} มาข้างในได้
    3. เจอ return ใน function จะ return ค่าที่เก็บไว้ในตัวแปร แล้วหยุดโปรแกรมใน function
*/

// #include <แทนชื่อ library ที่อยากใช้.h>
#include <Arduino.h>

// #define [ชื่อตัวแปรที่ต้องการ] [ค่าเริ่มต้น]
// #define เปลี่ยนแปลงค่าทีหลังไม่ได้
// #define LED_A_PIN 8
// #define LED_B_PIN 9

// ตัวแปร
/*
    // ชนิดของตัวแปร
    // int = จำนวนเต็ม (..., -1, 0, 1, ...)
    // float = จำนวนทศนิยม (..., -1.0, 0.0, 1.0, ...)
    // char = ตัวอักษร ('a', 'B', '3', ' ')
    // bool = true/false
    // String = ข้อความ ("Hello, World!", "Foo, Bar!", "", " ", ...)

    // โครงสร้างการประกาศ ตัวแปร

    // รูปแบบที่ 1
    [ชนิดของตัวแปร] [ชื่อตัวแปร] = [ค่าเริ่มต้น];

    // ตัวอย่าง
    int number = 10;

    // รูปแบบที่ 2
    [ชนิดของตัวแปร] [ชื่อตัวแปร];
    [ชื่อตัวแปร] = [ค่าที่อยากให้เก็บ];

    // ตัวอย่าง
    String title;
    title = "Hello, World!";

    // การเปลี่ยนค่าตัวแปร
    [ชื่อตัวแปร] = [ค่าที่อยากให้เก็บ];

    // ตัวอย่าง
    String title = "Hello, World!";
    title = "Foo, Bar!";

    // ประกาศตัวแปรแบบเปลี่ยนค่าภายหลังไม่ได้
    const [ชนิดของตัวแปร] [ชื่อตัวแปร] = [ค่าที่อยากให้เก็บ];

    ตัวอย่าง
    const int MAX_NUMBER = 10;
*/

// function ที่ สร้างเอง
/* 
    function คือ แม่แบบของคําสั่งที่ต้องการทำงาน เรียกใช้งานได้ รับค่าเข้าได้ ส่งคืนค่าได้

    ชนิดของค่าที่returnได้
    int = จำนวนเต็ม
    float = จำนวนทศนิยม
    char = ตัวอักษร
    bool = true/false
    String = ข้อความ
    void = ไม่ต้องมีค่าreturn

    โครงสร้างการประกาศ function
    [ชนิดของค่าที่อยากให้return] [ชื่อของfunction] ([ชนิดของค่าที่อยากให้รับเข้า 1] [ชื่อตัวแปรรับค่าเข้า 1], [ชนิดของค่าที่อยากให้รับเข้า 2] [ชื่อตัวแปรรับค่าเข้า 2]) {
        [คําสั่งที่ต้องการทำงาน]
    }

    // ตัวอย่างการประกาศ function
    int addNumber(int a, int b) { 
        return a + b; 
    }

    // ตัวอย่างการเรียกใช้ function
    void setup() {
        addNumber(1, 2);
    }

    // ตัวอย่างการเรียกใช้ function และเก็บค่าที่ return ในตัวแปร
    void loop() {
        int number = addNumber(3, 4); // ค่าที่ return จาก function เก็บไว้ในตัวแปร result
    }

    // ตัวอย่างเรียกใช้ function ในการประกาศ function
    // function 1
    int addNumber(int a, int b) { 
        return a + b;
    }

    // function 2
    bool isPlusResultGreaterThanTen(int a, int b) {
        bool result = addNumber(a, b) > 10;
        return result;
    }
*/

// การใช้ if else
/*
    // โครงสร้างการประกาศ if else
    if ([เงื่อนไข]) { // จำเป็นต้องมี
        [คําสั่งที่ต้องการทำงาน]
    } else if ([เงื่อนไข]) { // ไม่จำเป็นต้องมี
        [คําสั่งที่ต้องการทำงาน]
    } else { // ไม่จำเป็นต้องมี
        [คําสั่งที่ต้องการทำงาน]
    }

    // ตัวอย่าง
    if (number > 0) {
        Serial.println("Number is positive");
    } else if (number < 0) {
        Serial.println("Number is negative");
    } else {
        Serial.println("Number is zero");
    }

    // ตัวอย่าง เรียกใช้ function ในเงื่อนไข
    bool isNumberPositive(int inputNumber) {
        return inputNumber > 0;    
    }

    bool isNumberNegative(int inputNumber) {
        return inputNumber < 0;    
    }

    if (isNumberPositive(number)) {
        Serial.println("Number is positive");
    } else if (isNumberNegative(number)) {
        Serial.println("Number is negative");
    } else {
        Serial.println("Number is zero");
    }
*/

// function ที่ run ครั้งเดียวเมื่อบอร์ดเริ่มทำงาน
void setup() {
    /*
        ประกาศตัวแปร
        หรือ
        เรียกใช้ function ที่ สร้างเอง
    */
}

// function ที่ run วนตลอดจนกว่าบอร์ดจะหยุด
void loop() {
    /*
        ประกาศตัวแปร
        หรือ
        เรียกใช้ function ที่ สร้างเอง
    */
}