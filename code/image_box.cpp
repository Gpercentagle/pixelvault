#include "image_box.hpp"
#include <FL/fl_draw.H>
#include <cstring>
#include <algorithm>

ImageBox::ImageBox(int x, int y, int w, int h, const char *label)
    : Fl_Box(x, y, w, h, label) {
    box(FL_FLAT_BOX);
    color(fl_rgb_color(24, 24, 37));
}

ImageBox::~ImageBox() {
    clearImage();
}

void ImageBox::clearImage() {
    if (m_displayImage) {
        delete m_displayImage;
        m_displayImage = nullptr;
    }
    if (m_rawPixels) {
        delete[] m_rawPixels;
        m_rawPixels = nullptr;
    }
    m_imgW = 0;
    m_imgH = 0;
    m_imgChannels = 0;
    redraw();
}

void ImageBox::setImage(const uint8_t *pixelData, int width, int height, int channels) {
    clearImage();
    if (!pixelData || width <= 0 || height <= 0 || channels < 1) {
        redraw();
        return;
    }

    m_imgW = width;
    m_imgH = height;
    m_imgChannels = channels;

    size_t sz = static_cast<size_t>(width) * height * channels;
    m_rawPixels = new uint8_t[sz];
    std::memcpy(m_rawPixels, pixelData, sz);

    updateDisplayImage();
    redraw();
}

void ImageBox::updateDisplayImage() {
    if (m_displayImage) {
        delete m_displayImage;
        m_displayImage = nullptr;
    }
    if (!m_rawPixels || m_imgW <= 0 || m_imgH <= 0) return;

    int maxW = w() - 8;
    int maxH = h() - 8;
    if (maxW <= 0 || maxH <= 0) return;

    double scale = std::min(static_cast<double>(maxW) / m_imgW,
                            static_cast<double>(maxH) / m_imgH);
    int targetW = std::max(1, static_cast<int>(m_imgW * scale));
    int targetH = std::max(1, static_cast<int>(m_imgH * scale));

    Fl_RGB_Image temp(m_rawPixels, m_imgW, m_imgH, m_imgChannels);
    m_displayImage = dynamic_cast<Fl_RGB_Image*>(temp.copy(targetW, targetH));
}

void ImageBox::draw() {
    // Background card box
    fl_color(fl_rgb_color(24, 24, 37));
    fl_rectf(x(), y(), w(), h());

    // Border
    fl_color(fl_rgb_color(69, 71, 90));
    fl_rect(x(), y(), w(), h());

    if (m_displayImage) {
        int imgX = x() + (w() - m_displayImage->w()) / 2;
        int imgY = y() + (h() - m_displayImage->h()) / 2;
        m_displayImage->draw(imgX, imgY);
    } else {
        // Draw placeholder text
        fl_color(fl_rgb_color(166, 173, 200));
        fl_font(FL_HELVETICA, 14);
        fl_draw(m_placeholder.c_str(), x(), y(), w(), h(), FL_ALIGN_CENTER);
    }
}
