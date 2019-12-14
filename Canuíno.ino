// This sites tell you all about ESP8266 setup before to run the code in arduino.
// https://eldontronics.wordpress.com/2017/08/28/beginning-iot-with-esp8266-01-wifi-module-and-cayenne-iot-platform/
//*****VERY IMPORTANT: MY 8 CH RELAY ARE NORMALLY OPEN SO I HAVE TO REVERSE !HIGH and !LOW STATE IN CODE.
///////////////////////SO IF YOU HAVE A NORNALLY CLOSED RELAY YOU SHOULD CHANGE IN CODE THE HIGH and LOW.




//Arduino Code
#include <CayenneMQTTESP8266Shield.h> //Cayenne ESP8266 wifi library
#include "RTClib.h" //RTC1307 library
#include <Wire.h> //Comunication between rtc and arduino, SCL,SDA library
#include "DHT.h" //Temperature and humidity sensor library
#include <Time.h> // time library
#include <DS1307RTC.h> //RTC1307, to set decrescent days of planted plants


char ssid[] = "NOS_Internet_Movel_C254_ext"; /// insert your name network
char wifiPassword[] = "04047761"; /// insert your router/device password

/// generated by cayenne login in arduino Mega2560 with ESP8266, choosing "Arduino Mega > Arduino Mega ESP8266 Wifi"
char username[] = "cd805270-4f14-11e9-b395-f53190c2ab6f";
char password[] = "dcb5452141ec23d783bcc0c0ef46b4a89e57cc5e";
char clientID[] = "eea1f030-1d54-11ea-a38a-d57172a4b4d4";

// Comunnication between ESP8266 and Arduino thru TX1 and RX1 on Arduino pins
#define EspSerial Serial1
ESP8266 wifi(&EspSerial);

//RTC1307 SDA SCL Connection
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif
RTC_DS1307 rtc;

//DHT22 Sensor ( 1xVegetation + 1xFlowering )
#define DHTPIN_veg 2
#define DHTPIN_flo 3
#define DHTTYPE DHT22
DHT dht_flo(DHTPIN_flo, DHTTYPE);
DHT dht_veg(DHTPIN_veg, DHTTYPE);

//8 Channel Relay pins
//HPS light
#define hps 29 // R1
int hpsOut = 29;

//CFL light
#define cfl 31 // R2
int cflOut = 31;

//Grow nutrients waterpump
#define grow 33  // R3
int growOut = 33;

//Bloom nutrients waterpump
#define bloom 35 // R4
int bloomOut = 35;

//Watering waterpump
#define rega 37 // R5
int regaOut = 37;

//HPS light Cooling tube
#define ctube 39 // R8
int ctubeOut = 39;


void setup()
{
  Serial.begin(9600);
  EspSerial.begin(115200);
  Cayenne.begin(username, password, clientID, wifi, ssid, wifiPassword);
  Wire.begin();

  ///to get the rtc on'clock, run just one time the code with and then without
  //rtc.adjust(DateTime(2019, 12, 14, 02, 49 , 0)); ///  to get the rtc on'clock, run just one time the code with and then without

  dht_veg.begin();
  dht_flo.begin();
  pinMode(29, OUTPUT); //1
  pinMode(31, OUTPUT); //2
  pinMode(33, OUTPUT); //3
  pinMode(35, OUTPUT); //4
  pinMode(37, OUTPUT); //5
  pinMode(39, OUTPUT); //8
  
  //// Because my Relays are normally open so they turn on in normal state
  digitalWrite(29, HIGH);
  digitalWrite(31, HIGH);
  digitalWrite(33, HIGH);
  digitalWrite(35, HIGH);
  digitalWrite(37, HIGH);
  digitalWrite(39, HIGH);
  digitalWrite(41, HIGH);
}

void loop() {
  Nutrients_Flowering();
  Nutrients_Vegetative();
  timerLampadas ();
  temHumiCayenne();
  soloCayenne();
  infoCayenne();
  Cayenne.loop(); // To upload/download the state of buttons in Cayenne
}



/////// Functions
/////////////////////////////Lights Operation
void timerLampadas () {
  DateTime now = rtc.now();
  /// 12 hours - Flowering Stage
  if (now.hour() >= 19 && now.hour() <= 7 )
  {
    digitalWrite(hps, HIGH);
    digitalWrite(ctube, HIGH);
  } else {
    digitalWrite(hps, LOW);
    digitalWrite(ctube, LOW);
  }
  /// 12 hours - Flowering Stage
  if ((now.hour() >= 18 && now.hour() <= 12 ))
  {
    digitalWrite(cfl, HIGH);
  } else {
    digitalWrite(cfl, LOW);
  }
}

//2x Temperature and Humidity sensors to monitoring on Cayenne
void temHumiCayenne() {
  float hum_veg = dht_veg.readHumidity(); // this -25 are for calibrate with other external sensor
  float temp_veg = dht_veg.readTemperature();
  float hum_flo = dht_flo.readHumidity(); // this -25 are for calibrate with other external sensor
  float temp_flo = dht_flo.readTemperature();
  Cayenne.virtualWrite(1, temp_veg, TYPE_TEMPERATURE, UNIT_CELSIUS);
  Cayenne.virtualWrite(2, hum_veg, TYPE_RELATIVE_HUMIDITY, UNIT_PERCENT);
  Cayenne.virtualWrite(3, temp_flo, TYPE_TEMPERATURE, UNIT_CELSIUS);
  Cayenne.virtualWrite(4, hum_flo, TYPE_RELATIVE_HUMIDITY, UNIT_PERCENT);
}

//2x Capacitive Soil Sensors to Cayenne Monitoring
void soloCayenne() {
  int output_value_cfl;
  int solo_cfl = 0;
  int output_value_hps;
  int solo_hps = 0;
  solo_cfl = analogRead(A14); // 1x capacitive soil sensor
  solo_hps = analogRead(A15); // 1x capacitive soil sensor

  // Calibration and data from sensor
  ///Dry: (520 430]     //// ( 609 - 590 - 560 ) | 0% a 23% dry
  ///Wet: (430 350]     //// ( 560 - 455 - 350 ) | 24% a 64% wet
  ///Water: (350 260]   //// ( 350 - 305 - 276 ) | 65% a 100% water
  Cayenne.virtualWrite(44, solo_hps); // Values readings from sensor from 276 to 609
  Cayenne.virtualWrite(45, solo_cfl); // Values readings from sensor from 276 to 609
}

///INFO to cayenne
void infoCayenne() {
  //Lights
  Cayenne.virtualWrite(8, !digitalRead(29));
  Cayenne.virtualWrite(9, !digitalRead(33));
  Cayenne.virtualWrite(10, !digitalRead(39));

  // Hour, Minute, Day, Month
  DateTime now = rtc.now();
  Cayenne.virtualWrite(26, now.hour());
  Cayenne.virtualWrite(27, now.minute());
  Cayenne.virtualWrite(28, now.day());
  Cayenne.virtualWrite(29, now.month());
  Cayenne.virtualWrite(30, now.year());

}

CAYENNE_IN(V18)
{
  if (getValue.asInt() == 0) {
    digitalWrite(rega, !LOW);
  }
  else {
    digitalWrite(rega, !HIGH);
  }
}

//Nutrients on every Sunday until a limit of wet
void Nutrients_Flowering()
{
  DateTime now = rtc.now();
  if ((now.dayOfTheWeek() == 6 ) && (analogRead(15) >= 400)) // now.dayOfTheWeek() == 6 can change 0-Sun,  (...) 6-Saturday
  {
    digitalWrite(bloom, !HIGH);
  }
  else
  {
    digitalWrite(bloom, !LOW);
  }
}

void Nutrients_Vegetative()
{
  DateTime now = rtc.now();
  if ((now.dayOfTheWeek() == 6 ) && (analogRead(14) >= 400)) // now.dayOfTheWeek() == 6 can change 0-Sun,  (...) 6-Saturday
  {
    digitalWrite(grow, !HIGH);
  }
  else
  {
    digitalWrite(grow, !LOW);
  }
}
//To acess this button on Cayenne you must do Cayenne Web
// Add new...> Widget> Actuators > Generic > Digital > fill info request
CAYENNE_IN(V19)
{
  setSyncProvider(RTC.get);
  time_t daysToHarvest, femiDias;
  femiDias = (1586736000L - 1575922807L); //  ( 5 de Mai  - 1 jan 2020 ) = 126 dias
  int diaZero = 0;
  diaZero = now();
  // days to harvest
  daysToHarvest = ((femiDias - diaZero) / 60 / 60 / 24);
  //days passed
  int daysPassed = 0;
  daysPassed = ((femiDias - (diaZero + femiDias) / 60 / 60 / 24));
  // week passed
  int weekWeAreOn = 0;
  weekWeAreOn = (18 - (femiDias - diaZero) / 60 / 60 / 24 / 7);

  if (getValue.asInt() == 1) {
    Cayenne.virtualWrite(47, daysToHarvest); //
    Cayenne.virtualWrite(48, daysPassed); //
    Cayenne.virtualWrite(49, weekWeAreOn); //
  }
  else {
    Cayenne.virtualWrite(47, 0.00);
    Cayenne.virtualWrite(48, 00.0);
    Cayenne.virtualWrite(49, 000); //1 ou 0
  }
}

CAYENNE_IN(V20)
{
  setSyncProvider(RTC.get);
  time_t daysToHarvest, femiDias;
  femiDias = (1586736000L - 1575922807L); //  ( 5 de Mai  - 1 jan 2020 ) = 126 dias
 int diaZero = 0;
  diaZero = now();
  // days to harvest
  daysToHarvest = ((femiDias - diaZero) / 60 / 60 / 24);
  //days passed
  int daysPassed = 0;
  daysPassed = ((femiDias - (diaZero + femiDias) / 60 / 60 / 24));
  // week passed
  int weekWeAreOn = 0;
  weekWeAreOn = (18 - (femiDias - diaZero) / 60 / 60 / 24 / 7);

  if (getValue.asInt() == 0) {
    Cayenne.virtualWrite(50, daysToHarvest); //
    Cayenne.virtualWrite(51, daysPassed); //
    Cayenne.virtualWrite(52, weekWeAreOn); //
  }
  else {
    Cayenne.virtualWrite(50, 0.00);
    Cayenne.virtualWrite(51, 00.0);
    Cayenne.virtualWrite(52, 000); //1 ou 0
  }
}

CAYENNE_IN(V21)
{
  setSyncProvider(RTC.get);
  time_t daysToHarvest, autoDias;
  autoDias = (1582588800L - 1575922807L); // ( 25 de FEV  - 1 jan 2020 ) = 77 dias
  int diaZero = 0;
  diaZero = now();
  // days to harvest
  daysToHarvest = ((autoDias - diaZero) / 60 / 60 / 24);
  //days passed
  int daysPassed = 0;
  daysPassed = (autoDias-((diaZero + autoDias) / 60 / 60 / 24));
  // week passed
  int weekWeAreOn = 0;
  weekWeAreOn = (11 - (autoDias - diaZero) / 60 / 60 / 24 / 7);

  if (getValue.asInt() == 1) {
    Cayenne.virtualWrite(53, daysToHarvest); //
    Cayenne.virtualWrite(54, daysPassed); //
    Cayenne.virtualWrite(55, weekWeAreOn); //
  }
  else {
    Cayenne.virtualWrite(53, 0.00);
    Cayenne.virtualWrite(54, 00.0);
    Cayenne.virtualWrite(55, 000); //1 ou 0
  }
}
