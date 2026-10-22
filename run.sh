#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

OS="$(uname -s)"
case "$OS" in
    Darwin*)
        echo "Обнаружена macOS. Переход в папку macos/..."
        exec "$DIR/macos/run.sh" "$@"
        ;;
    Linux*)
        echo "Обнаружен Linux. Переход в папку linux/..."
        exec "$DIR/linux/run.sh" "$@"
        ;;
    CYGWIN*|MINGW*|MSYS*)
        echo "Обнаружен Windows. Переход в папку windows/..."
        exec "$DIR/windows/run.bat" "$@"
        ;;
    *)
        echo "Неизвестная ОС: $OS. Пожалуйста, откройте соответствующую папку: macos/, linux/ или windows/."
        exit 1
        ;;
esac
