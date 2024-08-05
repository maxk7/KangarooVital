// ********* basic.ino **********/
// Interfacing template for our Arduino circuit and Firebase Server
// By Max Konzerowsky
// August 2024

#include <WiFiNINA.h>
#include <Firebase_Arduino_WiFiNINA.h>


// ********* wifi/firebase ********* //
// Replace with your network credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Firebase project credentials
#define FIREBASE_HOST "your_project_id.firebaseio.com"
#define FIREBASE_AUTH "your_database_secret_or_API_key"

FirebaseData firebaseData;

// Identification for this user
String userId = "example_user_id";
String path = "/users/" + userId;


// ********* hr pulse ********* //
// Pins
const int potPin = A0;              // Define the analog pin for the potentiometer
const int fetGatePin = 3;           // Define the pin for controlling the FET gate
const int internalLedPin = 13;      // Define the pin for the internal LED on the board

// Potentiometer value
int potValue = 0;

// HR & BPM
unsigned long hrBeatTime = 0;       // Starting time of current heartbeat (ms)


// ********* arduino setup ********* //
void setup() {
  pinMode(fetGatePin, OUTPUT);        // Initialize the FET gate pin as an output
  pinMode(internalLedPin, OUTPUT);    // Initialize the internal LED pin as an output
  digitalWrite(fetGatePin, LOW);      // Set initial output to 0V for haptic motor
  digitalWrite(internalLedPin, LOW);  // Set initial output to LOW for the internal LED

  // Initialize the serial communication for debugging
  Serial.begin(9600);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}


// ********* arduino loop ********* //
void loop() {
  // Read ardBPM from Firebase
  int ardBPM = getArdBPM();
  
  // Use the ardBPM value in the pulse function
  pulse(ardBPM);

  // Optionally, update ardBPM in Firebase with a new value
  int newArdBPM = 75; // Example value
  setArdBPM(newArdBPM);

  delay(10000); // Wait for 10 seconds before next loop
}


// ********* helper functions ********* //  
// Firebase function to get ardBPM
int getArdBPM() {
  if (Firebase.getInt(firebaseData, path + "/ardBPM")) {
    int ardBPM = firebaseData.intData();
    Serial.print("ardBPM from Firebase: ");
    Serial.println(ardBPM);
    return ardBPM;
  } else {
    Serial.println("Failed to get ardBPM: " + firebaseData.errorReason());
    return 0; // Return a default value or handle the error as needed
  }
}


// Firebase function to set ardBPM
void setArdBPM(int newArdBPM) {
  if (Firebase.setInt(firebaseData, path + "/ardBPM", newArdBPM)) {
    Serial.println("Updated ardBPM successfully");
  } else {
    Serial.println("Failed to update ardBPM: " + firebaseData.errorReason());
  }
}


// Pulse haptic motor at specific BPM
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

