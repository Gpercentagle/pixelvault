#include <FL/Fl.H>
#include <FL/Fl_Shared_Image.H>
#include "gui.hpp"
#include <iostream>

int main(int argc, char **argv) {
    // Enable system-wide image support in FLTK (PNG, JPEG, BMP)
    fl_register_images();

    // Enable high-DPI scaling / smooth rendering on macOS Retina displays
    Fl::scheme("gtk+");

    MainWindow win(980, 620, "PixelVault — Стеганография & Криптография (C++ / FLTK)");
    win.show(argc, argv);

    return Fl::run();
}
