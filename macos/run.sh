#!/usr/bin/env bash
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

if [ -d "PixelVault.app" ]; then
    echo "Запуск PixelVault.app..."
    open PixelVault.app
elif [ -f "pixelvault_mac" ]; then
    ./pixelvault_mac
else
    echo "Сборка приложения..."
    make app
    open PixelVault.app
fi
