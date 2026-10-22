@echo off
if exist "pixelvault.exe" (
    start pixelvault.exe
) else (
    echo [ИНФО] pixelvault.exe не найден. Запуск сборки...
    call build_windows.bat
    if exist "pixelvault.exe" start pixelvault.exe
)
