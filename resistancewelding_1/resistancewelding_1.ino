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

bool isManualMode = false;             // Флаг режима (ручной/автоматический)
bool isManualCycle = false;            // Флаг выполнения ручного цикла
bool isSettingMode = false;            // Флаг реresistance weldingжима настройки
bool isEditingDelay = false;           // Флаг настройки timaDelayRelay
bool isEditingDuration = false;        // Флаг настройки relayDuration

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

  // Переключатель между ручным и автоматическим режимами
  if (digitalRead(5) == LOW) { // Например, переключатель на пине 5
    isManualMode = !isManualMode;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(isManualMode ? "Manual Mode" : "Auto Mode");
    delay(700); // Защита от дребезга
    displayMainInfo();
  }

  if (isSettingMode) {
    handleSettingMode();
  } else {
    if (isManualMode) {
      handleManualMode();
    } else {
      handleAutoMode();
    }
  }
}

void handleSettingMode() {
  // Переключение между параметрами настройки
  if (encoder.isClick()) {
    if (isEditingDelay) {
      isEditingDelay = false;
      isEditingDuration = true; // Переход к настройке relayDuration
    } else if (isEditingDuration) {
      isEditingDuration = false;
      isSettingMode = false; // Выход из режима настройки
      displayMainInfo();
    } else {
      isEditingDelay = true; // Переход к настройке timaDelayRelay
    }
    lcd.clear();
  }

  // Настройка timaDelayRelay
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
  }

  // Настройка relayDuration
  if (isEditingDuration) {
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

void handleManualMode() {
  // Обработка ручного цикла переключения реле
  if (encoder.isClick()) { // Если кнопка нажата
    isManualCycle = true;  // Запускаем ручной цикл
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Manual Cycle Start");
    delay(700); // Антидребезг кнопки

    // Ручной цикл переключения реле
    for (int i = 0; i < relayCount; i++) {
      digitalWrite(relayPins[i], LOW); // Включаем реле
      lcd.setCursor(0, 1);
      lcd.print("Relay #");
      lcd.print(i + 1);
      lcd.print(" ON");
      delay(relayDuration);
      digitalWrite(relayPins[i], HIGH); // Выключаем реле
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Manual Cycle Done");
    delay(1000); // Пауза перед возвратом
    displayMainInfo();
    isManualCycle = false; // Завершаем ручной цикл
  }
}

void handleAutoMode() {
  // Автоматический режим
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
  lcd.print(isManualMode ? "Manual Mode" : "Auto Mode");
  lcd.setCursor(0, 1);
  lcd.print("Relay Duration:");
  lcd.setCursor(0, 2);
  lcd.print(relayDuration);
  lcd.print(" ms");
  lcd.setCursor(0, 3);
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
