// ********* basic.ino **********/
// Interfacing template for our Arduino circuit
// By Max Konzerowsky
// July 2024


/********** variables **********/
// Pins
const int potPin = A0;              // Define the analog pin for the potentiometer
const int fetGatePin = 3;           // Define the pin for controlling the FET gate
const int internalLedPin = 13;      // Define the pin for the internal LED on the board

// Potentiometer value
int potValue = 0;

// HR & BPM
unsigned long hrBeatTime = 0;       // Starting time of current heartbeat (ms)


/********** setup **********/
void setup() {
  pinMode(fetGatePin, OUTPUT);      // Initialize the FET gate pin as an output
  pinMode(internalLedPin, OUTPUT);  // Initialize the internal LED pin as an output
  digitalWrite(fetGatePin, LOW);    // Set initial output to 0V for haptic motor
  digitalWrite(internalLedPin, LOW);// Set initial output to LOW for the internal LED

  // Initialize the serial communication for debugging
  Serial.begin(9600);
}


/********** main loop **********/
void loop() {
  // Read the potentiometer value (0-1023)
  potValue = analogRead(potPin);

  // Map the potentiometer value to BPM range (55-85)
  int bpm = map(potValue, 0, 1023, 55, 85);

  // Print the BPM value to the serial monitor for debugging
  Serial.print("Potentiometer Value: ");
  Serial.print(potValue);
  Serial.print("\t BPM: ");
  Serial.println(bpm);

  // Helper function execution
  pulse(bpm);
}


// Pulse functionality
void pulse(int bpm) {
  static unsigned long hrBeatTime = 0;      // timestamp at beginning of heartbeat
  static unsigned long currentMillis = 0;   // current timestamp in milliseconds
  static long hrLength = 0;                 // heartbeat duration given bpm
  static long elapsedTime = 0;              // elapsed time of current heartbeat
  static bool pwmState = LOW;               // track the state of fetGatePin

  currentMillis = millis();                 // Call millis() only once

  // Is a new beat starting
  if (hrBeatTime == 0) {
    hrBeatTime = currentMillis;             // Update heartbeat start time to current
    pwmState = HIGH;
    digitalWrite(fetGatePin, pwmState);     // Turn haptic motor on
    digitalWrite(internalLedPin, HIGH);     // Turn internal LED on
    return;
  }

  hrLength = 60000 / bpm;                    // heartbeat length based on the BPM
  elapsedTime = currentMillis - hrBeatTime;  // time elapsed since heartbeat began

  // Is the current heartbeat still elapsing
  if (elapsedTime < hrLength) {
    // Is the heartbeat on the first pulse or second pulse
    if ((elapsedTime < 41) || ((140 < elapsedTime) && (elapsedTime < 181))) {
      if (!pwmState) {
        pwmState = HIGH;
        digitalWrite(fetGatePin, pwmState);
        digitalWrite(internalLedPin, HIGH); // Turn internal LED on
      }
    } else {
      if (pwmState) {
        pwmState = LOW;
        digitalWrite(fetGatePin, pwmState);
        digitalWrite(internalLedPin, LOW);  // Turn internal LED off
      }
    }
  } else {
    if (pwmState) {
      pwmState = LOW;
      digitalWrite(fetGatePin, pwmState);   // Turn off the motor
      digitalWrite(internalLedPin, LOW);    // Turn internal LED off
    }
    hrBeatTime = 0;                         // Indicate next beat is ready to start
  }
}

