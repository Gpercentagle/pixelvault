CXX = clang++
CXXFLAGS = -std=c++17 -O2 $(shell fltk-config --cxxflags) -I../code
LDFLAGS = -L/opt/homebrew/lib $(shell fltk-config --ldflags --use-images)

STATIC_FLTK_IMG = /opt/homebrew/Cellar/fltk/1.4.5/lib/libfltk_images.a
STATIC_FLTK     = /opt/homebrew/Cellar/fltk/1.4.5/lib/libfltk.a
STATIC_PNG      = /opt/homebrew/Cellar/libpng/1.6.58/lib/libpng.a
STATIC_JPEG     = /opt/homebrew/Cellar/jpeg-turbo/3.2.0/lib/libjpeg.a
STATIC_FLAGS    = $(STATIC_FLTK_IMG) $(STATIC_FLTK) $(STATIC_PNG) $(STATIC_JPEG) \
                  -lz -pthread -framework Cocoa -weak_framework UniformTypeIdentifiers -weak_framework ScreenCaptureKit

SRC = ../code/crypto.cpp \
      ../code/stego_engine.cpp \
      ../code/image_box.cpp \
      ../code/gui.cpp \
      ../code/main.cpp

OBJ = $(SRC:.cpp=.o)
TARGET = pixelvault_mac

all: app

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(STATIC_FLAGS) -o $@

app: $(TARGET)
	@mkdir -p PixelVault.app/Contents/MacOS
	@mkdir -p PixelVault.app/Contents/Resources
	@cp $(TARGET) PixelVault.app/Contents/MacOS/pixelvault
	@chmod +x PixelVault.app/Contents/MacOS/pixelvault
	@echo "==> Standalone PixelVault.app готов к работе!"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: app
	open PixelVault.app

.PHONY: all app clean run
