#ifndef STEGO_ENGINE_HPP
#define STEGO_ENGINE_HPP

#include <string>
#include <vector>
#include <cstdint>

struct StegoPayload {
    bool isFile = false;
    std::string filename;          // Name of the embedded file, empty if plain text
    std::vector<uint8_t> data;     // Raw payload content (text or binary file bytes)
};

class StegoEngine {
public:
    // Magic header identifier "PVLT"
    static constexpr uint8_t MAGIC[4] = {'P', 'V', 'L', 'T'};
    static constexpr uint8_t VERSION = 1;

    static constexpr uint8_t FLAG_ENCRYPTED = 0x01;
    static constexpr uint8_t FLAG_IS_FILE   = 0x02;

    // Maximum safe raw payload capacity in bytes for given image dimensions
    static size_t getCapacityBytes(int width, int height, int channels);

    // Embed payload into image pixels buffer (RGB or RGBA)
    static bool embed(uint8_t *pixels, int width, int height, int channels,
                      const StegoPayload &payload,
                      const std::string &password,
                      std::string &errorMsg);

    // Extract payload from image pixels buffer
    static bool extract(const uint8_t *pixels, int width, int height, int channels,
                        const std::string &password,
                        StegoPayload &outPayload,
                        std::string &errorMsg);

    // Visual analysis: extract bit plane (0 = least significant bit)
    // Amplifies the bit to 0 or 255 for visual inspection of steganographic noise
    static void extractBitPlane(const uint8_t *src, uint8_t *dst,
                                int width, int height, int channels,
                                int bitIndex = 0);

    // Visual analysis: generate diff heatmap between original and stego image
    static void generateDiffMap(const uint8_t *orig, const uint8_t *stego, uint8_t *dst,
                                int width, int height, int channels);
};

#endif // STEGO_ENGINE_HPP
