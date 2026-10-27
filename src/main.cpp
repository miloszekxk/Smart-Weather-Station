#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <Preferences.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PRZYCISK_BOOT 0

const char* NAZWA_HOTSPOTU = "Meteo-Setup";
const unsigned long INTERWAL = 10UL * 60 * 1000;      // OpenWeatherMap odswieza dane co ok. 10 min
const unsigned long INTERWAL_PO_BLEDZIE = 30UL * 1000;
const unsigned long CZAS_PRZYTRZYMANIA = 3000;        // tyle trzeba trzymac BOOT, zeby wejsc w konfiguracje
const int TIMEOUT_PORTALU = 300;                      // sekundy

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WiFiManager wm;
Preferences pamiec;

// Ustawienia zapisywane w pamieci nieulotnej (NVS) - wpisywane przez przegladarke, nie w kodzie
String kluczAPI = "";
String miastoUstawione = "";   // puste = lokalizacja automatyczna po adresie IP

WiFiManagerParameter poleKlucz("klucz", "Klucz API OpenWeatherMap", "", 40);
WiFiManagerParameter poleMiasto("miasto", "Miasto (puste = automatycznie)", "", 40);

// Lokalizacja wykryta po adresie IP
bool maLokalizacje = false;
float szerokosc = 0;
float dlugosc = 0;

unsigned long ostatniePobranie = 0;
unsigned long odstep = 0;             // 0 = pierwsze pobranie od razu po starcie
unsigned long wcisniecieOd = 0;


// Ekran SSD1306 nie ma polskich znakow - zamiana na litery bez ogonkow
String bezPolskichZnakow(const String& tekst) {
  const char* polskie[] = {"ą", "ć", "ę", "ł", "ń", "ó", "ś", "ź", "ż", "Ą", "Ć", "Ę", "Ł", "Ń", "Ó", "Ś", "Ź", "Ż"};
  const char* zwykle[]  = {"a", "c", "e", "l", "n", "o", "s", "z", "z", "A", "C", "E", "L", "N", "O", "S", "Z", "Z"};
  String wynik = tekst;
  for (int i = 0; i < 18; i++) {
    wynik.replace(polskie[i], zwykle[i]);
  }
  return wynik;
}

// Nazwa miasta w adresie URL (spacje, polskie znaki)
String kodujURL(const String& tekst) {
  String wynik = "";
  const char* hex = "0123456789ABCDEF";
  for (size_t i = 0; i < tekst.length(); i++) {
    uint8_t c = tekst[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.') {
      wynik += (char)c;
    } else {
      wynik += '%';
      wynik += hex[c >> 4];
      wynik += hex[c & 0x0F];
    }
  }
  return wynik;
}

void komunikat(const char* linia1, const char* linia2 = "", const char* linia3 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println(linia1);
  display.setCursor(0, 30);
  display.println(linia2);
  display.println(linia3);
  display.display();
  Serial.printf("%s %s %s\n", linia1, linia2, linia3);
}


void wczytajUstawienia() {
  pamiec.begin("meteo", true);
  kluczAPI = pamiec.getString("klucz", "");
  miastoUstawione = pamiec.getString("miasto", "");
  pamiec.end();

  poleKlucz.setValue(kluczAPI.c_str(), 40);
  poleMiasto.setValue(miastoUstawione.c_str(), 40);
}

void zapiszUstawienia() {
  kluczAPI = String(poleKlucz.getValue());
  kluczAPI.trim();
  miastoUstawione = String(poleMiasto.getValue());
  miastoUstawione.trim();

  pamiec.begin("meteo", false);
  pamiec.putString("klucz", kluczAPI);
  pamiec.putString("miasto", miastoUstawione);
  pamiec.end();
  Serial.println("Zapisano ustawienia");
}

// Tryb konfiguracji na zadanie (przytrzymany BOOT albo brak klucza API)
void uruchomPortal() {
  komunikat("Tryb konfiguracji", "Polacz sie z siecia:", NAZWA_HOTSPOTU);
  wm.startConfigPortal(NAZWA_HOTSPOTU);
  // Po zapisie albo po czasie restart - ustawienia wczytaja sie od nowa
  ESP.restart();
}


void pobierzLokalizacje() {
  komunikat("Lokalizowanie (IP)...");

  // Darmowy plan ip-api.com dziala tylko po HTTP
  HTTPClient http;
  http.begin("http://ip-api.com/json/?fields=status,lat,lon");
  int kodOdpowiedzi = http.GET();

  if (kodOdpowiedzi == 200) {
    JsonDocument doc;
    DeserializationError blad = deserializeJson(doc, http.getString());
    if (!blad && doc["status"] == "success") {
      szerokosc = doc["lat"];
      dlugosc = doc["lon"];
      maLokalizacje = true;
    }
  }
  http.end();
}

// Zwraca true, gdy udalo sie pobrac i wyswietlic pogode
bool pobierzPogode() {
  String zapytanie;
  if (miastoUstawione.length() > 0) {
    zapytanie = "q=" + kodujURL(miastoUstawione);
  } else {
    if (!maLokalizacje) pobierzLokalizacje();
    if (maLokalizacje) {
      zapytanie = "lat=" + String(szerokosc, 4) + "&lon=" + String(dlugosc, 4);
    } else {
      zapytanie = "q=Warsaw";
    }
  }

  komunikat("Pobieranie pogody...");

  // HTTPS, zeby klucz API nie szedl po sieci otwartym tekstem
  WiFiClientSecure klient;
  klient.setInsecure();   // szyfrowanie bez weryfikacji certyfikatu - ESP32 nie ma wgranych certyfikatow CA
  HTTPClient http;
  http.begin(klient, "https://api.openweathermap.org/data/2.5/weather?" + zapytanie + "&units=metric&appid=" + kluczAPI);
  int kodOdpowiedzi = http.GET();

  if (kodOdpowiedzi == 401) {
    http.end();
    komunikat("Bledny klucz API!", "Przytrzymaj BOOT 3s,", "aby go zmienic");
    return false;
  }
  if (kodOdpowiedzi == 404) {
    http.end();
    komunikat("Nie znaleziono miasta", "Przytrzymaj BOOT 3s,", "aby je zmienic");
    return false;
  }
  if (kodOdpowiedzi != 200) {
    http.end();
    String kod = "Kod: " + String(kodOdpowiedzi);
    komunikat("Blad serwera pogody", kod.c_str());
    return false;
  }

  JsonDocument doc;
  DeserializationError blad = deserializeJson(doc, http.getString());
  http.end();
  if (blad) {
    komunikat("Blad danych!");
    return false;
  }

  float temperatura = doc["main"]["temp"];
  int wilgotnosc = doc["main"]["humidity"];
  const char* stan = doc["weather"][0]["main"] | "?";
  float wiatr = doc["wind"]["speed"];
  String miasto = bezPolskichZnakow(doc["name"].as<String>());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Pogoda: ");
  display.println(miasto);
  display.println("---------------------");

  display.setTextSize(2);
  display.print(temperatura, 1);
  display.println(" C");

  display.setTextSize(1);
  display.println();
  display.print("Stan: ");
  display.println(stan);
  display.print("Wilg: ");
  display.print(wilgotnosc);
  display.println(" %");
  display.print("Wiatr: ");
  display.print(wiatr, 1);
  display.println(" m/s");
  display.display();

  Serial.printf("%s: %.1f C, %d %%, %s, %.1f m/s\n", miasto.c_str(), temperatura, wilgotnosc, stan, wiatr);
  return true;
}

void sprawdzPrzycisk() {
  if (digitalRead(PRZYCISK_BOOT) == LOW) {
    if (wcisniecieOd == 0) wcisniecieOd = millis();
    if (millis() - wcisniecieOd > CZAS_PRZYTRZYMANIA) uruchomPortal();
  } else {
    wcisniecieOd = 0;
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(PRZYCISK_BOOT, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Nie wykryto ekranu OLED - sprawdz polaczenia");
    for (;;) delay(1000);
  }
  display.setTextColor(WHITE);

  wczytajUstawienia();
  wm.addParameter(&poleKlucz);
  wm.addParameter(&poleMiasto);
  wm.setSaveConfigCallback(zapiszUstawienia);
  wm.setSaveParamsCallback(zapiszUstawienia);
  wm.setConfigPortalTimeout(TIMEOUT_PORTALU);

  // Bez klucza API stacja nic nie pokaze - od razu tryb konfiguracji
  if (kluczAPI.length() == 0) {
    uruchomPortal();
  }

  komunikat("Laczenie z Wi-Fi...", "Jesli brak sieci,", "szukaj: Meteo-Setup");
  if (!wm.autoConnect(NAZWA_HOTSPOTU)) {
    ESP.restart();
  }
}

void loop() {
  sprawdzPrzycisk();

  if (millis() - ostatniePobranie < odstep) return;
  ostatniePobranie = millis();

  if (WiFi.status() != WL_CONNECTED) {
    komunikat("Brak Wi-Fi", "Ponowne laczenie...");
    odstep = INTERWAL_PO_BLEDZIE;
    return;
  }

  bool udane = pobierzPogode();
  odstep = udane ? INTERWAL : INTERWAL_PO_BLEDZIE;
}
