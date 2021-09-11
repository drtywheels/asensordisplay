#include <DFRobot_GDL.h>
/*M0*/
#define TFT_DC  10
#define TFT_CS  4
#define TFT_RST 5

DFRobot_ST7789_240x320_HW_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);

int sensorValuePrev[]={0, 0, 0, 0};

void setup() {
  Serial.begin(115200);
  screen.begin();
  screen.fillScreen(COLOR_RGB565_BLACK);
  screen.setTextWrap(false);
  screen.setTextSize(2);
  screen.setRotation(1);
  screen.setTextColor(COLOR_RGB565_BLUE);
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
    printSensorPrev(10, 95, sensorValuePrev[0]);
    printSensorValue(10, 95, A0);
    printSensorPrev(180, 95, sensorValuePrev[1]);
    printSensorValue(180, 95, A1);
    printSensorPrev(10, 215, sensorValuePrev[2]);
    printSensorValue(10, 215, A2);
    printSensorPrev(180, 215, sensorValuePrev[3]);
    printSensorValue(180, 215, A3);
    delay(500);
}

void printSensorPrev(int x, int y, int sValue) {
  float voltage = (sValue*5.0)/1024.0;
  float pressure_psi = ((float)voltage - 0.5) * 25.0;
  screen.setTextColor(COLOR_RGB565_BLACK);
  screen.setFont(&FreeSans12pt7b);
  screen.setTextSize(4);
  screen.setCursor(x, y);
  screen.print(pressure_psi, 0);
}

void printSensorValue(int x, int y, int AX) {  
  int sensorValue = analogRead(AX);
  if (AX == A0) {
    sensorValuePrev[0] = sensorValue;
  } else if (AX == A1) {
    sensorValuePrev[1] = sensorValue;
  } else if (AX == A2) {
    sensorValuePrev[2] = sensorValue;
  } else if (AX == A3) {
    sensorValuePrev[3] = sensorValue;
  }
  float voltage = (sensorValue*5.0)/1024.0;
  float pressure_psi = ((float)voltage - 0.5) * 25.0;
  screen.setFont(&FreeSans12pt7b);
  if (pressure_psi < 20) {
    screen.setTextColor(COLOR_RGB565_RED);
  } else if ((pressure_psi > 20) and (voltage < 60)) {
    screen.setTextColor(COLOR_RGB565_GREEN);
  } else {
    screen.setTextColor(COLOR_RGB565_ORANGE);
  }
  screen.setTextSize(4);
  screen.setCursor(x, y);
  screen.print(pressure_psi, 0);
}