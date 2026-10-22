cmake_minimum_required(VERSION 3.16)
project(PixelVault VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(FLTK REQUIRED)

include_directories(
    code
    ${FLTK_INCLUDE_DIR}
    /opt/homebrew/include
)

link_directories(
    /opt/homebrew/lib
)

set(SOURCES
    code/crypto.hpp
    code/crypto.cpp
    code/stego_engine.hpp
    code/stego_engine.cpp
    code/image_box.hpp
    code/image_box.cpp
    code/gui.hpp
    code/gui.cpp
    code/main.cpp
)

add_executable(pixelvault ${SOURCES})

target_link_libraries(pixelvault PRIVATE
    ${FLTK_LIBRARIES}
    fltk_images
    png
    jpeg
    z
)
