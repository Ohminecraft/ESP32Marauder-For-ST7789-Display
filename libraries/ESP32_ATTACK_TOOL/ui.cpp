// ui.cpp - Giao diện cho ESP32 Attack Tool

#include "ui.h"
#include "wificonfig.h"
#include "bleconfig.h"

extern Adafruit_SSD1306 display;

// ====== UI State ======
int mainMenuIndex = 0;
int maxPage = 4;
bool feature_exit_requested = false

// ====== UI Functions ======

void drawMainMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("ESP32 Attack Tool v1");
  if (mainMenuIndex != 3) {
    display.println(mainMenuIndex == 0 ? "> WiFi Deauther" : "  WiFi Deauther");
    display.println(mainMenuIndex == 1 ? "> WiFi Beacon Spam" : "  WiFi Beacon Spam");
    display.println(mainMenuIndex == 2 ? "> BLE Sour Apple" : "  BLE Sour Apple");
  } else {
    display.println("  WiFi Beacon Spam");
    display.println("  BLE Sour Apple");
    display.println("> BLE Spoofer Devices");
  }

  display.display();
}

bool isButtonPressed(int pin) {
  return !digitalRead(pin);
}

void handleMenuButton() {
  if (isButtonPressed(BTN_LEFT)) {
    mainMenuIndex = (mainMenuIndex - 1 + maxPage) % maxPage;
    drawMainMenu();
  }

  if (isButtonPressed(BTN_RIGHT)) {
    mainMenuIndex = (mainMenuIndex + 1) % maxPage;
    drawMainMenu();
  }

  if (isButtonPressed(BTN_MID)) {
    if (mainMenuIndex == 0) {
      feature_exit_requested = false;
      deautherSetup();
      while (mainMenuIndex == 0 && !feature_exit_requested) {
        mainMenuIndex = 0;
        WifiDeauthhandleButton();
        if (isButtonPressed(BTN_MID)) {
          unsigned long holdStart = millis();
          while (isButtonPressed(BTN_MID)) {
            if (millis() - holdStart > 2000) {
              sourappleUnloop();
              feature_exit_requested = false;
              break;
            }
          }
        }
      }
    }

    if (mainMenuIndex == 2) {
      feature_exit_requested = false;
      sourappleSetup();
      while (mainMenuIndex == 2 && !feature_exit_requested) {
        mainMenuIndex = 2;
        sourappleLoop();
        if (isButtonPressed(BTN_MID)) {
          unsigned long holdStart = millis();
          while (isButtonPressed(BTN_MID)) {
            if (millis() - holdStart > 2000) {
              sourappleUnloop();
              feature_exit_requested = false;
              break;
            }
          }
        }
      }
      return_to_mainmenu();
    }
    if (mainMenuIndex == 3) {
      feature_exit_requested = false;
      spooferdeviceSetup();
      while (mainMenuIndex == 3 && !feature_exit_requested) {
        mainMenuIndex = 3;
        spooferdevicePostActivate();
        feature_exit_requested = false;
      }
      return_to_mainmenu();
    }
  }
}

void uiLoop() {
      case BEACON_WIFI_PAGE:
        beaconSetup();
        beaconwifiRunning = true;
        digitalWrite(STATUS_LED, HIGH);
        drawWifiStatus();
        while (beaconwifiRunning) {
          beaconLoop();
          if (digitalRead(BTN_MID) == LOW) {
              unsigned long holdStart = millis();
              while (digitalRead(BTN_MID) == LOW) {
                if (millis() - holdStart > 2000) {
                  beaconwifiRunning = false;
                  digitalWrite(STATUS_LED, LOW);
                  wifiDeInit();
                  drawWifiStatus();
                  while (digitalRead(BTN_MID) == LOW);
                  currentPage = MAIN_MENU;
                  drawMainMenu();
                  return;
                }
              }
            }
        }
      case DEAUTH_WIFI_PAGE:
        if (!networks.empty()) {
          deautherSetup();
          deauthwifiRunning = true;
          digitalWrite(STATUS_LED, HIGH);
          drawWifiStatus();
          while (deauthwifiRunning) {
            deautherLoop(deauthwifiIndex);
            if (digitalRead(BTN_MID) == LOW) {
              unsigned long holdStart = millis();
              while (digitalRead(BTN_MID) == LOW) {
                if (millis() - holdStart > 2000) {
                  deauthwifiRunning = false;
                  digitalWrite(STATUS_LED, LOW);
                  wifiDeInit();
                  drawWifiStatus();
                  while (digitalRead(BTN_MID) == LOW);
                  currentPage = MAIN_MENU;
                  drawMainMenu();
                  return;
                }
              }
            }
          }
        }
        break;

      default:
        break;

    }
    delay(200);
  }
}


