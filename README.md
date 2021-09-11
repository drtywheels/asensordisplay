# asensordisplay

An arduino nano program to reading in analog sensors and display on a small 320x240 spi screen.

screen details can be found [here](https://wiki.dfrobot.com/2.0_Inches_320_240_IPS_TFT_LCD_Display_with_MicroSD_Card_Breakout_SKU_DFR0664)

screen library can be found [here](https://github.com/DFRobot/DFRobot_GDL)


## asensordisplay.ino v0.1

basic program to display 4 sensors on a 2" display
Limited functionality and not entirely working.

## asensordisplay.ino v0.2

added functionality to write data to SD card if it's inserted and functional.
added more functionality to calculate values for specific sensors

## asensordisplay.ino v0.3

added display for oil temp based on max6775 thermocouple. Good info available [here](https://electropeak.com/learn/interfacing-max6675-k-type-thermocouple-module-with-arduino/)
added gps date/time and speed logging.
displaying speed. on screen
rotated display to fit the sensor values a little more cleanly.
can not add much more functionality till more optimisation is complete. nano barely has the program space to handle the amount of code required to do it properly.
currently it's at 99% program memory & 75% dynamic memory.