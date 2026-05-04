
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// ==========================================
// UWAGA: Przed wgraniem na ESP32 podmień poniższy tekst na swój klucz API!
// ==========================================
String kluczAPI = "TUTAJ_WKLEJ_SWOJ_KLUCZ";
String miasto = "";


unsigned long ostatniePobranie = 0;
const unsigned long INTERWAL = 60000; 


void pobierzLokalizacje() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Lokalizowanie GPS...");
  display.display();

  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.begin("http://ip-api.com/json/"); 
    int kodOdpowiedzi = http.GET();

    if (kodOdpowiedzi == 200) {
      String odpowiedzJSON = http.getString();
      JsonDocument doc; 
      deserializeJson(doc, odpowiedzJSON);

      miasto = doc["city"].as<String>(); 
    } else {
      miasto = "Warsaw";
    }
    http.end();
  }
}


void pobierzPogode() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Pobieranie pogody...");
  display.display();

  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + miasto + "&units=metric&appid=" + kluczAPI;
    
    http.begin(url); 
    int kodOdpowiedzi = http.GET(); 

    if (kodOdpowiedzi == 200) {
      String odpowiedzJSON = http.getString();
      JsonDocument doc; 
      DeserializationError blad = deserializeJson(doc, odpowiedzJSON);

      if (!blad) {
        float temperatura = doc["main"]["temp"];
        int wilgotnosc = doc["main"]["humidity"];
        const char* status = doc["weather"][0]["main"]; 
        float wiatr = doc["wind"]["speed"];

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Pogoda: ");
        display.println(miasto);
        display.println("---------------------");

        display.setTextSize(2);
        display.print(temperatura, 1); 
        display.println(" C");
        
        display.setTextSize(1);
        display.println();
        display.print("Stan: ");
        display.println(status);
        display.print("Wilg: ");
        display.print(wilgotnosc);
        display.println(" %");
        display.print("Wiatr: ");
        display.print(wiatr, 1);
        display.println(" m/s");

      } else {
        display.println("Blad danych!");
      }
    } else {
      display.print("Blad serwera: ");
      display.println(kodOdpowiedzi);
    }
    http.end(); 
  }
  display.display();
}

void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { for(;;); }
  
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Laczenie z Wi-Fi...");
  display.setCursor(0, 30);
  display.println("Jesli brak sieci,");
  display.println("szukaj: Meteo-Setup");
  display.display();

 
 // --- WIFI MANAGER (Tymczasowo wyłączona dla symulatora) ---
  WiFiManager wm;
  bool res = wm.autoConnect("Meteo-Setup");
  if(!res) { ESP.restart(); }

  // Wymuszone połączenie z internetem Wokwi:
  // WiFi.begin("Wokwi-GUEST", "", 6);
  // while (WiFi.status() != WL_CONNECTED) {
  //  delay(250);
 // }
  // --------------------------

 
  pobierzLokalizacje();
  pobierzPogode();
}

void loop() {
  if (millis() - ostatniePobranie > INTERWAL) {
    ostatniePobranie = millis();
    pobierzPogode();
  }
}