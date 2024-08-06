// ********* basic.ino **********/
// Interfacing template for our Arduino circuit and Firebase Server
// By Max Konzerowsky
// August 2024

#include <WiFiNINA.h>
#include <utility/wifi_drv.h>
#include <Firebase_Arduino_WiFiNINA.h>


// ********* wifi/firebase ********* //
// Replace with your network credentials
const char* ssid = "Dartmouth Public";
const char* password = "";

// Firebase project credentials
#define FIREBASE_HOST "kangaroo-1b446-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "yCLm2yRO86RGI4eiEWvc4ZzmWpDADQWz7BQ1d7hOC"

FirebaseData firebaseData;

// Identification for this user
String userId = "example_user_id";
String path = "/users/" + userId;


// ********* hr pulse ********* //
// Pins
const int potPin = A0;              // Define the analog pin for the potentiometer
const int fetGatePin = 3;           // Define the pin for controlling the FET gate
const int internalLedRed = 26;      // Define the pin for the internal LED on the board
const int internalLedGrn = 25;      // Define the pin for the internal LED on the board
const int internalLedBlu = 27;      // Define the pin for the internal LED on the board

// Potentiometer value
int potValue = 0;

// HR & BPM
int bpm = 0;                        // BPM of haptic motor
unsigned long hrBeatTime = 0;       // Starting time of current heartbeat (ms)
unsigned long previousMillis = 0;   // Stores the last time bpm was updated
unsigned long currentMillis = 0;    // Stores the current time in ms
const long interval = 60000/4;        // Interval to update bpm


// ********* arduino setup ********* //
void setup() {
  pinMode(fetGatePin, OUTPUT);                  // Initialize the FET gate pin as an output
  digitalWrite(fetGatePin, LOW);                // Set initial output to 0V for haptic motor
  
  WiFiDrv::pinMode(internalLedRed, OUTPUT);     // Initialize the internal LED pin as an output
  WiFiDrv::analogWrite(internalLedRed, 0);      // Set initial output to LOW for the internal LED

  // Initialize the serial communication for debugging
  Serial.begin(9600);

  // Connect to WiFi
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, ssid, password);
  Firebase.reconnectWiFi(true);

  bpm = getArdBPM();
}


// ********* arduino loop ********* //
void loop() {
  currentMillis = millis();

  // Check if a minute has passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Update bpm from Firebase
    bpm = getArdBPM();
    
    // Optionally, update ardBPM in Firebase with a new value
    // int newArdBPM = 175; // Example value
    // setArdBPM(newArdBPM);
  }
  
  // Use the ardBPM value in the pulse function (outside of the interval check to keep using the last bpm value)
  pulse();
}


// ********* helper functions ********* //  
// Firebase function to get ardBPM
int getArdBPM() {
  if (Firebase.getInt(firebaseData, path + "/ard")) {
    int ardBPM = firebaseData.intData();
    Serial.print("ardBfrom Firebase: ");
    Serial.println(ardBPM);
    return ardBPM;
  } else {
    Serial.println("Failed to get ard: " + firebaseData.errorReason());
    return 65; // Return a default value or handle the error as needed
  }
}


// Firebase function to set ardBPM
void setArdBPM(int newArdBPM) {
  if (Firebase.setInt(firebaseData, path + "/ard", newArdBPM)) {
    Serial.println("Updated ard successfully");
  } else {
    Serial.println("Failed to update ard: " + firebaseData.errorReason());
  }
}


// Pulse haptic motor at specific BPM
void pulse() {
  static bool pwmState = LOW;                             // track the state of fetGatePin
  unsigned long hrLength = 60000 / bpm;                   // heartbeat duration given bpm
  unsigned long elapsedTime = 0;                          // elapsed time of current heartbeat

  currentMillis = millis();                               // Update currentMillis inside pulse function

  // Is a new beat starting
  if (hrBeatTime == 0) {
    hrBeatTime = currentMillis;                           // Update heartbeat start time to current
    pwmState = HIGH;
    digitalWrite(fetGatePin, pwmState);                   // Turn haptic motor on
    WiFiDrv::analogWrite(internalLedRed, 10);             // Turn internal LED on
    return;
  }

  elapsedTime = currentMillis - hrBeatTime;               // Update elapsed time of current heartbeat

  // Is the current heartbeat still elapsing
  if (elapsedTime < hrLength) {
    // Is the heartbeat on the first pulse or second pulse
    if ((elapsedTime < 41) || ((140 < elapsedTime) && (elapsedTime < 181))) {
      if (!pwmState) {
        pwmState = HIGH;
        digitalWrite(fetGatePin, pwmState);
        WiFiDrv::analogWrite(internalLedRed, 10);         // Turn internal LED on
      }
    } 
    
    else {
      if (pwmState) {
        pwmState = LOW;
        digitalWrite(fetGatePin, pwmState);
        WiFiDrv::analogWrite(internalLedRed, 0);          // Turn internal LED off
      }
    }
  } 
  
  else {
    if (pwmState) {
      pwmState = LOW;
      digitalWrite(fetGatePin, pwmState);                 // Turn off the motor
      WiFiDrv::analogWrite(internalLedRed, 0);            // Turn internal LED off
    }

    hrBeatTime = 0;                                       // Indicate next beat is ready to start
  }
}
