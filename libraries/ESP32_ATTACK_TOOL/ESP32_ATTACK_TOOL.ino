#include "wificonfig.h"
#include "bleconfig.h"
#include "ui.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RST, OLED_CS);

void setup() {
    Serial.begin(115200);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_MID, INPUT_PULLUP);
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);
    delay(1000);
    Serial.println(F("Starting ESP32 Attack Tool..."));
    // Initialize the display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(3, 12);
    display.println(F("ESP32 Attack Tool v1"));
    display.display();
    display.clearDisplay();
    delay(3000);
    display.setCursor(43, 8);
    display.println(F("Code By"));
    display.setCursor(22, 20);
    display.println(F("Xun Anh Nguyen"));
    display.display();
    delay(3000);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Initializing..."));
    display.display();
    digitalWrite(STATUS_LED, HIGH);
    delay(2000);
    digitalWrite(STATUS_LED, LOW);
    drawMainMenu();
}
void loop() {
    uiLoop();
}