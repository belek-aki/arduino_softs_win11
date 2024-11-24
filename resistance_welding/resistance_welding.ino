#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <GyverEncoder.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // Адрес I2C дисплея и размер 20x4
Encoder encoder(2, 3, 4);           // Пины CLK, DT, SW

const int relayPins[] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33};
const int relayCount = sizeof(relayPins) / sizeof(relayPins[0]);
int currentRelay = 0;
unsigned long relayDuration = 1000; // Время работы реле
int timaDelayRelay = 50;            // Задержка изменения времени
unsigned long lastRelaySwitchTime = 0;

int menuPos = 0;                    // Текущая позиция в меню
bool editMode = false;              // Режим редактирования (false — навигация, true — изменение)

// Значения для отображения в меню
const char* menuItems[] = {
  "Relay Time:",
  "Delay Step:"
};
const int menuCount = 2;

void setup() {
  Serial.begin(9600);
  encoder.setType(TYPE2); // Установка типа энкодера
  
  lcd.init();
  lcd.backlight();

  for (int i = 0; i < relayCount; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // Все реле выключены
  }

  displayMenu(true); // Первичная отрисовка меню
}

void loop() {
  encoder.tick();

  if (encoder.isClick()) {
    editMode = !editMode; // Переключение между навигацией и редактированием
    displayMenu(true);    // Полная перерисовка меню
  }

  if (editMode) {
    handleEditMode();
  } else {
    handleNavigationMode();
  }

  handleRelayControl();
}

void handleEditMode() {
  int increment = 0;
  if (encoder.isRight()) increment = 1;
  if (encoder.isLeft()) increment = -1;

  if (menuPos == 0) {
    // Изменение relayDuration
    relayDuration += increment * timaDelayRelay;
    relayDuration = constrain(relayDuration, 100, 2000);
  } else if (menuPos == 1) {
    // Изменение timaDelayRelay
    timaDelayRelay += increment;
    timaDelayRelay = constrain(timaDelayRelay, 10, 50);
  }

  displayMenu(false); // Обновляем только измененные значения
}

void handleNavigationMode() {
  if (encoder.isRight()) menuPos++;
  if (encoder.isLeft()) menuPos--;

  menuPos = constrain(menuPos, 0, menuCount - 1);
  displayMenu(false); // Обновляем только курсор
}

void handleRelayControl() {
  if (millis() - lastRelaySwitchTime > relayDuration) {
    digitalWrite(relayPins[currentRelay], HIGH); // Выключаем текущее реле
    currentRelay = (currentRelay + 1) % relayCount; // Переходим к следующему реле
    digitalWrite(relayPins[currentRelay], LOW); // Включаем следующее реле
    lastRelaySwitchTime = millis(); // Сбрасываем таймер
    displayRelayInfo(); // Обновляем информацию о текущем реле
  }
}

void displayMenu(bool fullUpdate) {
  static int lastMenuPos = -1;
  static bool lastEditMode = false;
  static unsigned long lastRelayDuration = 0;
  static int lastTimaDelayRelay = 0;
  static int lastRelay = -1;

  if (fullUpdate || lastMenuPos != menuPos) {
    // Обновляем стрелку
    if (!fullUpdate) {
      lcd.setCursor(19, lastMenuPos);
      lcd.print(" ");
    }
    lcd.setCursor(19, menuPos);
    lcd.write(126);
    lastMenuPos = menuPos;
  }

  if (fullUpdate || lastRelayDuration != relayDuration) {
    lcd.setCursor(0, 0);
    lcd.print(menuItems[0]);
    lcd.setCursor(12, 0);
    lcd.print(relayDuration);
    lcd.print(" ms ");
    lastRelayDuration = relayDuration;
  }

  if (fullUpdate || lastTimaDelayRelay != timaDelayRelay) {
    lcd.setCursor(0, 1);
    lcd.print(menuItems[1]);
    lcd.setCursor(12, 1);
    lcd.print(timaDelayRelay);
    lcd.print(" ms ");
    lastTimaDelayRelay = timaDelayRelay;
  }

  if (fullUpdate || lastRelay != currentRelay) {
    lcd.setCursor(0, 2);
    lcd.print("Relay #");
    lcd.print(currentRelay + 1);
    lcd.print(" ON ");
    lastRelay = currentRelay;
  }

  if (fullUpdate || lastEditMode != editMode) {
    lcd.setCursor(0, 3);
    lcd.print(editMode ? "Editing...         " : "Navigation         ");
    lastEditMode = editMode;
  }
}

void displayRelayInfo() {
  lcd.setCursor(0, 2);
  lcd.print("Relay #");
  lcd.print(currentRelay + 1);
  lcd.print(" ON ");
}
