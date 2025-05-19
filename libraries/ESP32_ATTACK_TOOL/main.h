#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "WiFi.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertisedDevice.h>
#include <vector>

#define BTN_LEFT  32
#define BTN_RIGHT 33
#define BTN_MID   25

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_DC 4
#define OLED_CS 5
#define OLED_RST 16

#define STATUS_LED 2

extern Adafruit_SSD1306 display;