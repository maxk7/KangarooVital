// ********* basic.ino **********/
// Interfacing template for our Arduino circuit
// By Max Konzerowsky
// July 2024
//
// Basic UI: Input from `potPin` outputted to `ledPin` voltage
// Basic HR: 250mv outputted to `pwmPin` every `interval` milliseconds


/********** variables **********/
// Pins
const int potPin = A0;              // Define the analog pin for the potentiometer
const int ledPin = 9;               // Define the digital pin for the LED
const int pwmPin = 3;               // Define the pin for pulse width modulation for heartbeat

// Potentiometer and LED values
int potValue = 0;
int ledValue = 0;

// HR & BPM
unsigned long hrBeatTime = 0;       // Starting time of current heartbeat (ms)


/********** setup **********/
void setup() {
  pinMode(ledPin, OUTPUT);    // Initialize the LED pin as an output
  pinMode(pwmPin, OUTPUT);    // Initialize the PWM pin as an output
  digitalWrite(pwmPin, LOW);  // Set initial output to 0V for haptic motor

  // Initialize the serial communication for debugging
  Serial.begin(9600);
}


/********** main loop **********/
void loop() {
  // BPM hardcoded for testing
  int bpm = 65;

  // Helper function execution
  pulse(bpm);
  nonHRInterface();
}


/********** helper functions **********/
// Basic UI for potentiometer and LED input
void nonHRInterface() {
  static unsigned long lastPrintTime = 0;
  const unsigned long printInterval = 500;    // Print every 500ms

  potValue = analogRead(potPin);              // Read the potentiometer value (0-1023)
  ledValue = (potValue * 255) / 1023;         // Map the potentiometer value to a PWM value (0-255)
  analogWrite(ledPin, ledValue);              // Output the PWM value to the LED

  unsigned long currentMillis = millis();
  if (currentMillis - lastPrintTime >= printInterval) {
    lastPrintTime = currentMillis;
    // Print the potentiometer and LED values to the serial monitor
    Serial.print("Potentiometer Value: ");
    Serial.print(potValue);
    Serial.print("\t LED Value: ");
    Serial.println(ledValue);
  }
}


// Pulse functionality
void pulse(int bpm) {
  static unsigned long hrBeatTime = 0;      // timestamp at beginning of heartbeat
  static unsigned long currentMillis = 0;   // current timestamp in milliseconds
  static long hrLength = 0;                 // heartbeat duration given bpm
  static long elapsedTime = 0;              // elapsed time of current heartbeat
  static bool pwmState = LOW;               // track the state of pwmPin

  currentMillis = millis();                 // Call millis() only once

  // Is a new beat is starting
  if (hrBeatTime == 0) {
    hrBeatTime = currentMillis;             // Update heartbeat start time to current
    pwmState = HIGH;
    digitalWrite(pwmPin, pwmState);         // Turn haptic motor on and continue
    return;
  }

  hrLength = 60000 / bpm;                    // heartbeat length based on the BPM
  elapsedTime = currentMillis - hrBeatTime;  // time elapsed since heartbeat began

  // Is the current heartbeat still elapsing
  if (elapsedTime < hrLength) {
    // Is the heartbeat on the first pulse or second pulse
    if ((elapsedTime < 5) || ((12 < elapsedTime) && (elapsedTime < 19))) {
      if (!pwmState) {
        pwmState = HIGH;
        digitalWrite(pwmPin, pwmState);
      }
    } else {
      if (pwmState) {
        pwmState = LOW;
        digitalWrite(pwmPin, pwmState);
      }
    }
  } else {
    if (pwmState) {
      pwmState = LOW;
      digitalWrite(pwmPin, pwmState);       // Turn off the motor
    }
    hrBeatTime = 0;                         // Indicate next beat is ready to start
  }
}

