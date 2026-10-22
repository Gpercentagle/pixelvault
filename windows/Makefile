CXX = g++
CXXFLAGS = -std=c++17 -O2 -I../code
LDFLAGS = -static -static-libgcc -static-libstdc++ \
          -lfltk_images -lfltk -lpng -ljpeg -lz \
          -mwindows -lgdi32 -luser32 -lcomctl32 -lole32 -luuid -lshell32

SRC = ../code/crypto.cpp \
      ../code/stego_engine.cpp \
      ../code/image_box.cpp \
      ../code/gui.cpp \
      ../code/main.cpp

OBJ = $(SRC:.cpp=.o)
TARGET = pixelvault.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(OBJ) $(TARGET)

run: $(TARGET)
	start $(TARGET)

.PHONY: all clean run
