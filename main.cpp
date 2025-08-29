#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// WiFi credentials
const char* ssid = "Display";
const char* password = "aaaabbbb";

// Groq API
String groq_api_key = "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";
String groq_url = "https://api.groq.com/openai/v1/chat/completions";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
    display.display();
  }
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi Connected!");
  display.display();

  server.begin();
}

String getGroqReply(String userText) {
  if(WiFi.status() != WL_CONNECTED) return "WiFi Error";

  HTTPClient http;
  http.begin(groq_url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + groq_api_key);

  String requestBody = "{\"model\": \"llama3-8b-8192\", \"messages\": [{\"role\": \"user\", \"content\": \"" + userText + "\"}]}";

  int httpResponseCode = http.POST(requestBody);
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);
    String reply = doc["choices"][0]["message"]["content"];
    http.end();
    return reply;
  } else {
    http.end();
    return "API Error";
  }
}

void loop() {
  WiFiClient client = server.available();
  if(client) {
    String currentLine = "";
    String receivedText = "";
    while(client.connected()) {
      if(client.available()) {
        char c = client.read();
        currentLine += c;

        if(c == '\n') {
          if(currentLine.length() == 1) {
            if(receivedText.length() > 0) {
              String reply = getGroqReply(receivedText);

              display.clearDisplay();
              display.setCursor(0,0);
              display.println("Q: " + receivedText);
              display.println("A: " + reply);
              display.display();
            }
            break;
          }
          if(currentLine.startsWith("msg=")) {
            receivedText = currentLine.substring(4);
            receivedText.trim();
          }
          currentLine = "";
        }
      }
    }
    client.stop();
  }
}
