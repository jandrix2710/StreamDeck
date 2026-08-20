#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBHIDConsumerControl.h"

USBHIDKeyboard Keyboard;
USBHIDConsumerControl ConsumerControl;

const int potPin = 1;
const int buttonPin = 11;


// =====================================================
// POTENTIOMETER SETTINGS
// =====================================================

// Number of ADC readings to average
const int smoothingSamples = 10;

// How much genuine movement is needed for one
// volume step
const int volumeThreshold = 80;

// Current potentiometer position
int lastPotValue = 0;

// Accumulated potentiometer movement
int accumulatedMovement = 0;


// =====================================================
// BUTTON SETTINGS
// =====================================================

bool lastButtonState = HIGH;


// =====================================================
// READ SMOOTHED POTENTIOMETER
// =====================================================

int readPotentiometer() {

  long total = 0;

  for (int i = 0; i < smoothingSamples; i++) {

    total += analogRead(potPin);

    delayMicroseconds(500);
  }

  return total / smoothingSamples;
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  // Start USB HID
  Keyboard.begin();
  ConsumerControl.begin();
  USB.begin();

  delay(2000);

  // Get starting potentiometer position
  lastPotValue = readPotentiometer();

  Serial.println("Stream Deck test started");
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  // =====================================================
  // POTENTIOMETER / VOLUME
  // =====================================================

  int potValue = readPotentiometer();

  // Calculate how much the knob moved since
  // the previous reading
  int difference = potValue - lastPotValue;

  // Always update the previous position
  lastPotValue = potValue;

  // Add the movement to our accumulator
  accumulatedMovement += difference;


  // =====================================================
  // VOLUME UP
  // =====================================================

  while (accumulatedMovement >= volumeThreshold) {

    ConsumerControl.press(
      CONSUMER_CONTROL_VOLUME_INCREMENT
    );

    delay(5);

    // Remove one volume step worth of movement
    accumulatedMovement -= volumeThreshold;

    Serial.println("Volume UP");
  }


  // =====================================================
  // VOLUME DOWN
  // =====================================================

  while (accumulatedMovement <= -volumeThreshold) {

    ConsumerControl.press(
      CONSUMER_CONTROL_VOLUME_DECREMENT
    );

    delay(5);

    // Add one volume step back towards zero
    accumulatedMovement += volumeThreshold;

    Serial.println("Volume DOWN");
  }


  // =====================================================
  // BUTTON / OPEN OBS
  // =====================================================

  bool buttonState = digitalRead(buttonPin);


  // Detect button press
  if (buttonState == LOW && lastButtonState == HIGH) {

    Serial.println("Opening OBS...");


    // Open Windows Run
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press('r');

    delay(100);

    Keyboard.releaseAll();

    delay(300);


    // Type OBS executable
    Keyboard.print(
      "C:/Program Files/obs-studio/bin/64bit/obs64.exe"
    );

    delay(100);


    // Press Enter
    Keyboard.press(KEY_RETURN);
    Keyboard.release(KEY_RETURN);

    delay(500);
  }


  // Save button state
  lastButtonState = buttonState;

  delay(10);
}
