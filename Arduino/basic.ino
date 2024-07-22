// ********* basic.ino **********/
// Interfacing template for our Arduino circuit
// By Max Konzerowsky
// July 2024
//
// Basic UI: Input from `potPin` outputted to `ledPin` voltage
// Basic HR: 250mv outputted to `pwmPin` every `interval` milliseconds

/********** variables **********/
// pins
const int potPin = A0;              // Define the analog pin for the potentiometer
const int ledPin = 9;               // Define the digital pin for the LED
const int pwmPin = 3;               // Define the pin for pulse width modulation for heartbeat

// potentiometer and LED values
int potValue = 0;
int ledValue = 0;

// HR & BPM
unsigned long previousMillis = 0;   // Will store last time the output was updated

// pulse
const int operatingVoltage = 5000;  // Operating voltage in millivolts
bool pulseActive = false;

/********** setup **********/
void setup() {
  pinMode(ledPin, OUTPUT);    // Initialize the LED pin as an output
  pinMode(pwmPin, OUTPUT);    // Initialize the PWM pin as an output
  analogWrite(pwmPin, 0);     // Set initial output to 0V for haptic motor

  // Initialize the serial communication for debugging
  Serial.begin(9600);
}

/********** main loop **********/
void loop() {
  // Pulse hardcoded for testing (75 bpm, 20ms, 250mV)
  int bpm = 75;                               // Beats per minute
  int targetMillivolts = 250;                 // Output voltage in millivolts
  long pulseDuration = 20;                    // Duration of the pulse in milliseconds

  // Helper function execution
  pulse(targetMillivolts, pulseDuration, bpm);
  nonHRInterface();

  // Add a small delay for stability
  delay(10);
}

/********** helper functions **********/
// Basic UI for potentiometer and LED input
void nonHRInterface() {
  potValue = analogRead(potPin);              // Read the potentiometer value (0-1023)
  ledValue = map(potValue, 0, 1023, 0, 255);  // Map the potentiometer value to a PWM value (0-255)
  analogWrite(ledPin, ledValue);              // Output the PWM value to the LED
  
  // Print the potentiometer and LED values to the serial monitor
  Serial.print("Potentiometer Value: ");
  Serial.print(potValue);
  Serial.print("\t LED Value: ");
  Serial.println(ledValue);
}

// Pulse functionality
void pulse(int targetMillivolts, long pulseDuration, int bpm) {
  // Calculate the interval based on the BPM
  long calculatedInterval = 60000 / bpm;

  // Get the current time
  unsigned long currentMillis = millis();

  // Check if the calculated interval has passed and the pulse is not active
  if ((currentMillis - previousMillis >= calculatedInterval) && !pulseActive) {
    // Save the last time the pulse started
    previousMillis = currentMillis;

    // Calculate the duty cycle for the target millivolts
    int dutyCycle = calculateDutyCycle(targetMillivolts, operatingVoltage);

    // Start the pulse by sending the target millivolts output
    analogWrite(pwmPin, dutyCycle);
    pulseActive = true;
  }

  // Check if the pulse should be turned off
  if (pulseActive && (currentMillis - previousMillis >= pulseDuration)) {
    // Turn off the pulse
    analogWrite(pwmPin, 0);
    pulseActive = false;
  }
}

// Return duty cycle inferred from specified millivolts
int calculateDutyCycle(int millivolts, int operatingVoltage = 5000) {
  // Ensure the millivolts value is within the range
  if (millivolts < 0) {
    millivolts = 0;
  } else if (millivolts > operatingVoltage) {
    millivolts = operatingVoltage;
  }

  // Calculate the duty cycle
  int dutyCycle = (millivolts * 255) / operatingVoltage;
  return dutyCycle;
}
