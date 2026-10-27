<div align="center">

# 🌦️ ESP32 Smart Weather Station (Auto-Location & WiFiManager)

![Platform](https://img.shields.io/badge/Platform-ESP32-blue.svg)
![Framework](https://img.shields.io/badge/Framework-Arduino%20%2F%20PlatformIO-orange.svg)
![API](https://img.shields.io/badge/API-OpenWeatherMap-red.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)
[![Build](https://img.shields.io/github/actions/workflow/status/miloszekxk/Smart-Weather-Station/build.yml?branch=main&label=Build)](https://github.com/miloszekxk/Smart-Weather-Station/actions/workflows/build.yml)

> Inteligentna stacja pogodowa oparta na mikrokontrolerze ESP32. Projekt pobiera aktualne dane meteorologiczne z sieci, jednak w odróżnieniu od prostych układów, **nie wymaga wpisywania haseł Wi-Fi, klucza API ani lokalizacji w kodzie**.

Układ jest w 100% zautomatyzowany – po podłączeniu do zasilania samodzielnie namierza swoje położenie geograficzne (na podstawie adresu IP) i pobiera pogodę dla tego miejsca.

</div>

---

## ✨ Główne funkcje

* 📡 **WiFiManager - zero haseł w kodzie:** jeśli urządzenie nie zna sieci Wi-Fi albo nie ma klucza API, tworzy własny hotspot (`Meteo-Setup`). Sieć Wi-Fi, klucz OpenWeatherMap i (opcjonalnie) miasto ustawia się z przeglądarki w telefonie.
* 💾 **Pamięć nieulotna:** klucz API i miasto zapisywane są w pamięci ESP32 (NVS) i przetrwają odłączenie zasilania.
* 🌍 **Auto-geolokalizacja:** bez podanego miasta stacja wykrywa współrzędne na podstawie adresu IP (`ip-api.com`) i pobiera pogodę dla tego miejsca.
* ☁️ **Pogoda na żywo:** temperatura, wilgotność, wiatr i stan nieba z **OpenWeatherMap**, pobierane po HTTPS.
* 📺 **Ekran OLED:** czytelny interfejs na SSD1306 (I2C), polskie nazwy miast wyświetlane bez ogonków.
* 🔘 **Zmiana ustawień przyciskiem:** przytrzymanie przycisku **BOOT** przez 3 sekundy uruchamia tryb konfiguracji.
* 🛡️ **Obsługa błędów:** zły klucz API, nieznane miasto i brak Wi-Fi są pokazywane na ekranie, a po błędzie stacja ponawia próbę po 30 sekundach.
* 🔄 **Oszczędne odpytywanie:** dane odświeżane co 10 minut - tak często, jak aktualizuje je OpenWeatherMap.

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

Wersje są przypięte w `platformio.ini`, PlatformIO pobierze je automatycznie:
* `tzapu/WiFiManager` 2.0.17
* `bblanchon/ArduinoJson` 7.4.2
* `adafruit/Adafruit SSD1306` 2.5.17
* `adafruit/Adafruit GFX Library` 1.13.0

## 🚀 Instrukcja uruchomienia

### 1. Przygotowanie środowiska
1. Zainstaluj **Visual Studio Code** z rozszerzeniem **PlatformIO IDE**.
2. Sklonuj to repozytorium i otwórz folder w PlatformIO.
3. Załóż darmowe konto na [OpenWeatherMap](https://openweathermap.org/) i w zakładce *API keys* skopiuj swój klucz. Nowy klucz zaczyna działać po kilkudziesięciu minutach.

### 2. Wgranie programu
Podłącz ESP32 przez USB i kliknij **Upload** (strzałka na dolnym pasku PlatformIO) albo w terminalu:

```bash
pio run -t upload
pio device monitor
```

Gotowy plik `firmware.bin` można też pobrać z zakładki **Actions** (artefakt `firmware` z ostatniej kompilacji).

### 3. Konfiguracja przez telefon
1. Po pierwszym uruchomieniu ekran pokaże komunikat **Tryb konfiguracji**.
2. Połącz się telefonem z siecią Wi-Fi **`Meteo-Setup`** - strona konfiguracji otworzy się sama (albo wejdź na `192.168.4.1`).
3. Wybierz *Configure WiFi*, wskaż swoją sieć i wpisz hasło.
4. Wklej **klucz API OpenWeatherMap**. Pole **Miasto** zostaw puste, żeby lokalizacja była wykrywana automatycznie, albo wpisz np. `Krakow`.
5. Kliknij **Save** - stacja uruchomi się ponownie i po chwili pokaże pogodę.

### Zmiana ustawień
Przytrzymaj przycisk **BOOT** na płytce przez 3 sekundy - stacja wejdzie w tryb konfiguracji i znów udostępni sieć `Meteo-Setup`. Jeśli nic nie zostanie zapisane w ciągu 5 minut, urządzenie wróci do normalnej pracy.

## 📝 Licencja
Projekt udostępniony na licencji [MIT](LICENSE). Możesz go dowolnie modyfikować i używać w swoich rozwiązaniach domowych.
