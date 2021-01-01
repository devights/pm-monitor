#include "Adafruit_PM25AQI.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4);
SoftwareSerial pmSerial(2, 3);
String fileName;

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

void setup() {
  // Wait for serial monitor to open
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Adafruit PMSA003I Air Quality Sensor");

  // Wait one second for sensor to boot up!
  delay(1000);

  pmSerial.begin(9600);

  if (! aqi.begin_UART(&pmSerial)) { // connect to the sensor over software serial
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  Serial.println("PM25 found!");

  setFile();
  Serial.println("File set");
  lcd.init();                      // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("DustWatcher");
  lcd.setCursor(0, 1);
  lcd.print("            v0.1");
}

void loop() {
  unsigned long lastWrite = 0;
  unsigned long lastWriteLCD = 0;
  unsigned long runTime;
  unsigned int pm10;
  unsigned int pm25;
  unsigned int pm100;
  PM25_AQI_Data data;
  runTime = millis();

  if (! aqi.read(&data)) {
    //    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }
  //  Serial.println("AQI reading success");
  pm10 = data.pm10_standard;
  pm25 = data.pm25_standard;
  pm100 = data.pm100_standard;

  if (runTime - lastWrite >= 10000) {
    Serial.println(runTime);
    Serial.println(lastWrite);
    writeDataFile(runTime, pm10, pm25, pm100);

    lastWrite = runTime;
  }
  if (runTime - lastWriteLCD >= 10000) {
    writeDataLCD(pm10, pm25, pm100);
    lastWriteLCD = runTime;
  }

}
void writeDataLCD(int pm10, int pm25, int pm100) {
  char mu = 228;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1");
  lcd.print(mu);
  lcd.print(":" + String(pm10));
  lcd.setCursor(9, 0);
  lcd.print("| 10");
  lcd.print(mu);
  lcd.print(":");
  lcd.setCursor(0, 1);
  lcd.print("2.5");
  lcd.print(mu);
  lcd.print(":" + String(pm25));
  lcd.setCursor(9, 1);
  lcd.print("| " + String(pm100));
}

void writeDataFile(int sec, int pm10, int pm25, int pm100) {
  Serial.println("storing results");
  File outFile =  SD.open(fileName, FILE_WRITE);

  outFile.print(sec / 1000);
  outFile.print(", ");
  outFile.print(pm10);
  outFile.print(", ");
  outFile.print(pm25);
  outFile.print(", ");
  outFile.print(pm100);
  outFile.println();

  outFile.close();
}

String getFilename(int num) {
  String name = "run_";
  name.concat(num);
  name.concat(".csv");
  return name;
}

void setFile() {
  int fileNum = 0;
  bool isFileSet = false;

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }
  while (!isFileSet) {
    isFileSet = !SD.exists(getFilename(fileNum));
    if (!isFileSet) {
      fileNum++;
    }
  }
  fileName = getFilename(fileNum);
}
