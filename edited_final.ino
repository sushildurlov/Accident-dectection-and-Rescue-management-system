
#include <TinyGPS++.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "Wire.h"
#include <MPU6050_light.h>
MPU6050 mpu(Wire);
#define rxPin 16
#define txPin 17
long timer = 0;
int crash = 23;
int vib = 19;
int ledPin = 5;
int push_bottom = 15;
const char* ssid = "DESKTOP-5U0341V 2371";
const char* password = "11111111";
const char* SERVER_NAME = "http://relfurn.com/gpsdata.php";
String ESP32_API_KEY = "Ad5F10jkBM0";
String vehicleno = "Ba1pa4887";
HardwareSerial neogps(1);
TinyGPSPlus gps;
int val = digitalRead(crash);
int vibval = digitalRead(vib);
void setup()
{
  Serial.begin(115200);
  Serial.println("esp32 serial initialize");
  neogps.begin(9600, SERIAL_8N1, rxPin, txPin);
  Serial.println("neogps serial initialize");
  delay(1000);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Wire.begin();
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while (status != 0) { } // stop everything if could not connect to MPU6050
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(100);
  mpu.calcOffsets(true, true); // gyro and accelero
  Serial.println("Done!\n");

  pinMode(crash, INPUT);
  pinMode(vib, INPUT);

}
void loop()
{
  mpu.update();
  int val = digitalRead(crash);
  int vibval = digitalRead(vib);
  int Push_button_state = digitalRead(push_bottom);
  if (WiFi.status() == WL_CONNECTED)
  {
    if (millis() - timer > 1000) // print data every second
    {
      float x = mpu.getAccAngleX();
      float y = mpu.getAccAngleY();
      if (x > 45.00 && y < 2.00 && vibval == 0 || x > 11.00 && y > 30.00 && vibval == 0 || x > 10.00 && y > -38.00 && vibval == 0)
      {
        Serial.println("accident detected by 1st condition");
        delay(30000);
        sendGpsToServer();
      }
      if (vibval == 0 && val == LOW)
      {
        Serial.println("accident detected by 2st condition");
        delay(30000);
        sendGpsToServer();
      }
    }

  }
}
//////////////////////////////////////////////////////////////////////////
int sendGpsToServer()
{
  while (neogps.available() > 0)
  {
    gps.encode(neogps.read());
    String latitude, longitude;
    latitude = String(gps.location.lat(), 6); // Latitude in degrees (double)
    longitude = String(gps.location.lng(), 6); // Longitude in degrees (double)
    HTTPClient http;
    //domain name with path
    http.begin(SERVER_NAME);
    if (latitude.toFloat() > 26 && longitude.toFloat() > 80)
    {
      //Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      //----------------------------------------------
      //HTTP POST request data
      String gps_data;
      gps_data = "api_key=" + ESP32_API_KEY;
      gps_data += "&latitude=" + latitude;
      gps_data += "&longitude=" + longitude;
      gps_data += "&vehicle_id=" + vehicleno;
      int httpResponseCode = http.POST(gps_data);
      String httpResponseString = http.getString();
      Serial.print("gps_data: ");
      Serial.println(gps_data);
      if (httpResponseCode > 0)
      {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.println(httpResponseString);
      }
      else
      {
        Serial.print("Error on HTTP request - Error code: ");
        Serial.println(httpResponseCode);
        Serial.println(httpResponseString);
      }

      Serial.println("accident detected ");
      for (;;);


    }
    Serial.println(gps.satellites.value());
    http.end();

  }

}
