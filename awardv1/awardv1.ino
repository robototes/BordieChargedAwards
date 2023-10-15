#include <Adafruit_NeoPixel.h>

#define MOSI_LED 2
#define LED 3
#define SWITCH 4
#define NUMPIXELS 11

Adafruit_NeoPixel bordieLED(NUMPIXELS, LED, NEO_GRB + NEO_KHZ800);

uint32_t teamColor1 = bordieLED.Color(255,255,255);
uint32_t teamColor2 = bordieLED.Color(255,255,255);
uint32_t redColor = bordieLED.Color(255,0,0);

bool ledOn = true;
unsigned long buttonTriggerTime = 0;
uint8_t buttonTriggered = 0;
bool buttonPressed = false;
unsigned long buttonPressTime = 0;
bool mosiLEDPower = true;
uint8_t brightness = 255;
bool brightnessDecrease = true;

void setup() {
  bordieLED.begin();
  pinMode(MOSI_LED,OUTPUT);
  pinMode(SWITCH,INPUT);
  Serial.begin(9600);
}

void setLEDRed() {
  bordieLED.fill(redColor, 0, NUMPIXELS);
}

void setLEDTeamColor() {
  bordieLED.fill(teamColor1, 0, 5);
  bordieLED.fill(teamColor2, 5, NUMPIXELS);
}

void loop() { 
  int buttonRead = digitalRead(SWITCH);
  if (buttonRead == LOW && !buttonPressed) {
    buttonPressed = true;
    buttonPressTime = millis();
  } else if (buttonRead == LOW) {
    if (millis() - buttonPressTime >= 500) {
      if (brightness < 255 && !brightnessDecrease) {
        brightness++;
      } else if (brightness == 255) {
        brightnessDecrease = true;
      }
      if (brightness >= 1 && brightnessDecrease) {
        brightness--;
      } else if (brightness == 0) {
        brightnessDecrease = false;
      }
      delay(10);
    }
  } else if (buttonRead == HIGH && buttonPressed) {
    buttonPressed = false;
    if (millis() - buttonPressTime < 500) {
      buttonTriggered++;
      buttonTriggerTime = millis();
    }
  }

  if (millis() - buttonTriggerTime > 250) {
    if (buttonTriggered == 1) {
      ledOn = !ledOn;
    }
    if (buttonTriggered > 1) {
      mosiLEDPower = !mosiLEDPower;
    }
    buttonTriggered = 0;
  }

  if (!ledOn) {
    bordieLED.setBrightness(0);
    digitalWrite(MOSI_LED, LOW);
  } else {
    bordieLED.setBrightness(brightness);
    digitalWrite(MOSI_LED, mosiLEDPower);
  }
  bordieLED.clear();
  setLEDTeamColor();
  bordieLED.show();
}