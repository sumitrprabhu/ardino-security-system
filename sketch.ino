
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo doorServo;

// Existing pin connections
const int pinButton = 2;
const int enterButton = 3;
const int clearButton = 4;

const int servoPin = 6;
const int redLED = 8;
const int greenLED = 9;
const int buzzer = 10;

String enteredPIN = "";
const String correctPIN = "1234";

bool unlocked = false;
unsigned long unlockTime = 0;

int currentDigit = 0;

void showMessage(String line1, String line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 15);
  display.println(line1);
  display.setCursor(0, 35);
  display.println(line2);
  display.display();
}

void allLEDsOff() {
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);
}

void setup() {
  pinMode(pinButton, INPUT_PULLUP);
  pinMode(enterButton, INPUT_PULLUP);
  pinMode(clearButton, INPUT_PULLUP);

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  allLEDsOff();

  doorServo.attach(servoPin);
  doorServo.write(0);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // If the OLED fails, blink LEDs to indicate an error.
    while (true) {
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);
      delay(300);
      digitalWrite(redLED, LOW);
      delay(300);
    }
  }

  // Startup LED test: both LEDs should glow for 3 seconds.
  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, HIGH);
  delay(3000);
  allLEDsOff();

  showMessage("SMART DOOR LOCK", "PIN: 1234");
  delay(1500);
  showMessage("Enter PIN", "Press PIN button");
}

void loop() {
  // Automatically lock the door after 5 seconds.
  if (unlocked && millis() - unlockTime >= 5000) {
    doorServo.write(0);
    digitalWrite(greenLED, LOW);
    unlocked = false;
    enteredPIN = "";
    currentDigit = 0;
    showMessage("DOOR LOCKED", "Enter PIN");
  }

  if (unlocked) {
    return;
  }

  // Button 1: cycle through digits 0-9.
  if (digitalRead(pinButton) == LOW) {
    currentDigit = (currentDigit + 1) % 10;

    if (enteredPIN.length() < 4) {
      enteredPIN += String(currentDigit);
    } else {
      enteredPIN = String(currentDigit);
    }

    showMessage("PIN: " + enteredPIN, "Press ENTER");
    delay(250);
    while (digitalRead(pinButton) == LOW) {
      delay(10);
    }
  }

  // Button 2: check the entered PIN.
  if (digitalRead(enterButton) == LOW) {
    if (enteredPIN == correctPIN) {
      allLEDsOff();
      digitalWrite(greenLED, HIGH);
      tone(buzzer, 1000, 200);
      doorServo.write(90);

      unlocked = true;
      unlockTime = millis();

      showMessage("ACCESS GRANTED", "Door Unlocked");
    } else {
      allLEDsOff();
      digitalWrite(redLED, HIGH);
      tone(buzzer, 400, 800);

      showMessage("ACCESS DENIED", "Wrong PIN!");
      delay(1000);

      digitalWrite(redLED, LOW);
      enteredPIN = "";
      currentDigit = 0;
      showMessage("PIN CLEARED", "Try Again");
    }

    delay(250);
    while (digitalRead(enterButton) == LOW) {
      delay(10);
    }
  }

  // Button 3: clear the entered PIN.
  if (digitalRead(clearButton) == LOW) {
    enteredPIN = "";
    currentDigit = 0;
    allLEDsOff();

    showMessage("PIN CLEARED", "Enter PIN");
    delay(250);

    while (digitalRead(clearButton) == LOW) {
      delay(10);
    }
  }
}
