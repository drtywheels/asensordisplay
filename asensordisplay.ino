#include <SPI.h>
#include <SD.h>
#include <DFRobot_GDL.h>

/*M0*/
#define TFT_DC  10
#define TFT_CS  4
#define TFT_RST 5
#define TFT_SD  9

DFRobot_ST7789_240x320_HW_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);

float sValPrev[]={0, 0, 0, 0};

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  screen.begin();
  screen.fillScreen(COLOR_RGB565_BLACK);
  screen.setTextWrap(true);
  screen.setTextSize(2);
  screen.setRotation(1);
  screen.setTextColor(COLOR_RGB565_BLUE);
  screen.setCursor(0, 0);
  screen.println("\nInitializing SD card...");
  if (!SD.begin(TFT_SD)) {
    screen.println("SD initialization failed!");
    screen.println("continuing without datalogging");
  }
  delay(2000);
  screen.fillScreen(COLOR_RGB565_BLACK);
  screen.setTextWrap(false);
  screen.setCursor(0, 0);
  screen.print("Oil_PSI");
  screen.setCursor(170, 0);
  screen.print("Fuel_PSI");
  screen.setCursor(0, 120);
  screen.print("Oil_Temp");
  screen.setCursor(170, 120);
  screen.print("AFR");
}

void loop(){
  String dataString = "";
  for (int analogPin = 0; analogPin < 4; analogPin++) {
    int sensor = analogRead(analogPin);
    float svolts = (sensor*5.0)/1023.0;
    if (analogPin == 0) {
      float sdata = ((float)svolts - 0.5) * 25.0;
      dataString += String(sdata);
      printSPrev(10, 95, sValPrev[0]);
      printSValue(10, 95, sdata);
      sValPrev[0] = sdata;
    } else if (analogPin == 1) {
      float sdata = ((float)svolts - 0.5) * 25.0;
      dataString += String(sdata);
      printSPrev(180, 95, sValPrev[1]);
      printSValue(180, 95, sdata);
      sValPrev[1] = sdata;
    } else if (analogPin == 2) {
      dataString += String(svolts);
      printSPrev(10, 215, sValPrev[2]);
      printSValue(10, 215, svolts);
      sValPrev[2] = svolts;
    } else if (analogPin == 3) {
      dataString += String(svolts);
      printSPrev(180, 215, sValPrev[3]);
      printSValue(180, 215, svolts);
      sValPrev[3] = svolts;
    }
    if (analogPin < 3) {
      dataString += ",";
    }
  }
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
  }
  else {
    Serial.println("error opening datalog.txt");
  }
  delay(1000);
}

void printSPrev(int x, int y, float sdata) {
  screen.setTextColor(COLOR_RGB565_BLACK);
  screen.setFont(&FreeSans12pt7b);
  screen.setTextSize(4);
  screen.setCursor(x, y);
  screen.print(sdata, 0);
}

void printSValue(int x, int y, float sdata) {
  screen.setFont(&FreeSans12pt7b);
  if (sdata < 20) {
    screen.setTextColor(COLOR_RGB565_RED);
  } else if ((sdata > 20) and (sdata < 60)) {
    screen.setTextColor(COLOR_RGB565_GREEN);
  } else {
    screen.setTextColor(COLOR_RGB565_ORANGE);
  }
  screen.setTextSize(4);
  screen.setCursor(x, y);
  screen.print(sdata, 0);
}
