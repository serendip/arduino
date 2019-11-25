/*
   This is a basic example on how to use Espalexa with RGB color devices.
*/
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
//#define ESPALEXA_ASYNC //コメントアウト
#include <Espalexa.h>
#include <M5StickC.h>

// prototypes
boolean connectWifi();

//callback function prototype
void colorLightChanged(uint8_t brightness, uint32_t rgb);
void brightnessChanged(uint8_t brightness);

// Change this!!
const char* ssid = "TP-Link_5797";
const char* password = "22954626";

boolean wifiConnected = false;

Espalexa espalexa;

#define LED 10 //M5StickCに内蔵されているLEDを使う

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  // Initialise wifi connection
  wifiConnected = connectWifi();

  if (wifiConnected) {
    espalexa.addDevice("フルカラーデバイス", colorLightChanged);
    espalexa.addDevice("明るさデバイス", brightnessChanged);
    espalexa.begin();

  } else
  {
    while (1) {
      Serial.println("Cannot connect to WiFi. Please check data and reset the ESP.");
      delay(2500);
    }
  }
  M5.begin();

  M5.Lcd.setRotation(1);
  M5.Lcd.setTextFont(4);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("I'm linking to Alexa");


}

void loop()
{
  espalexa.loop();
  delay(1);
}


void brightnessChanged(uint8_t brightness) {
  Serial.print("brightness :");
  Serial.println(brightness);

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 16);
  M5.Lcd.setTextFont(7);
  M5.Lcd.printf("%d", brightness);
}


//the color device callback function has two parameters
void colorLightChanged(uint8_t brightness, uint32_t rgb) {
  //do what you need to do here, for example control RGB LED strip
  Serial.print("Brightness: ");
  Serial.print(brightness);
  Serial.print(", Red: ");
  Serial.print((rgb >> 16) & 0xFF); //get red component
  Serial.print(", Green: ");
  Serial.print((rgb >>  8) & 0xFF); //get green
  Serial.print(", Blue: ");
  Serial.println(rgb & 0xFF); //get blue

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Bright:%d\n", brightness);
  M5.Lcd.printf("R: %d\n", (rgb >> 16) & 0xFF);
  M5.Lcd.printf("G: %d\n", (rgb >> 8) & 0xFF);
  M5.Lcd.printf("B: %d\n", rgb & 0xFF);
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi() {
  boolean state = true;
  int i = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 40) {
      state = false; break;
    }
    i++;
  }
  Serial.println("");
  if (state) {
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED, LOW);
    delay(3000);
    digitalWrite(LED, HIGH);
  }
  else {
    Serial.println("Connection failed.");
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED, LOW);
      delay(500);
      digitalWrite(LED, HIGH);
      delay(500);
    }
  }
  return state;
}
