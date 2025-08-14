
# 🛠️ Инструкция по сборке

## 1. Прошивка Arduino
- Откройте `VendingMain.ino` в Arduino IDE
- Выберите: Инструменты → Плата → Arduino Uno
- Загрузите код

## 2. Подключение
- LCD: SDA → A4, SCL → A5
- Кнопки: D6–D10 → товары
- Двигатели: D8–D12 → через транзисторы

## 3. Запуск сервера
bash
cd cloud_backend
npm install
node server.js

Откройте: `http://localhost:3000`

## 4. Веб-панель
Положите `web_admin/` в папку сервера
