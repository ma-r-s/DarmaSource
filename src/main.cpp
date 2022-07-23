#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SPI.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

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
#define ARRSIZE 10

const char *ssid = "Darma";
const char *password = "123456789";

Servo myservo;
DHT dht(DHTPIN, DHT11);
WebServer server(80);
StaticJsonDocument<3000> jsonDocument;
char buffer[3000];

int dist1[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int dist2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int dist3[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int humedad[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int temperatura[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int luz[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/15, /* dc=*/4, /* reset=*/2); // Nokia 5110 Display

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

void push(int val, int *arreglo)
{
  for (int i = ARRSIZE - 2; i >= 0; i--)
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
  int distance = min(duration * 0.034 / 2, 50.0); // Speed of sound wave divided by 2 (go and back)
  push(distance, arreglo);
  return distance;
}

int leerHumedad(int *arreglo)
{

  int h = random(48, 50.5);
  if (isnan(h))
  {
    Serial.println(F("Fallo leyendo humedad"));
  }
  push(h, humedad);
  return h;
}

int leerTemperatura(int *arreglo)
{

  int t = random(22, 24.5);
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
  push(val, arreglo);
  return val;
}

void pantalla(void *pvParameters)
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_helvR08_tr);
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawStr(0, 9, "Proyecto Final");
  u8g2.drawStr(0, 18, "Fund Materiales");
  u8g2.drawStr(0, 27, "Mario Ruiz");
  u8g2.drawStr(0, 36, "Nicolas Correal");
  u8g2.drawStr(0, 45, "Prf. Alba Avila");
  u8g2.sendBuffer();
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  for (;;)
  {
    u8g2.clearBuffer();
    String dato = "Temp: " + String(temperatura[0]);
    u8g2.drawStr(0, 9, dato.c_str());
    dato = "Hum: " + String(humedad[0]);
    u8g2.drawStr(44, 27, dato.c_str());
    dato = "Luz: " + String(luz[0] / 10);
    u8g2.drawStr(44, 9, dato.c_str());
    dato = "Dist: " + String(dist2[0]);
    u8g2.drawStr(44, 18, dato.c_str());
    dato = "Comi: " + String(dist1[0]);
    u8g2.drawStr(0, 27, dato.c_str());
    dato = "Agua: " + String(dist3[0]);
    u8g2.drawStr(0, 18, dato.c_str());

    u8g2.sendBuffer();
    vTaskDelay(400 / portTICK_PERIOD_MS);
  }
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

void movServo()
{
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  myservo.write(jsonDocument["ang"]);
  server.send(200, "application/json", "{}");
}

void comer()
{

  myservo.write(26);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  myservo.write(0);
  server.send(200, "application/json", "{}");
}

void agua()
{

  digitalWrite(AGUA, HIGH);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  digitalWrite(AGUA, LOW);
  server.send(200, "application/json", "{}");
}

void aniadirArreglo(char *nombre, int *arreglo)
{
  StaticJsonDocument<200> aux;
  JsonArray array = aux.to<JsonArray>();
  for (int i = 0; i < ARRSIZE; i++)
  {
    array.add(arreglo[i]);
  }
  jsonDocument[nombre] = array;
}

void getData()
{
  Serial.print("Get data");
  jsonDocument.clear();
  aniadirArreglo("luz", luz);
  aniadirArreglo("humedad", humedad);
  aniadirArreglo("temperatura", temperatura);
  aniadirArreglo("dist1", dist1);
  aniadirArreglo("dist2", dist2);
  aniadirArreglo("dist3", dist3);
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

void wifi(void *pvParameters)
{
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.enableCORS();
  server.on("/data", getData);
  server.on("/servo", HTTP_POST, movServo);
  server.on("/comer", HTTP_POST, comer);
  server.on("/agua", HTTP_POST, agua);
  server.begin();
  for (;;)
  {
    server.handleClient();
    vTaskDelay(10);
  }
}

void hardware(void *pvParameters)
{

  inicializarPines();
  for (;;)
  {
    leerDistancia(TRG1, ECH1, dist1);
    leerDistancia(TRG2, ECH2, dist2);
    leerDistancia(TRG3, ECH3, dist3);
    leerHumedad(humedad);
    leerTemperatura(temperatura);
    leerLuz(luz);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    if (dist2[0] < 30)
    {
      myservo.write(26);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      myservo.write(0);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  xTaskCreatePinnedToCore(
      wifi,   /* Task function. */
      "wifi", /* name of task. */
      10000,  /* Stack size of task */
      NULL,   /* parameter of the task */
      1,      /* priority of the task */
      &Task1, /* Task handle to keep track of created task */
      0);     /* pin task to core 0 */

  xTaskCreatePinnedToCore(
      hardware,   /* Task function. */
      "hardware", /* name of task. */
      10000,      /* Stack size of task */
      NULL,       /* parameter of the task */
      1,          /* priority of the task */
      &Task2,     /* Task handle to keep track of created task */
      1);         /* pin task to core 1 */
  xTaskCreatePinnedToCore(
      pantalla,   /* Task function. */
      "pantalla", /* name of task. */
      10000,      /* Stack size of task */
      NULL,       /* parameter of the task */
      2,          /* priority of the task */
      &Task3,     /* Task handle to keep track of created task */
      1);
}
/* pin task to core 1 */

void loop()
{
}
