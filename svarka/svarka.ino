#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <GyverEncoder.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // Адрес I2C дисплея и размер 20x4
Encoder encoder(2, 3, 4);           // Пины CLK, DT, SW

const int relayPins[] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39};
const int relayCount = sizeof(relayPins) / sizeof(relayPins[0]);
int currentRelay = 0;
unsigned long relayDuration = 1000;     // Начальная длительность работы реле в миллисекундах
unsigned long lastRelaySwitchTime = 0;
int timaDelayRelay = 50;                // Задержка изменения времени, начальное значение 50 мс

bool isSettingMode = false;             // Флаг режима настройки
bool isEditingDelay = false;            // Флаг настройки timaDelayRelay или relayDuration

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  encoder.setType(TYPE2); // Установка типа энкодера
  
  for (int i = 0; i < relayCount; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // Все реле выключены
  }

  displayMainInfo();
}

void loop() {
  encoder.tick();
  
  // Обработка долгого нажатия для входа/выхода из режима настройки
  if (encoder.isHold()) {
    isSettingMode = !isSettingMode; // Переключение режима
    lcd.clear();
    if (isSettingMode) {
      lcd.setCursor(0, 0);
      lcd.print("Setting Mode");
      isEditingDelay = false; // Начинаем с настройки relayDuration
    } else {
      displayMainInfo();
    }
    delay(700); // Антидребезг кнопки
  }
  
  if (isSettingMode) {
    handleSettingMode();
  } else {
    handleRelayControl();
  }
}

void handleSettingMode() {
  // Переключение между параметрами настройки по нажатию на кнопку энкодера
  if (encoder.isClick()) {
    isEditingDelay = !isEditingDelay;
    lcd.clear();
  }

  if (isEditingDelay) {
    if (encoder.isRight()) { // Увеличение timaDelayRelay
      timaDelayRelay += 1;
      timaDelayRelay = constrain(timaDelayRelay, 10, 50);
      displayDelaySetting();
    }

    if (encoder.isLeft()) { // Уменьшение timaDelayRelay
      timaDelayRelay -= 1;
      timaDelayRelay = constrain(timaDelayRelay, 10, 50);
      displayDelaySetting();
    }
  } else {
    if (encoder.isRight()) { // Увеличение relayDuration
      relayDuration += timaDelayRelay; // Увеличиваем на timaDelayRelay
      relayDuration = constrain(relayDuration, 100, 2000);
      displayDurationSetting();
    }

    if (encoder.isLeft()) { // Уменьшение relayDuration
      relayDuration -= timaDelayRelay; // Уменьшаем на timaDelayRelay
      relayDuration = constrain(relayDuration, 100, 2000);
      displayDurationSetting();
    }
  }
}

void handleRelayControl() {
  // Управление реле
  if (millis() - lastRelaySwitchTime > relayDuration) {
    digitalWrite(relayPins[currentRelay], HIGH); // Выключаем текущее реле
    currentRelay = (currentRelay + 1) % relayCount; // Переходим к следующему реле
    digitalWrite(relayPins[currentRelay], LOW); // Включаем следующее реле
    lastRelaySwitchTime = millis(); // Сбрасываем таймер
    
    displayRelayInfo();
  }
}

void displayMainInfo() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Relay Controller");
  lcd.setCursor(0, 1);
  lcd.print("Duration: ");
  lcd.print(relayDuration);
  lcd.print(" ms");
  lcd.setCursor(0, 2);
  lcd.print("Relay: ");
  lcd.print(currentRelay + 1);
}

void displayDurationSetting() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Relay Time:");
  lcd.setCursor(0, 1);
  lcd.print(relayDuration);
  lcd.print(" ms");
}

void displayDelaySetting() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Delay Step:");
  lcd.setCursor(0, 1);
  lcd.print(timaDelayRelay);
  lcd.print(" ms");
}

void displayRelayInfo() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Relay #");
  lcd.print(currentRelay + 1);
  lcd.print(" ON");
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(relayDuration);
  lcd.print(" ms");
}
