# 🛡️ PixelVault (C++17 / FLTK)

> **PixelVault** — кроссплатформенное десктопное приложение для скрытия файлов и текстов внутри изображений (**LSB-стеганография**) со встроенным шифрованием (**SHA-256 + Stream Cipher**) и аппаратной верификацией целостности (**CRC-32**).

---

## 📸 Скриншоты и возможности

1. **Внедрение любых данных (LSB Steganography)**:
   - Прячет как **текстовые послания**, так и **любые бинарные файлы** (`.zip`, `.pdf`, `.docx`, `.mp3`, `.exe` и т.д.) в контейнеры PNG, BMP, JPEG (результат сохраняется в lossless PNG).
   - Расчет емкости контейнера в реальном времени (`(Ширина * Высота * 3) / 8` байт) с визуальной шкалой заполнения.
   - Альфа-канал прозрачности не затрагивается, что исключает визуальные артефакты.

2. **Криптографическая защита**:
   - Генерация 256-битного мастер-ключа из пароля и соли через **SHA-256**.
   - Потоковое шифрование полезной нагрузки в режиме счетчика (CTR).
   - Контрольная сумма **CRC-32 (IEEE 802.3)** гарантирует целостность: при неверном пароле или повреждении файла приложение мгновенно выдаст предупреждение.

3. **Стеганоанализ (Forensics / LSB Viewer)**:
   - Визуализация битовых срезов (Bit Plane Slicing: LSB 0 и LSB 1) позволяет наглядно исследовать шум матрицы и обнаружить скрытые зашифрованные данные.

---

## 📁 Структура проекта

Весь исходный код вынесен в папку `code/`, а скрипты сборки и бинарники разделены по целевым платформам:

```text
pixelvault/
├── code/                     # 💻 Исходный код C++17
│   ├── crypto.hpp / .cpp     # Криптографическое ядро (SHA-256, CRC-32, Stream Cipher)
│   ├── stego_engine.hpp/.cpp # Движок стеганографии и LSB анализа
│   ├── image_box.hpp / .cpp  # FLTK виджет превью с авто-масштабированием
│   ├── gui.hpp / .cpp        # Графический интерфейс и логика вкладок
│   └── main.cpp              # Точка входа программы
│
├── macos/                    # 🍏 macOS
│   ├── PixelVault.app        # Готовое автономное приложение с иконкой
│   ├── pixelvault_mac        # Статический бинарник
│   ├── Makefile              # Скрипт сборки под macOS
│   └── run.sh                # Запуск в 1 клик
│
├── windows/                  # 🪟 Windows
│   ├── build_windows.bat     # Сборка автономного pixelvault.exe (1 клик)
│   ├── run.bat               # Запуск готового EXE
│   └── Makefile              # Сборка через MinGW
│
├── linux/                    # 🐧 Linux
│   ├── build_linux.sh        # Сборка автономного pixelvault-linux (1 клик)
│   ├── run.sh                # Запуск на Linux
│   └── Makefile              # Сборка через GNU make
│
├── run.sh                    # 🚀 Универсальный скрипт: сам определяет ОС и запускает нужную версию
└── Makefile                  # Корневой Makefile
```

---

## 🚀 Полная инструкция по запуску для всех ОС

---

### 🍏 1. macOS

Приложение на macOS собрано **со статической линковкой** библиотек FLTK, libpng и libjpeg. Оно автономно и **не требует установки Homebrew или внешних библиотек** на компьютере пользователя.

#### Быстрый запуск (в 1 клик):
1. Откройте папку `pixelvault/macos/` в Finder.
2. Дважды кликните на **`PixelVault.app`**.

#### Запуск из терминала:
```bash
# Из корня репозитория (авто-определение ОС):
./run.sh

# Либо напрямую:
open macos/PixelVault.app
```

#### Сборка из исходников на Mac (если вносили изменения):
```bash
cd macos
make app       # Создает свежий PixelVault.app
make run       # Собирает и сразу открывает приложение
```

---

### 🪟 2. Windows

На Windows приложение собирается в полностью автономный **`pixelvault.exe`** со статическими библиотеками (`-static -static-libgcc -static-libstdc++ -mwindows`). Ему **не требуются сторонние DLL** для работы.

#### Требования для сборки:
* Установленный компилятор MinGW-w64 (например, через [MSYS2](https://www.msys2.org/) или [w64devkit](https://github.com/skeeto/w64devkit)).
* Если используете MSYS2, установите FLTK одной командой:
  ```bash
  pacman -S mingw-w64-x86_64-fltk mingw-w64-x86_64-toolchain
  ```

#### Сборка и запуск в 1 клик:
1. Зайдите в папку `windows/`.
2. Дважды кликните на **`build_windows.bat`** (скрипт скомпилирует `pixelvault.exe`).
3. Для запуска дважды кликните на **`run.bat`** или `pixelvault.exe`.

#### Сборка через консоль:
```cmd
cd windows
make
make run
```

---

### 🐧 3. Linux (Ubuntu, Debian, Fedora, Arch)

На Linux скрипт сборки автоматически проверяет наличие библиотек и собирает бинарник `pixelvault-linux`.

#### Запуск и автоматическая сборка в 1 клик:
```bash
cd linux
./run.sh
```
*(Скрипт проверит наличие бинарника, при необходимости скомпилирует его и запустит программу)*.

#### Ручная установка библиотек и сборка:
* **Ubuntu / Debian / Linux Mint:**
  ```bash
  sudo apt-get update
  sudo apt-get install -y libfltk1.3-dev libpng-dev libjpeg-dev zlib1g-dev build-essential
  cd linux
  make
  ./pixelvault-linux
  ```
* **Arch Linux / Manjaro:**
  ```bash
  sudo pacman -S fltk libpng libjpeg-turbo zlib base-devel
  cd linux
  make
  ./pixelvault-linux
  ```
* **Fedora / RHEL:**
  ```bash
  sudo dnf install fltk-devel libpng-devel libjpeg-turbo-devel zlib-devel gcc-c++
  cd linux
  make
  ./pixelvault-linux
  ```

---

## 🌐 Автоматическая сборка релизов в облаке (GitHub Actions)

В репозитории настроен файл [`.github/workflows/build_all_os.yml`](.github/workflows/build_all_os.yml).

При отправке кода на GitHub:
1. GitHub автоматически запустит сборщики на серверах **Windows**, **Linux** и **macOS**.
2. В разделе **Actions ➔ Artifacts** появятся готовые архивы для скачивания:
   * 📦 `PixelVault-Windows-x64.zip` — готовый `pixelvault.exe`
   * 📦 `PixelVault-Linux-x86_64.tar.gz` — бинарник для Linux
   * 📦 `PixelVault-macOS.zip` — бандл `PixelVault.app` для Mac

Пользователям вообще ничего не придется компилировать — они смогут просто скачать архив для своей ОС и запустить программу.

---

## 🔍 Спецификация заголовка PixelVault (`PVLT`)

| Поле | Размер | Назначение |
|---|---|---|
| `Magic` | 4 байта | Сигнатура контейнера `'P','V','L','T'` |
| `Version` | 1 байт | Версия формата (`0x01`) |
| `Flags` | 1 байт | `0x01` = Encrypted, `0x02` = IsFile |
| `FilenameLen` | 2 байта | Длина исходного имени файла (uint16 BE) |
| `Filename` | N байт | Исходное имя файла (UTF-8, если внедрен файл) |
| `PayloadSize` | 4 байта | Размер полезной нагрузки в байтах (uint32 BE) |
| `CRC32` | 4 байта | Хеш CRC-32 (зашифрован, если задан пароль) |
| `Payload` | S байт | Данные файла или текст (зашифрованы, если задан пароль) |
