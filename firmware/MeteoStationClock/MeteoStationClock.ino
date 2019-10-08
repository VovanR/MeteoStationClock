/**
 * See: https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
 * See: https://alexgyver.ru/meteoclock/
 * See: https://www.arduino.cc/reference/en/#variables
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_CONTRAST 10 // From 0 to 255 (sets contrast e.g. brightness)

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

// Button
#define BTN_PIN 4

// Variables
float dispTemp;
byte dispHum;
int dispPres;
int dispRain;

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(SCREEN_CONTRAST);

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(100); // Pause for 2 seconds
//
//  // Clear the buffer
//  display.clearDisplay();
//
//  display.setTextSize(1);
//  display.setTextColor(WHITE);
//  display.cp437(true);
//  display.setCursor(0, 0);
//  display.println(F("22:24 04 Wed Sep 2019"));
//  display.println(F("Temp +14 C"));
//  display.println(F("Hum   60 %"));
//  display.println(F("CO2  410 ppm"));
//  display.println(F("Pres 758 mm"));
//  display.println(F("Rain  50 %"));
//  display.display();


  Serial.print(F("BME280... "));
  delay(50);
  if (bme.begin(BME280_ADDRESS, &Wire) || bme.begin(BME280_ADDRESS_ALTERNATE, &Wire)) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("ERROR"));
  }

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF);

  bme.takeForcedMeasurement();
  uint32_t Pressure = bme.readPressure();

  // Button
  pinMode(BTN_PIN, INPUT_PULLUP);
}

// Button
boolean buttFlag = 0;
boolean butt;
unsigned long lastPress;

// Render
unsigned long lastRender;
int renderType = 1;

void loop() {
  butt = !digitalRead(BTN_PIN);

  if (butt == 1 && buttFlag == 0 && millis() - lastPress > 100) {
    buttFlag = 1;
    Serial.println("Button pressed");
    lastPress = millis();
  }

  if (butt == 0 && buttFlag == 1) {
    buttFlag = 0;
    
    renderType++;
    if (renderType > 2) {
      renderType = 0;
    }

    if (renderType == 2) {
      display.ssd1306_command(SSD1306_DISPLAYOFF);
    } else {
      display.ssd1306_command(SSD1306_DISPLAYON);
    }
    
    Serial.println("Button released");
    Serial.println(renderType);
  }

//  if (millis() - lastPress > 2000) {
//    display.ssd1306_command(SSD1306_DISPLAYOFF); // Switch display off
//  } else {
//    display.ssd1306_command(SSD1306_DISPLAYON); // Switch display back on
//  }

  if (millis() - lastRender > 1000) {
    lastRender = millis();

    readSensors();
    
    if (renderType == 0) {
      drawSensors();
    } else if (renderType == 1) {
      drawSensors2();
    }
  }
}


void readSensors() {
  bme.takeForcedMeasurement();
  dispTemp = bme.readTemperature();
  
  dispHum = bme.readHumidity();
  dispPres = (float)bme.readPressure() * 0.00750062;
}

void drawSensors() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.cp437(true);
  display.setCursor(0, 0);
  display.println("22:24 04 Wed Sep 2019");
  display.setTextSize(2);
  if (dispTemp > 0) {
    display.println("Temp +" + String(dispTemp, 0) + " C");
  } else {
    display.println("Temp " + String(dispTemp, 0) + " C");
  }
  display.setTextSize(1);
  display.println("Hum   " + String(dispHum) + " %");
  display.println("CO2  410 ppm");
  display.println("Pres " + String(dispPres) + " mm");
  display.println("Rain  " + String(dispRain) + " %");
  
  display.display();
}

String drawNum(int num) {
  String stringNum = String(num);
  String n = stringNum;
  if (num < 10) {
    n = "  " + stringNum;
  } else if (num < 100) {
    n = " " + stringNum;
  }
  return n;
}

void drawSensors2() {
  display.clearDisplay();
  
  display.setTextColor(WHITE);
  display.cp437(true);
  
  display.setTextSize(2);
  display.setCursor(0, 0);
  
  if (dispTemp > 0) {
    display.println("+" + String(dispTemp, 0));
  } else {
    display.println(String(dispTemp, 0));
  }
  display.println(drawNum(dispHum));
  display.println("  -");
  display.println(drawNum(dispPres));
  
  display.setCursor(60, 48);
  display.println(drawNum(dispRain));

  display.setTextSize(1);
  display.setCursor(36, 0);
  display.println("Temp");
  display.setCursor(36, 8);
  display.println("C");
  display.setCursor(36, 16);
  display.println("Hum");
  display.setCursor(36, 24);
  display.println("%");
  display.setCursor(36, 32);
  display.println("CO2");
  display.setCursor(36, 40);
  display.println("ppm");
  display.setCursor(36, 48);
  display.println("Pres");
  display.setCursor(36, 56);
  display.println("mm");
  
  display.setCursor(96, 48);
  display.println("Rain");
  display.setCursor(96, 56);
  display.println("%");

  // Clock
  display.setTextSize(2);
  display.setCursor(72, 0);
  display.println("22");
  display.setCursor(92, 0);
  display.println(":");
  display.setCursor(102, 0);
  display.println("24");

  // Date
  display.setTextSize(1);
  display.setCursor(72, 16);
  display.println("04");
  display.setCursor(90, 16);
  display.println("Wed");
  display.setCursor(72, 24);
  display.println("Sep");
  display.setCursor(96, 24);
  display.println("2019");
  
  display.display();
}
