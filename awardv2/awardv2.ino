#include <Adafruit_NeoPixel.h>

// AUTHOR: Jonah Kowal, 10/15/2023
// This code is so messy and unreadable. I'm sorry

#define FRONT_LED 28 // these are the lights inside the frame of the award that shine out the front
#define LED 3 // this is the led strip below the acryllic
#define WHITE_LED 29 // these are the bright white leds under the acryllic
#define SWITCH 4 // this is the button to the right of the usb-c
#define MODE_SWITCH 26 // this is the button to the left of the usb-c
#define NUMPIXELS 8

#define PULSE_SPEED 0.0025

Adafruit_NeoPixel bordieLED(NUMPIXELS, LED, NEO_GRB + NEO_KHZ800);

uint32_t teamColor1 = bordieLED.Color(0,255,0);
uint32_t teamColor2 = bordieLED.Color(185,66,245);
uint32_t redColor = bordieLED.Color(255,0,0);
uint32_t whiteColor = bordieLED.Color(255, 255, 255);

bool ledOn = true;
unsigned long buttonTriggerTime = 0;
uint8_t buttonTriggered = 0;
bool buttonPressed = false;
unsigned long buttonPressTime = 0;
bool mosiLEDPower = true;
double brightness = 255.0;
bool brightnessDecrease = true;

int8_t ledMode = -1;
// 0 = static team colors
// 1 = pulsing team colors
// 2 = rainbow
// 3 = robototes colors (red)
// 4 = bright led on
bool auxButtonTriggered = false;

void setup() {
  bordieLED.begin();
  pinMode(FRONT_LED,OUTPUT);
  pinMode(SWITCH,INPUT);
  pinMode(MODE_SWITCH, INPUT);
  pinMode(WHITE_LED, OUTPUT);
  Serial.begin(9600);
}

void setLEDRed() {
  bordieLED.fill(redColor, 0, 5);
  bordieLED.fill(whiteColor, 5, NUMPIXELS);
}

void setLEDTeamColor() {
  bordieLED.fill(teamColor1, 0, 5);
  bordieLED.fill(teamColor2, 5, NUMPIXELS);
}

void pulseLEDTeamColor() {
  bordieLED.fill(teamColor1, 0, 5);
  bordieLED.fill(teamColor2, 5, NUMPIXELS);

  double pulsation = sin(millis()*PULSE_SPEED) * 0.08;
  if ((brightness + pulsation) < 255 && (brightness + pulsation) > 0) {
    brightness += pulsation;
  }
}

void setWhiteLED() {
  digitalWrite(WHITE_LED, HIGH);
}

uint32_t hsl(uint16_t ih, uint8_t is, uint8_t il) {

  float h, s, l, t1, t2, tr, tg, tb;
  uint8_t r, g, b;

  h = (ih % 360) / 360.0;
  s = constrain(is, 0, 100) / 100.0;
  l = constrain(il, 0, 100) / 100.0;

  if ( s == 0 ) { 
    r = g = b = 255 * l;
    return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
  } 
  
  if ( l < 0.5 ) t1 = l * (1.0 + s);
  else t1 = l + s - l * s;
  
  t2 = 2 * l - t1;
  tr = h + 1/3.0;
  tg = h;
  tb = h - 1/3.0;

  r = hsl_convert(tr, t1, t2);
  g = hsl_convert(tg, t1, t2);
  b = hsl_convert(tb, t1, t2);

  // NeoPixel packed RGB color
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

uint8_t hsl_convert(float c, float t1, float t2) {

  if ( c < 0 ) c+=1; 
  else if ( c > 1 ) c-=1;

  if ( 6 * c < 1 ) c = t2 + ( t1 - t2 ) * 6 * c;
  else if ( 2 * c < 1 ) c = t1;
  else if ( 3 * c < 2 ) c = t2 + ( t1 - t2 ) * ( 2/3.0 - c ) * 6;
  else c = t2;
  
  return (uint8_t)(c*255); 
}

void rainbow() {
  bordieLED.fill(hsl(360.0 * abs(sin(millis()*PULSE_SPEED*0.1)), 100, 50), 0, NUMPIXELS);
}

void loop() { 
  int buttonRead = digitalRead(SWITCH);

  // register button states, enter brightness modifcation loop if warranted
  if (buttonRead == LOW && !buttonPressed) {
    buttonPressed = true;
    buttonPressTime = millis();
  } else if (buttonRead == LOW) {
    if (millis() - buttonPressTime >= 500) {
      if (brightness < 255 && !brightnessDecrease) {
        brightness++;
      } else if (brightness >= 255) {
        brightnessDecrease = true;
      }
      if (brightness > 0 && brightnessDecrease) {
        brightness--;
      } else if (brightness <= 0) {
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

  // check button state for either auxilery led off or all led off
  if (millis() - buttonTriggerTime > 250) {
    if (buttonTriggered == 1) {
      ledOn = !ledOn;
    }
    if (buttonTriggered > 1) {
      mosiLEDPower = !mosiLEDPower;
    }
    buttonTriggered = 0;
  }

  // turn led on or off
  if (!ledOn) {
    bordieLED.setBrightness(0);
    digitalWrite(FRONT_LED, LOW);
    digitalWrite(WHITE_LED, LOW);
  } else {
    bordieLED.setBrightness((uint8_t) brightness);
    digitalWrite(FRONT_LED, mosiLEDPower);
  }

  bool auxButtonRead = digitalRead(MODE_SWITCH);
  // increment led mode if aux button is down
  if (auxButtonRead == HIGH && !auxButtonTriggered) {
    auxButtonTriggered = true;
    if (ledMode < 4) {
      ledMode++;
    } else {
      ledMode = 0;
    }
  } else if (auxButtonRead == LOW && auxButtonTriggered) {
    auxButtonTriggered = false;
  }

  if (ledMode != 4) {
    digitalWrite(WHITE_LED, LOW);
  }

  bordieLED.clear();
  switch(ledMode) {
    case 0: // static team colors
      setLEDTeamColor();
      break;
    case 1: // pulsing team colors
      pulseLEDTeamColor();
      break;
    case 2: // rainbow
      rainbow();
      break;
    case 3: // red
      setLEDRed();
      break;
    case 4: // bright white led
      setWhiteLED();
      break;
    default:
      Serial.println("Something went wrong. Oops.");
  }
  bordieLED.show(); 
}