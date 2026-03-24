/*
 * ESP32 Code for "WorkHere" Project
 * Reads Temperature, Sound, Light sensors
 * Sends data to Gemini API for scoring
 * Communicates with MCXC444 via UART
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ===== CONFIGURATION - EDIT THESE =====
const char* ssid = "John";
const char* password = "19062003";
const char* apiKey = "AIzaSyDkqd9CSOz0ufQGuP_PaAchDXZMV0yZPsQ";
// ======================================

// UART Pins for MCXC444 communication
#define UART_TX_PIN 17
#define UART_RX_PIN 18

// Sensor Pins
#define TEMP_PIN 1
#define SOUND_PIN 2
#define LIGHT_PIN 3

// Global variables for sensor readings
float temperature = 0;
int soundLevel = 0;
int lightLevel = 0;

// Gemini response structure
struct {
  int tempScore;
  int soundScore;
  int lightScore;
  String overall;
} scores;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(TEMP_PIN);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature tempSensor(&oneWire);

void setup() {
  Serial.begin(115200);
  
  // Initialize UART for MCXC444 communication
  Serial1.begin(9600, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  
  analogReadResolution(12);
  tempSensor.begin();

  // Initialize sensor pins
  pinMode(TEMP_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);
  
  // Connect to WiFi
  connectToWiFi();
  Serial.println("ESP32 Ready - Waiting for trigger from MCXC444");
}

void loop() {
  // Check if MCXC444 requested sensor reading
  if (Serial1.available() > 0) {
    String command = Serial1.readStringUntil('\n');
    Serial.println(command);
    command.trim();
    
    if (command == "READ_SENSORS") {
      Serial.println("Received trigger - Reading sensors...");
      
      // Read all sensors
      readSensors();
      
      // Send to Gemini and get scores
      if (queryGemini()) {
        // Send scores back to MCXC444
        sendScoresToMCXC();
      } else {
        Serial1.println("ERROR:API_FAILED");
      }
    }
  }
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void readSensors() {
  // Read Digital Temperature (DS18B20)
  tempSensor.requestTemperatures(); // Send the command to get temperatures
  temperature = tempSensor.getTempCByIndex(0); // Read the temperature in Celsius
  
  // Read analog sound level
  soundLevel = analogRead(SOUND_PIN);
  
  // Read analog light level
  lightLevel = analogRead(LIGHT_PIN);
  
  Serial.println("Sensor Readings:");
  Serial.print("  Temperature: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("  Sound Level: "); Serial.println(soundLevel);
  Serial.print("  Light Level: "); Serial.println(lightLevel);
}

bool queryGemini() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    return false;
  }
  
  HTTPClient http;
  
  // NEW: Point to the Gemini OpenAI-compatible endpoint
  http.begin("https://generativelanguage.googleapis.com/v1beta/openai/chat/completions");
  
  http.setTimeout(15000);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(apiKey)); // Gemini accepts the exact same Bearer format
  
  // Build prompt with sensor data
  String prompt = "Based on workplace environment data: ";
  prompt += "Temperature = " + String(temperature) + "°C, ";
  prompt += "Sound level = " + String(soundLevel) + ", ";
  prompt += "Light level = " + String(lightLevel) + ". ";
  prompt += "Rate each from 1-5 (1=poor, 5=excellent) for workplace comfort. ";
  prompt += "Provide overall assessment: Good/Poor/Bad. ";
  prompt += "Return ONLY a raw JSON object. DO NOT wrap it in markdown formatting, backticks, or code blocks. Format: ";
  prompt += "{\"tempScore\":x, \"soundScore\":x, \"lightScore\":x, \"overall\":\"x\"}";
  
  // Create JSON payload
  StaticJsonDocument<1024> doc;
  
  // NEW: Use the extremely fast Gemini model
  doc["model"] = "gemini-3.1-flash-lite-preview"; 
  
  JsonArray messages = doc.createNestedArray("messages");
  JsonObject message = messages.createNestedObject();
  message["role"] = "user";
  message["content"] = prompt;
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.println("Sending to Gemini API...");
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("Response received:");
    Serial.println(response);
    
    // Parse JSON response (Your existing parser will work perfectly!)
    return parseGeminiResponse(response);
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
    String errorResponse = http.getString();
    Serial.println(errorResponse);
    return false;
  }
  
  http.end();
}

bool parseGeminiResponse(String jsonResponse) {
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, jsonResponse);
  
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return false;
  }
  
  // Extract the content from Gemini response
  String content = doc["choices"][0]["message"]["content"];
  Serial.print("Gemini content: ");
  Serial.println(content);
  
  // Parse the JSON inside content
  StaticJsonDocument<512> scoreDoc;
  DeserializationError scoreError = deserializeJson(scoreDoc, content);
  
  if (scoreError) {
    Serial.println("Could not parse scores from content");
    return false;
  }
  
  // Extract scores
  scores.tempScore = scoreDoc["tempScore"] | 3;
  scores.soundScore = scoreDoc["soundScore"] | 3;
  scores.lightScore = scoreDoc["lightScore"] | 3;
  scores.overall = scoreDoc["overall"] | "Unknown";
  
  Serial.println("Parsed Scores:");
  Serial.print("  Temperature: "); Serial.println(scores.tempScore);
  Serial.print("  Sound: "); Serial.println(scores.soundScore);
  Serial.print("  Light: "); Serial.println(scores.lightScore);
  Serial.print("  Overall: "); Serial.println(scores.overall);
  
  return true;
}

void sendScoresToMCXC() {
  // Format: "T:4,S:3,L:5,O:Good\n"
  String scoreString = "T:" + String(scores.tempScore) + 
                       ",S:" + String(scores.soundScore) +
                       ",L:" + String(scores.lightScore) +
                       ",O:" + scores.overall + "\n";
  
  Serial1.print(scoreString);
  Serial.print("Sent to MCXC444: ");
  Serial.print(scoreString);
}