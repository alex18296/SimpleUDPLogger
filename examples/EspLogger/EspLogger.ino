#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#elif ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#error Unsupported architecture, use ARDUINO_ARCH_ESP8266 or ARDUINO_ARCH_ESP32
#endif

#define SSID          "YourRouterSSID"
#define PASSWORD      "YourRouterPassword"

#define MY_SSID       "MyESP"
#define MY_PASSWORD   "ESP00000"

#define R_SERVER      "192.168.14.47"

#define UDP_LOGGER
#include <SimpleUDPLogger.h>

#define EXAMPLE_STA_MODE
//#define EXAMPLE_AP_MODE

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.println("Disabling wifi module");
  WiFi.mode(WIFI_OFF);
  delay(5000);
#if defined(EXAMPLE_STA_MODE) && !defined(EXAMPLE_AP_MODE)
  Serial.println("Initializing STA mode");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.print("\r\nConnected! IP address: ");
  Serial.println(WiFi.localIP());
#elif !defined(EXAMPLE_STA_MODE) && defined(EXAMPLE_AP_MODE)
  Serial.println("Initializing AP mode");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(MY_SSID, MY_PASSWORD);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
#elif defined(EXAMPLE_STA_MODE) && defined(EXAMPLE_AP_MODE)
  Serial.println("Initializing AP STA mode");
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(MY_SSID, MY_PASSWORD);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.print("\r\nConnected! IP address: ");
  Serial.println(WiFi.localIP());
#endif

  UDP_LOG_BEGIN(R_SERVER, LOG_MODE_DEBUG);
  UDP_LOG_INFO("start");
}

void loop() {
  static unsigned long last_ms = 0;
  if (abs(millis() - last_ms) > 500) {
    last_ms = millis();
    UDP_LOG_DEBUG("ms: %lu", last_ms);
  }
}

/*

In file /etc/rsyslog.conf uncomment line for loading module "imudp" and using udp port 514
# provides UDP syslog reception
module(load="imudp")
input(type="imudp" port="514")

Restart rsyslog.service
sudo systemctl restart rsyslog.service

See log output result 
tail -f /var/log/syslog |grep 'ESP:'
Jun  9 17:34:37 192.168.14.57 ESP: INFO: start
Jun  9 17:34:37 192.168.14.57 ESP: DEBUG: ms: 6665
Jun  9 17:34:37 192.168.14.57 ESP: DEBUG: ms: 7166
Jun  9 17:34:37 192.168.14.57 ESP: DEBUG: ms: 7667
Jun  9 17:34:38 192.168.14.57 ESP: DEBUG: ms: 8168
Jun  9 17:34:38 192.168.14.57 ESP: DEBUG: ms: 8669
Jun  9 17:34:39 192.168.14.57 ESP: DEBUG: ms: 9170
Jun  9 17:34:39 192.168.14.57 ESP: DEBUG: ms: 9671
Jun  9 17:34:40 192.168.14.57 ESP: DEBUG: ms: 10172

*/
