#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <ESP32Servo.h>
Servo myservo;
U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/15, /* dc=*/4, /* reset=*/2); // Nokia 5110 Display
int ang = 0;
void setup()
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_helvR08_tr);
  pinMode(32, INPUT_PULLUP);
  myservo.attach(16);
  myservo.write(0);
}

void loop()
{
  if (!digitalRead(32))
  {
    ang += 10;
    myservo.write(ang);
  }
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);

  u8g2.drawDisc(42, 30, 6);
  u8g2.drawLine(42, 30, 42 - 29 * cos(ang * 0.0174533), 30 - 29 * sin(ang * 0.0174533));
  u8g2.drawCircle(42, 31, 30);
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 32, 84, 20);
  u8g2.setDrawColor(1);

  u8g2.drawStr(0, 48, "Fund Materiales");

  u8g2.sendBuffer();

  delay(150);
}