#!/usr/bin/env bash
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

if [ -f "./pixelvault-linux" ]; then
    ./pixelvault-linux "$@"
else
    echo "Бинарник pixelvault-linux не найден. Запуск сборки..."
    ./build_linux.sh
    ./pixelvault-linux "$@"
fi
