Александр, [14.08.2025 13:17]
/**
 * 🛒 Вендинговый автомат — Основная прошивка
 * Arduino Uno | LCD 16x2 | 5 товаров | Кнопки | Двигатели
 * Поддержка: русский интерфейс, EEPROM, сервисный режим
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// === Дисплей I2C ===
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Проверьте адрес: 0x27 или 0x3F

// === Пины ===
const int buttonPins[5] = {6, 7, 8, 9, 10};     // Кнопки выбора товара
const int motorPins[5] = {8, 9, 10, 11, 12};     // Управление двигателями
const int sensorPins[5] = {13, A0, A1, A2, A3};  // Опто-датчики (LOW = есть товар)
const int passPins[4] = {2, 3, 4, 5};            // Кнопки пароля: 1-2-3-4

// === Переменные ===
int prices[5];
int selectedProduct = -1;
bool serviceMode = false;
int editIndex = 0;
int tempPrice = 0;

// === Пароль: нажать 1→2→3→4 ===
int passInput[4] = {0};
int passPos = 0;
const int password[4] = {1, 2, 3, 4};

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Загрузка...");

  // Загрузка цен из EEPROM
  for (int i = 0; i < 5; i++) {
    EEPROM.get(i * sizeof(int), prices[i]);
    if (prices[i] <= 0 || prices[i] > 200) prices[i] = 50;
  }

  // Настройка пинов
  for (int i = 0; i < 5; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(motorPins[i], OUTPUT);
    digitalWrite(motorPins[i], LOW);
    pinMode(sensorPins[i], INPUT);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(passPins[i], INPUT_PULLUP);
  }

  delay(1000);
  lcd.clear();
  lcd.print("Выберите товар:");
}

void loop() {
  if (serviceMode) {
    handleServiceMode();
  } else {
    handleNormalMode();
    checkPassword();
  }
  delay(50);
}

// === Проверка пароля ===
void checkPassword() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(passPins[i]) == LOW) {
      delay(100);
      passInput[passPos] = i + 1;
      if (passInput[passPos] != password[passPos]) {
        passPos = 0;
      } else {
        passPos++;
      }
      if (passPos == 4) {
        serviceMode = true;
        passPos = 0;
        lcd.clear();
        lcd.print("Режим: Сервис");
        delay(1000);
        editIndex = 0;
        tempPrice = prices[0];
      }
      while (digitalRead(passPins[i]) == LOW) delay(10);
    }
  }
}

// === Выбор товара ===
void handleNormalMode() {
  for (int i = 0; i < 5; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      delay(200);
      if (digitalRead(sensorPins[i]) == HIGH) {
        lcd.clear();
        lcd.print("Нет товара!");
        delay(1500);
        lcd.clear();
        lcd.print("Выберите товар:");
        return;
      }
      selectedProduct = i;
      lcd.clear();
      lcd.print("Товар ");
      lcd.print(i + 1);
      lcd.setCursor(0, 1);
      lcd.print("Цена: ");
      lcd.print(prices[i]);
      lcd.print(" руб");
      waitForPayment();
    }
  }
}

// === Упрощённая оплата (имитация) ===
void waitForPayment() {
  long start = millis();
  while (millis() - start < 30000) {
    if (Serial.available()) {
      String data = Serial.readString();
      if (data.indexOf("PAY") != -1 && data.indexOf(String(selectedProduct)) != -1) {
        dispenseProduct(selectedProduct);
        return;
      }
    }
    delay(100);
  }
  lcd.clear();
  lcd.print("Время истекло!");
  delay(2000);
  resetToMain();
}

// === Выдача товара ===
void dispenseProduct(int idx) {
  lcd.clear();
  lcd.print("Оплата принята!");
  delay(1000);
  lcd.clear();
  lcd.print("Выдача...");
  digitalWrite(motorPins[idx], HIGH);
  delay(2000);
  digitalWrite(motorPins[idx], LOW);
  delay(500);
  lcd.clear();
  lcd.print("Готово!");
  delay(1500);
  resetToMain();
}

// === Сервисный режим ===
void handleServiceMode() {
  if (digitalRead(6) == LOW) { editIndex = (editIndex - 1 + 5) % 5; tempPrice = prices[editIndex]; delay(200); }

Александр, [14.08.2025 13:17]
if (digitalRead(7) == LOW) { editIndex = (editIndex + 1) % 5; tempPrice = prices[editIndex]; delay(200); }
  if (digitalRead(8) == LOW) { tempPrice++; delay(100); }
  if (digitalRead(9) == LOW) { tempPrice = max(0, tempPrice - 1); delay(100); }
  if (digitalRead(10) == LOW) {
    prices[editIndex] = tempPrice;
    EEPROM.put(editIndex * sizeof(int), tempPrice);
    lcd.clear();
    lcd.print("Сохранено!");
    delay(1000);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Тов ");
  lcd.print(editIndex + 1);
  lcd.print(": ");
  lcd.print(tempPrice);
  lcd.print("р");
  lcd.setCursor(0, 1);
  lcd.print("< > + - Enter");
}

// === Возврат в главное меню ===
void resetToMain() {
  lcd.clear();
  lcd.print("Выберите товар:");
  selectedProduct = -1;
}