CXX = g++
CXXFLAGS = -std=c++17 -O2 -I../code
LDFLAGS = -lfltk_images -lfltk -lpng -ljpeg -lz -lX11 -lXext -lXinerama -lXft -lfontconfig -lpthread -ldl

SRC = ../code/crypto.cpp \
      ../code/stego_engine.cpp \
      ../code/image_box.cpp \
      ../code/gui.cpp \
      ../code/main.cpp

OBJ = $(SRC:.cpp=.o)
TARGET = pixelvault-linux

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
