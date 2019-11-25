#include <M5Stack.h>
#include <esp8266-google-home-notifier.h>
 
const char* ssid = "SH-M05-yutaka"; //ご自分のルーターのSSIDに書き換えてください
const char* password = "yutakaiwagaki"; //ご自分のルーターのパスワードに書き換えてください
 
GoogleHomeNotifier ghn;
const char displayName[] = "リビング"; //スマホアプリで確認。Google Home のデバイス名
 
void setup(){
  // initialize the M5Stack object
  M5.begin();
 
  // Lcd display
  M5.Lcd.println("ESP32/ESP8266 google-home-notifier Test");
  M5.Lcd.println("Press the button A English.");
  M5.Lcd.println("Press the button B Japanese1.");
  M5.Lcd.println("Press the button C Japanese2.");
 
  // Set the wakeup button
  M5.setWakeupButton(BUTTON_A_PIN);
  M5.setWakeupButton(BUTTON_B_PIN);
  M5.setWakeupButton(BUTTON_C_PIN);
 
  Serial.println("");
  Serial.print("connecting to Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP
}
 
void loop() {
  if(M5.BtnA.wasPressed()) {
    googleHomeConnection( "en", "English. Button A pushed. Hello World" );
  }
  if(M5.BtnB.wasPressed()) {
    googleHomeConnection( "ja", "日本語です。 ボタン B が押されました。Hello World" );
  }
  if(M5.BtnC.wasPressed()) {
    googleHomeConnection( "ja", "ボタンＣです。グーグルホームノティファイア、使えそう。" );
  }
 
  M5.update();
}
 
//****************************************
void googleHomeConnection( String lang_str1, String talk_str1 ){
  Serial.println("connecting to Google Home...");
  if (ghn.device( displayName, lang_str1.c_str() ) != true) {
    Serial.println(ghn.getLastError());
    return;
  }
  Serial.print("found Google Home(");
  Serial.print(ghn.getIPAddress());
  Serial.print(":");
  Serial.print(ghn.getPort());
  Serial.println(")");
 
  if (ghn.notify( talk_str1.c_str() ) != true) {
    Serial.println(ghn.getLastError());
    return;
  }
  Serial.println("Done.");
}
