/**
 * ESP8266 Controller — Wi-Fi, MQTT, OTA, Telegram
 * Управляет вендингом через Serial и Wi-Fi
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>

// === Настройки Wi-Fi ===
const char* ssid = "ВАША_СЕТЬ";
const char* password = "ВАШ_ПАРОЛЬ";

// === MQTT ===
const char* mqtt_server = "192.168.1.100"; // IP вашего сервера
WiFiClient espClient;
PubSubClient client(espClient);

// === Telegram (опционально) ===
const String BOT_TOKEN = "ВАШ_ТОКЕН";
const String CHAT_ID = "ВАШ_CHAT_ID";

void setup() {
  Serial.begin(9600);
  setupWiFi();
  client.setServer(mqtt_server, 1883);
  setupOTA();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();

  // Приём команд от Arduino
  if (Serial.available()) {
    String cmd = Serial.readString();
    cmd.trim();

    // Отправка в MQTT
    client.publish("vending/event", cmd.c_str());

    // Пример: отправка в Telegram
    if (cmd.startsWith("SOLD")) {
      sendToTelegram("🛒 Продажа: " + cmd);
    }
  }
}

void setupWiFi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Wi-Fi подключён");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  if (!client.connected()) {
    if (client.connect("VendingESP01")) {
      client.subscribe("vending/cmd");
    }
  }
}

void setupOTA() {
  ArduinoOTA.onStart([]() { Serial.println("Начало OTA..."); });
  ArduinoOTA.onEnd([]() { Serial.println("\nOTA завершено"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.begin();
}

void sendToTelegram(String message) {
  String url = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + message;
  HTTPClient http;
  http.begin(url);
  http.GET();
  http.end();
}