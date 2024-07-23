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

// Pulse
const int operatingVoltage = 5000;  // Operating voltage in millivolts


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
  // BPM hardcoded for testing
  int bpm = 75;

  // Helper function execution
  pulse(bpm);
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
void pulse(int bpm) {
  unsigned long currentMillis = millis();

  // Indicates a new beat is starting
  if (hrBeatTime == 0) {
    hrBeatTime = currentMillis;      // Update heartbeat start time
    return;                          // Haptic motor already off, need not update
  }

  // Calculate the heartbeat length based on the BPM
  long calculatedInterval = 60000 / bpm;

  // Is the current heartbeat still elapsing
  if (currentMillis - hrBeatTime < calculatedInterval) {
    // Calculate correct millivolts to send to haptic motor
    double targetMillivolts = calculateMillivolts(bpm, currentMillis - hrBeatTime);

    // Calculate the duty cycle (for pwm) using target millivolts
    int dutyCycle = calculateDutyCycle(targetMillivolts, operatingVoltage);

    // Send to haptic motor
    analogWrite(pwmPin, dutyCycle);
  } else {
    analogWrite(pwmPin, 0);   // Turn off the motor
    hrBeatTime = 0;           // Indicate next beat is ready to start
  }
}


// Function to calculate QT interval (milliseconds) given BPM
float calculateQTi(int bpm) {
  return -5 * bpm + 700;
}


// Piecewise function that returns target millivolts given BPM and milliseconds into the current heartbeat
double calculateMillivolts(int bpm, unsigned long milliseconds) {
  int x = milliseconds;
  int qrs = 80;
  int u = 120;
  float qti = calculateQTi(bpm);

  if (x >= 0 && x < qrs) {
    return sin((PI / qrs) * x) * 3000;
  } else if (x >= (qti - u) && x < qti) {
    return sin((PI / u) * (x - (qti - u))) * 1250;
  } else {
    return 0;
  }
}


// Return duty cycle inferred from specified millivolts
int calculateDutyCycle(double millivolts, int operatingVoltage) {
  // Ensure the millivolts value is within the range
  if (millivolts < 0) {
    millivolts = 0;
  } else if (millivolts > operatingVoltage) {
    millivolts = operatingVoltage;
  }

  // Calculate the duty cycle
  return (millivolts * 255) / operatingVoltage;
}

