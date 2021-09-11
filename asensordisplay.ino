#include <SPI.h>
#include <SD.h>
#include <DFRobot_GDL.h>
#include "max6675.h"
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

#define thermoDO  4
#define thermoCS  5
#define thermoCLK 6
#define TFT_CS    7
#define TFT_RST   8
#define TFT_SD    9
#define TFT_DC    10

DFRobot_ST7789_240x320_HW_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
SoftwareSerial gpsSerial(2,3);
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
TinyGPSPlus gps;

float sValPrev[5];

void setup() {
  Serial.begin(115200);

  gpsSerial.begin(9600);

  screen.begin();
  screen.fillScreen(COLOR_RGB565_BLACK);
  screen.setTextWrap(true);
  screen.setTextSize(2);
//    screen.setRotation(1)
  screen.setRotation(2);
  screen.setTextColor(COLOR_RGB565_BLUE);
  screen.setCursor(0, 0);
  if (!SD.begin(TFT_SD)) {
    screen.println(F("SD init failed. No datalog"));
  }
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    Serial.print(F("setup writing to datalog.csv, "));
    dataFile.println(F("Date,Time,Speed_kmph,Oil_PSI,Fuel_PSI,AFR,Oil_Temp_C"));
    dataFile.close();
  } else {
    Serial.println(F("error opening datalog.csv"));
    screen.println(F("SD write failed!"));
  }

  delay(5000);
  screen.fillScreen(COLOR_RGB565_BLACK);
  screen.setTextWrap(false);
  screen.setCursor(0, 0);
  screen.print(F("Oil_PSI"));
//  screen.setCursor(220, 0);
  screen.setCursor(120, 0);
  screen.print(F("Fuel_PSI"));
//  screen.setCursor(0, 140);
  screen.setCursor(0, 110);
  screen.print(F("Oil_Temp"));
  screen.setCursor(100, 220);
  screen.print(F("AFR"));
  screen.setCursor(120, 110);
  screen.print(F("km/h"));
}


// function inputs: x pos, y pos, previous sensor value, current sensor value, low value, high value, number of decimal places for output, text size)
void printValue(int x, int y, float pvalue, float value, int low, int high, int dp, int ts) {
  screen.setTextColor(COLOR_RGB565_BLACK);
  // font uses too much program space
//  screen.setFont(&FreeSans12pt7b);
  screen.setTextSize(ts);
  screen.setCursor(x, y);
  // only update display if previous value and current value are different.
  // Need to fix updates that occur due to decimal places.
  if (pvalue != value) {
    screen.print(pvalue, dp);
    if (value < low) {
      screen.setTextColor(COLOR_RGB565_RED);
    } else if ((value > low) and (value < high)) {
      screen.setTextColor(COLOR_RGB565_GREEN);
    } else {
      screen.setTextColor(COLOR_RGB565_ORANGE);
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
    Serial.print(F("writing to datalog.csv, "));
    if (gps.date.isValid()) {
      if (gps.date.isUpdated()) {
        dataFile.print(gps.date.year());
        dataFile.print(F("/"));
        if (gps.date.month() < 10) dataFile.print(F("0"));
        dataFile.print(gps.date.month());
        dataFile.print(F("/"));
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
         dataFile.print(gps.time.hour());
         dataFile.print(F(":"));
        if (gps.time.minute() < 10) dataFile.print(F("0"));
         dataFile.print(gps.time.minute());
         dataFile.print(F(":"));
        if (gps.time.second() < 10) dataFile.print(F("0"));
         dataFile.print(gps.time.second());
         dataFile.print(F("."));
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

//  if (gps.location.isValid()) {
//    if (gps.location.isUpdated()) {
//      sVal[5] = (gps.location.lat());
//      sVal[6] = (gps.location.lng());
//    } else {
//      sVal[5] = 888;
//      sVal[6] = 888;
//    }
//  } else {
//    sVal[5] = 999;
//    sVal[6] = 999;
//  }
//  Serial.print(sVal[5], 6);
//  Serial.print(F(","));
//  Serial.print(sVal[6], 6);
//  Serial.print(F(","));

  // Set speed
  if (gps.speed.isValid()) {
    sVal[4] = (gps.speed.kmph());
  } else {
    sVal[4] = 999;
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
  printValue(2, 20, sValPrev[0], sVal[0], 20, 60, 0, 10);
  sValPrev[0] = sVal[0];

  // display fuel pressure
  printValue(125, 20, sValPrev[1], sVal[1], 20, 60, 0, 10);
  sValPrev[1] = sVal[1];

  // display AFR
  printValue(2, 240, sValPrev[2], sVal[2], 10, 16, 1, 10);
  sValPrev[2] = sVal[2];

  // display oil temperature
  printValue(2, 130, sValPrev[3], sVal[3], 80, 120, 0, 10);
  sValPrev[3] = sVal[3];

  // display Speed in kmph
  if (sVal[4] > 99 && sValPrev[4] > 99) {
    printValue(125, 130, sValPrev[4], sVal[4], 41, 110, 0, 7);
  } else {
    printValue(125, 130, sValPrev[4], sVal[4], 41, 110, 0, 10);
  }

  sValPrev[4] = sVal[4];

//  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    Serial.println(F("final write to datalog.csv"));
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
