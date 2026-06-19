#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>

// Replace with your network credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Current firmware version
String current_version = "1.0";

// URL to the version file and firmware binary on GitHub
// Using raw.githubusercontent.com to get the actual file content
const char* version_url = "https://raw.githubusercontent.com/SUMUKH2597/esp82666_ota/main/version.txt";
const char* firmware_url = "https://raw.githubusercontent.com/SUMUKH2597/esp82666_ota/main/firmware.bin";

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  // Check for OTA updates
  checkForUpdates();
}

void loop() {
  // Your main code logic here
  
  // Example: Check for updates every hour (3600000 ms)
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 3600000) {
    checkForUpdates();
    lastCheck = millis();
  }
}

void checkForUpdates() {
  Serial.println("Checking for updates...");
  
  WiFiClientSecure client;
  // Use insecure connection to bypass SSL certificate validation for simplicity
  client.setInsecure();

  HTTPClient https;
  
  if (https.begin(client, version_url)) {
    int httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String new_version = https.getString();
      new_version.trim(); // Remove any newlines or spaces
      
      Serial.println("Current version: " + current_version);
      Serial.println("Available version: " + new_version);
      
      if (new_version != current_version && new_version != "") {
        Serial.println("New firmware available. Updating...");
        updateFirmware();
      } else {
        Serial.println("Already on latest version.");
      }
    } else {
      Serial.printf("Failed to fetch version file. HTTP Code: %d\n", httpCode);
    }
    https.end();
  } else {
    Serial.println("Unable to connect to GitHub.");
  }
}

void updateFirmware() {
  WiFiClientSecure client;
  client.setInsecure();

  // Disable auto-reboot to handle it manually if needed, or leave enabled
  ESPhttpUpdate.rebootOnUpdate(true);

  Serial.println("Downloading firmware...");
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, firmware_url);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK - System will reboot.");
      break;
  }
}
