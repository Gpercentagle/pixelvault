@echo off
echo ========================================================
echo  PixelVault - Сборка автономного EXE для Windows
echo ========================================================
echo.

where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo [ОШИБКА] Компилятор g++ не найден в переменной PATH!
    echo Установите MinGW-w64 (например через MSYS2 или w64devkit).
    pause
    exit /b 1
)

echo Компиляция статичного standalone EXE...
g++ -std=c++17 -O2 -static -static-libgcc -static-libstdc++ ^
    -I..\code ^
    ..\code\crypto.cpp ..\code\stego_engine.cpp ..\code\image_box.cpp ..\code\gui.cpp ..\code\main.cpp ^
    -o pixelvault.exe ^
    -lfltk_images -lfltk -lpng -ljpeg -lz -mwindows -lgdi32 -luser32 -lcomctl32 -lole32 -luuid -lshell32

if %errorlevel% equ 0 (
    echo.
    echo [УСПЕХ] pixelvault.exe успешно скомпилирован!
    echo Он полностью автономен и не требует сторонних DLL.
) else (
    echo.
    echo [ОШИБКА] Сборка завершилась с ошибкой.
)
pause
