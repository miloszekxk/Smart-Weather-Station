<div align="center">

# 🌦️ ESP32 Smart Weather Station (Auto-Location & WiFiManager)

![Platform](https://img.shields.io/badge/Platform-ESP32-blue.svg)
![Framework](https://img.shields.io/badge/Framework-Arduino%20%2F%20PlatformIO-orange.svg)
![API](https://img.shields.io/badge/API-OpenWeatherMap-red.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

> Inteligentna stacja pogodowa oparta na mikrokontrolerze ESP32. Projekt pobiera aktualne dane meteorologiczne z sieci, jednak w odróżnieniu od prostych układów, **nie wymaga wpisywania haseł Wi-Fi ani lokalizacji na stałe w kodzie**.

Układ jest w 100% zautomatyzowany – po podłączeniu do zasilania samodzielnie namierza swoje położenie geograficzne (na podstawie adresu IP) i pobiera parametry dla właściwego miasta.

</div>

---

## ✨ Główne funkcje 

* 📡 **WiFiManager (Brak haseł w kodzie!):** Jeśli urządzenie nie zna sieci Wi-Fi, automatycznie tworzy własny Hotspot (`Meteo-Setup`). Konfiguracja odbywa się z poziomu przeglądarki w telefonie.
* 🌍 **Auto-Geolokalizacja:** Automatyczne wykrywanie miasta na podstawie zewnętrznego API geolokalizacyjnego (`ip-api.com`).
* ☁️ **Live Weather API:** Pobieranie aktualnej temperatury, wilgotności, siły wiatru i stanu nieba z serwerów **OpenWeatherMap**.
* 📺 **OLED Display:** Przejrzysty interfejs użytkownika wyświetlany na ekranie SSD1306 (magistrala I2C).
* 🔄 **Zoptymalizowane odpytywanie:** Układ pobiera dane w ustalonych interwałach (np. co 60 sekund), oszczędzając darmowe limity zapytań do API.

## 🛠️ Wymagania sprzętowe 

* Płytka mikrokontrolera **ESP32** (np. DevKit V1)
* Wyświetlacz **OLED 0.96" SSD1306** (I2C)
* Przewody połączeniowe (Jumper wires)

### Schemat połączeń 

| Ekran OLED | ESP32 | Opis |
| :---: | :---: | :--- |
| **VCC** | `3V3` | Zasilanie 3.3V |
| **GND** | `GND` | Masa układu |
| **SCL** | `GPIO 22` | Linia zegara (I2C) |
| **SDA** | `GPIO 21` | Linia danych (I2C) |

## 📦 Wykorzystane biblioteki

Projekt do działania wymaga następujących bibliotek (PlatformIO pobierze je automatycznie):
* `tzapu/WiFiManager`
* `bblanchon/ArduinoJson`
* `adafruit/Adafruit SSD1306`
* `adafruit/Adafruit GFX Library`

## 🚀 Szczegółowa instrukcja uruchomienia (Step-by-Step Guide)

### 1. Przygotowanie środowiska
1. Pobierz i zainstaluj darmowy edytor **Visual Studio Code**.
2. Zainstaluj w nim rozszerzenie **PlatformIO IDE**.

### 2. Pobranie i konfiguracja projektu
1. Sklonuj to repozytorium lub pobierz je jako plik ZIP i rozpakuj.
2. Otwórz rozpakowany folder w Visual Studio Code za pomocą PlatformIO.
3. Wejdź na stronę [OpenWeatherMap](https://openweathermap.org/) i załóż darmowe konto.
4. Przejdź do zakładki *API keys* i wygeneruj swój prywatny klucz.
5. Otwórz plik `src/main.cpp` i wklej swój klucz w wyznaczonym miejscu na górze kodu:
   ```cpp
   String kluczAPI = "TUTAJ_WKLEJ_SWOJ_KLUCZ";

## 📝 Licencja
Projekt udostępniony na licencji MIT. Możesz go dowolnie modyfikować i używać w swoich rozwiązaniach domowych.