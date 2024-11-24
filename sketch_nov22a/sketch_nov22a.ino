#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Адрес устройства 0x27

const int trigPin = 3;
const int echoPin = 4;

void setup() {
  lcd.init();
  lcd.backlight();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  lcd.print("load...");
}

void loop() {
  long duration;
  int distance;

  // Генерация импульса
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Считываем время импульса
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Расчет расстояния

  // Вывод данных на дисплей
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("H.: ");
  lcd.print(distance);
  lcd.print(" sm");
  delay(1000);
}
