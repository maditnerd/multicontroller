/*
   MultiController
   By Remi Sarrailh (madnerd.org)
   URL: github.com/madnerdorg/multicontroller
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <FastLED.h>

// Serial
const String USB_NAME = "multi:42012";
const int USB_BAUDRATE = 115200;
String readString;

// Wiring
const int PIN_LEDS = 6;
const int PIN_KEYPAD_SCL = 8;
const int PIN_KEYPAD_SDO = 9;
const int PIN_IRSENSOR = 11;
const int PIN_POT1 = A0;
const int PIN_POT2 = A1;

// Leds Settings
const int NUM_LEDS = 4;
CRGB leds[NUM_LEDS];
int hue[NUM_LEDS] = {0, 0, 0, 0};
int sat[NUM_LEDS] = {0, 0, 0, 0};
int val[NUM_LEDS] = {0, 0, 0, 0};

//LCD Settings
LiquidCrystal_I2C lcd(0x27, 16, 2);
String lcd_line1 = "";
String lcd_line2 = "";
bool lcd_backlight = true;

// Other Settings
int keypad_array[16];
bool serial_state = false;

//STATE
const bool s_keypad = true;
const bool s_pot = true;
const bool s_lcd = true;
const bool s_leds = true;

int REFRESH_RATE = 25;

void setup() {
  Serial.begin(115200); //Serial

  //Keypad
  pinMode(PIN_KEYPAD_SCL, OUTPUT);
  pinMode(PIN_KEYPAD_SDO, INPUT);

  //LCD
  lcd.init();
  lcd.backlight();

  //Leds
  FastLED.addLeds<NEOPIXEL, PIN_LEDS>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(0, 0, 0);
  }
   FastLED.show();
}

void loop() {
  serialManager();

  if (serial_state) {

    if (s_pot) {
      Serial.print("POT1:");
      Serial.print(map(analogRead(PIN_POT1), 0, 1024, 0, 255)); //Map Analog Read to 0/254 (standard for midi pot)
      Serial.println();
      Serial.print("POT2:");
      Serial.print(map(analogRead(PIN_POT2), 0, 1024, 0, 255)); //Map Analog Read to 0/254 (standard for midi pot)
      Serial.println();
    }

    if (s_keypad) {
      Serial.print("KEY:");
      show_keypad(); //Display keypad state as an suit of boolean 0000000000000000
      Serial.println();
    }

    if (s_lcd) {
      //Display String display on LCD
      Serial.print("LCD:");
      Serial.print(lcd_line1);
      Serial.print(lcd_line2);
      Serial.println();
      //Display Backlight state of LCD
      Serial.print("LCDB:");
      Serial.print(lcd_backlight);
      Serial.println();
    }

    if (s_leds) {
      Serial.print("LED:");
      for (int i = 0; i < NUM_LEDS; i++) {
        Serial.print(hue[i]);
        Serial.print(sat[i]);
        Serial.print(val[i]);
        if (i != (NUM_LEDS - 1)) {
          Serial.print(";");
        }
      }
      Serial.println("");
    }

    delay(REFRESH_RATE);
  }
}

//Serial
void serialManager() {

  //Char to String
  while (Serial.available()) {
    delay(3);
    if (Serial.available() > 0) {
      char c = Serial.read();
      readString += c;
    }
  }

  // Commands
  if (readString.length() > 0) {

    //Serial
    if (readString == "/info") {
      Serial.println(USB_NAME);
      serial_state = true;
    }

    //LCD
    if (readString.substring(0, 4) == "LCD:") {
      write_lcd();
    }

    if (readString == "LCDB:ON") {
      lcd.backlight();
      lcd_backlight = true;
    }

    if (readString == "LCDB:OFF") {
      lcd.noBacklight();
      lcd_backlight = false;
    }

    //Leds
    if (readString.substring(0, 4) == "LED:") {
      String led_command = readString.substring(4);

      for (int i = 0; i < NUM_LEDS; i++) {
        int add = 3 * i;
        int h = splitString(led_command, ';', 0 + add).toInt();
        int s = splitString(led_command, ';', 1 + add).toInt();
        int v = splitString(led_command, ';', 2 + add).toInt();
        hue[i] = h;
        sat[i] = s;
        val[i] = v;
        leds[i] = CHSV(h, s, v);
      }


      FastLED.show();
    }

    readString = "";
  }
}

void write_lcd() {
  lcd.clear();
  int stringSize = readString.length();
  lcd.setCursor(0, 0);
  if (stringSize > 16) {
    lcd_line1 = readString.substring(4, 20);
    lcd_line2 = readString.substring(20, stringSize);
    lcd.print(lcd_line1);
    lcd.setCursor(0, 1);
    lcd.print(lcd_line2);
  } else {
    lcd_line1 = readString.substring(4, stringSize);
    lcd_line2 = "";
    lcd.print(lcd_line1);
  }
}

//Keypad
void show_keypad() {
  byte i;
  for (i = 1; i <= 16; i++) {
    digitalWrite(PIN_KEYPAD_SCL, LOW);
    Serial.print(!digitalRead(PIN_KEYPAD_SDO));
    digitalWrite(PIN_KEYPAD_SCL, HIGH);
  }
}

String splitString(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}



