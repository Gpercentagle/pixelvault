#ifndef IMAGE_BOX_HPP
#define IMAGE_BOX_HPP

#include <FL/Fl_Box.H>
#include <FL/Fl_RGB_Image.H>
#include <string>

class ImageBox : public Fl_Box {
public:
    ImageBox(int x, int y, int w, int h, const char *label = nullptr);
    ~ImageBox() override;

    void setImage(const uint8_t *pixelData, int width, int height, int channels);
    void clearImage();
    bool hasImage() const { return m_rawPixels != nullptr; }

    const uint8_t* getRawPixels() const { return m_rawPixels; }
    int getImageWidth() const { return m_imgW; }
    int getImageHeight() const { return m_imgH; }
    int getImageChannels() const { return m_imgChannels; }

    void setPlaceholder(const std::string &text) { m_placeholder = text; redraw(); }

    void draw() override;

private:
    uint8_t *m_rawPixels = nullptr;
    int m_imgW = 0;
    int m_imgH = 0;
    int m_imgChannels = 0;

    Fl_RGB_Image *m_displayImage = nullptr;
    std::string m_placeholder = "Нет изображения";

    void updateDisplayImage();
};

#endif // IMAGE_BOX_HPP
