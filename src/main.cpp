#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SPI.h>
#include <ESP32Servo.h>

#define DHTPIN 17

#define TRG1 14
#define ECH1 27
#define TRG2 26
#define ECH2 25
#define TRG3 21
#define ECH3 19
#define DHTPIN 17
#define LUZ 33
#define SERV 16
#define AGUA 22
#define BOTON 32

Servo myservo;
DHT dht(DHTPIN, DHT11);

int dist1[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int dist2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int dist3[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int humedad[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int temperatura[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int luz[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/15, /* dc=*/4, /* reset=*/2); // Nokia 5110 Display

void push(int val, int *arreglo)
{
  int size = sizeof(arreglo);
  for (int i = size - 2; i >= 0; i--)
  {
    arreglo[i + 1] = arreglo[i];
  }
  arreglo[0] = val;
}

int leerDistancia(int trigPin, int echoPin, int *arreglo)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  int distance = min(duration * 0.034 / 2, 30.0); // Speed of sound wave divided by 2 (go and back)
  push(distance, arreglo);
  return distance;
}

int leerHumedad(int *arreglo)
{
  int h = dht.readHumidity();
  if (isnan(h))
  {
    Serial.println(F("Fallo leyendo humedad"));
  }
  push(h, humedad);
  return h;
}

int leerTemperatura(int *arreglo)
{

  int t = dht.readTemperature();
  if (isnan(t))
  {
    Serial.println(F("Fallo leyendo temperatura"));
  }
  push(t, temperatura);
  return t;
}

int leerLuz(int *arreglo)
{
  int val = analogRead(LUZ);
  Serial.print("Luz: ");
  Serial.println(val);
  push(val, arreglo);
  return val;
}

void pantalla()
{
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);

  u8g2.drawDisc(42, 30, 6);
  // u8g2.drawLine(42, 30, 42 - 29 * cos(ang * 0.0174533), 30 - 29 * sin(ang * 0.0174533));
  u8g2.drawCircle(42, 31, 30);
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 32, 84, 20);
  u8g2.setDrawColor(1);
  // String d1 = String(leerDistancia(14, 27));
  // String d2 = String(leerDistancia(26, 25));
  // String resp = "D1:" + d1 + " D2:" + d2;

  // u8g2.drawStr(0, 48, resp.c_str());

  u8g2.sendBuffer();
}

void inicializarPines()
{
  pinMode(BOTON, INPUT_PULLUP);
  pinMode(TRG1, OUTPUT);
  pinMode(ECH1, INPUT);
  pinMode(TRG2, OUTPUT);
  pinMode(ECH2, INPUT);
  pinMode(TRG3, OUTPUT);
  pinMode(ECH3, INPUT);
  pinMode(AGUA, OUTPUT);
  myservo.attach(SERV);
  myservo.write(0);

  dht.begin();
}

void setup()
{
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(u8g2_font_helvR08_tr);
  inicializarPines();
}

void loop()
{
  pantalla();
  leerDistancia(TRG1, ECH1, dist1);
  leerDistancia(TRG2, ECH2, dist2);
  leerDistancia(TRG3, ECH3, dist3);
  leerHumedad(humedad);
  leerTemperatura(temperatura);
  leerLuz(luz);
  delay(2000);
}