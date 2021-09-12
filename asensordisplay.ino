#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SD.h>
#include "max6675.h"
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
//#include <Fonts/Org_01.h>

#define thermoDO  4
#define thermoCS  5
#define thermoCLK 6
#define TFT_CS    7
#define TFT_RST   8
#define TFT_SD    9
#define TFT_DC    10

Adafruit_ST7789 screen = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
SoftwareSerial gpsSerial(2,3);
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
TinyGPSPlus gps;

float sValPrev[5];


void setup() {
  Serial.begin(115200);

  gpsSerial.begin(9600);

  screen.init(240, 320);
  screen.fillScreen(ST77XX_BLACK);
  screen.setTextSize(2);
//  screen.setRotation(0);
  screen.setTextColor(ST77XX_BLUE);
  if (!SD.begin(TFT_SD)) {
    screen.println(F("!SD init failed!"));
    screen.println(F("!No datalogging!"));
  }
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(F("Date,Time,Speed_kmph,Oil_PSI,Fuel_PSI,AFR,Oil_Temp_C"));
    dataFile.close();
  } else {
    screen.println(F("!SD write failed!"));
    screen.println(F("!No datalogging!"));
  }
  delay(5000);
  screen.fillScreen(ST77XX_BLACK);
  screen.drawRect(0,0,240,320,0x7BE0);
  screen.drawFastHLine(0, 110, 240, 0x7BE0);
  screen.drawFastHLine(0, 220, 240, 0x7BE0);
  screen.drawFastVLine(120, 0, 220, 0x7BE0);
  screen.setCursor(20, 92);   screen.print(F("Oil_PSI"));
  screen.setCursor(135, 92);  screen.print(F("Fuel_PSI"));
  screen.setCursor(16, 202);  screen.print(F("Oil_Temp"));
  screen.setCursor(160, 202); screen.print(F("km/h"));
  screen.setCursor(102, 302); screen.print(F("AFR"));
}


// function inputs: x pos, y pos, previous sensor value, current sensor value, low value, high value, number of decimal places for output, text size)
void printValue(int x, int y, float pvalue, float value, int low, int high, int dp, int ts) {
  screen.setTextColor(ST77XX_BLACK);
  // font uses too much program space
//  screen.setFont(&Org_01);
  screen.setTextSize(ts);
  screen.setCursor(x, y);
  // only update display if previous value and current value are different.
  // Need to fix updates that occur due to decimal places.
  if (pvalue != value) {
    screen.print(pvalue, dp);
    if (value < low) {
      screen.setTextColor(ST77XX_RED);
    } else if ((value > low) and (value < high)) {
      screen.setTextColor(ST77XX_GREEN);
    } else {
      screen.setTextColor(ST77XX_ORANGE);
    }
    screen.setCursor(x, y);
    screen.print(value, dp);
  }
}


static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (gpsSerial.available())
      gps.encode(gpsSerial.read());
  } while (millis() - start < ms);
}


void loop() {
  float sVal[7];

  smartDelay(200);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));

  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    if (gps.date.isValid()) {
      if (gps.date.isUpdated()) {
        dataFile.print(gps.date.year()); dataFile.print(F("/"));
        if (gps.date.month() < 10) dataFile.print(F("0"));
        dataFile.print(gps.date.month()); dataFile.print(F("/"));
        if (gps.date.day() < 10) dataFile.print(F("0"));
        dataFile.print(gps.date.day());
      } else {
        dataFile.print(F("!Updated"));
      }
    } else {
      dataFile.print(F("INVALID"));
    }

    dataFile.print(F(","));

    if (gps.time.isValid()) {
      if (gps.time.isUpdated()) {
        if (gps.time.hour() < 10) dataFile.print(F("0"));
         dataFile.print(gps.time.hour()); dataFile.print(F(":"));
        if (gps.time.minute() < 10) dataFile.print(F("0"));
         dataFile.print(gps.time.minute()); dataFile.print(F(":"));
        if (gps.time.second() < 10) dataFile.print(F("0"));
         dataFile.print(gps.time.second()); dataFile.print(F("."));
        if (gps.time.centisecond() < 10) dataFile.print(F("0"));
         dataFile.print(gps.time.centisecond());
      } else {
        dataFile.print(F("!Updated"));
      }
    } else {
      dataFile.print(F("INVALID"));
    }
    dataFile.print(F(","));
  }

  // Set speed
  if (gps.speed.isValid()) {
    sVal[4] = (gps.speed.kmph());
  } else {
    sVal[4] = 199;
  }

  // Read Oil Pressure
  int sensor = analogRead(A0);
  float svolts = (sensor*5.0)/1023.0;
  sVal[0] = (svolts - 0.5) * 25.0;

  // Read Fuel Pressure
  sensor = analogRead(A1);
  svolts = (sensor*5.0)/1023.0;
  sVal[1] = (svolts - 0.5) * 25.0;

  // Read AFR
  sensor = analogRead(A2);
  svolts = (sensor*5.0)/1023.0;
  sVal[2] = map(sensor, 0, 1023, 7, 22);

  // Read Oil Temp
  sVal[3] = thermocouple.readCelsius();

  // display oil pressure
  printValue(4, 4, sValPrev[0], sVal[0], 20, 60, 0, 9);
  sValPrev[0] = sVal[0];

  // display fuel pressure
  printValue(121, 4, sValPrev[1], sVal[1], 20, 60, 0, 9);
  sValPrev[1] = sVal[1];

  // display AFR
  printValue(2, 230, sValPrev[2], sVal[2], 10, 16, 1, 8);
  sValPrev[2] = sVal[2];

  // display oil temperature
  printValue(4, 120, sValPrev[3], sVal[3], 80, 120, 0, 9);
  sValPrev[3] = sVal[3];


//  if (sVal[4] > 99 && sValPrev[4] > 99) {
  if (sVal[4] > 99) {
    printValue(127, 120, sValPrev[4], sVal[4], 41, 110, 0, 6);
  } else {
    printValue(127, 120, sValPrev[4], sVal[4], 41, 110, 0, 9);
  }
  sValPrev[4] = sVal[4];

//  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    for (int i = 0; i <= 4; i++) {
      dataFile.print(sVal[i]);
//      Serial.print(sVal[i]);
      if (i < 4) {
//        Serial.print(F(","));
        dataFile.print(",");
       }
    }
    dataFile.close();
//    Serial.println(F(""));
  } else {
    Serial.println(F("error opening datalog.csv"));
  }
}