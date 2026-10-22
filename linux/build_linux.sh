#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

echo "========================================================"
echo " PixelVault - Сборка автономного бинарника для Linux"
echo "========================================================"

# Auto-install build packages on Debian/Ubuntu
if command -v apt-get >/dev/null 2>&1; then
    echo "Проверка пакетов FLTK (libfltk1.3-dev)..."
    dpkg -s libfltk1.3-dev >/dev/null 2>&1 || {
        echo "Установка зависимостей сборки..."
        sudo apt-get update && sudo apt-get install -y libfltk1.3-dev libpng-dev libjpeg-dev zlib1g-dev build-essential
    }
fi

echo "Компиляция pixelvault-linux..."
g++ -std=c++17 -O2 -I../code \
    ../code/crypto.cpp ../code/stego_engine.cpp ../code/image_box.cpp ../code/gui.cpp ../code/main.cpp \
    -o pixelvault-linux \
    -lfltk_images -lfltk -lpng -ljpeg -lz -lX11 -lXext -lXinerama -lXft -lfontconfig -lpthread -ldl

echo "Сборка завершена: ./pixelvault-linux"
