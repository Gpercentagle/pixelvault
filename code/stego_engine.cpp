#include "stego_engine.hpp"
#include "crypto.hpp"
#include <cstring>
#include <cmath>

namespace {

// Helper bit-writer into RGB(A) pixel buffer
class PixelBitWriter {
public:
    PixelBitWriter(uint8_t *pixels, size_t totalPixels, int channels)
        : m_pixels(pixels), m_totalPixels(totalPixels), m_channels(channels),
          m_pixelIdx(0), m_channelIdx(0) {}

    bool writeBit(uint8_t bit) {
        if (m_pixelIdx >= m_totalPixels) return false;
        
        size_t byteOffset = m_pixelIdx * m_channels + m_channelIdx;
        m_pixels[byteOffset] = (m_pixels[byteOffset] & 0xFE) | (bit & 0x01);

        m_channelIdx++;
        if (m_channelIdx >= 3 || m_channelIdx >= m_channels) {
            m_channelIdx = 0;
            m_pixelIdx++;
        }
        return true;
    }

    bool writeByte(uint8_t byte) {
        for (int i = 7; i >= 0; --i) {
            if (!writeBit((byte >> i) & 1)) return false;
        }
        return true;
    }

    bool writeBytes(const uint8_t *data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            if (!writeByte(data[i])) return false;
        }
        return true;
    }

private:
    uint8_t *m_pixels;
    size_t m_totalPixels;
    int m_channels;
    size_t m_pixelIdx;
    int m_channelIdx;
};

// Helper bit-reader from RGB(A) pixel buffer
class PixelBitReader {
public:
    PixelBitReader(const uint8_t *pixels, size_t totalPixels, int channels)
        : m_pixels(pixels), m_totalPixels(totalPixels), m_channels(channels),
          m_pixelIdx(0), m_channelIdx(0) {}

    bool readBit(uint8_t &outBit) {
        if (m_pixelIdx >= m_totalPixels) return false;

        size_t byteOffset = m_pixelIdx * m_channels + m_channelIdx;
        outBit = m_pixels[byteOffset] & 0x01;

        m_channelIdx++;
        if (m_channelIdx >= 3 || m_channelIdx >= m_channels) {
            m_channelIdx = 0;
            m_pixelIdx++;
        }
        return true;
    }

    bool readByte(uint8_t &outByte) {
        outByte = 0;
        for (int i = 7; i >= 0; --i) {
            uint8_t bit = 0;
            if (!readBit(bit)) return false;
            outByte |= (bit << i);
        }
        return true;
    }

    bool readBytes(uint8_t *dst, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            if (!readByte(dst[i])) return false;
        }
        return true;
    }

private:
    const uint8_t *m_pixels;
    size_t m_totalPixels;
    int m_channels;
    size_t m_pixelIdx;
    int m_channelIdx;
};

} // anonymous namespace

size_t StegoEngine::getCapacityBytes(int width, int height, int channels) {
    if (width <= 0 || height <= 0 || channels < 3) return 0;
    // We only use 3 color channels (R, G, B) even if Alpha exists
    size_t usableChannels = 3;
    size_t totalBits = static_cast<size_t>(width) * height * usableChannels;
    size_t totalBytes = totalBits / 8;

    // Minimum header overhead:
    // 4(Magic) + 1(Ver) + 1(Flags) + 2(FileLen) + 4(Size) + 4(CRC) = 16 bytes
    if (totalBytes <= 32) return 0;
    return totalBytes - 32;
}

bool StegoEngine::embed(uint8_t *pixels, int width, int height, int channels,
                        const StegoPayload &payload,
                        const std::string &password,
                        std::string &errorMsg) {
    if (!pixels || width <= 0 || height <= 0 || channels < 3) {
        errorMsg = "Некорректные параметры изображения (требуется RGB или RGBA).";
        return false;
    }

    size_t filenameLen = payload.filename.size();
    if (filenameLen > 65535) {
        errorMsg = "Слишком длинное имя файла.";
        return false;
    }

    size_t payloadSize = payload.data.size();
    if (payloadSize == 0) {
        errorMsg = "Нет данных для внедрения (пустой текст или файл).";
        return false;
    }

    size_t headerSize = 4 + 1 + 1 + 2 + filenameLen + 4 + 4;
    size_t totalRequiredBytes = headerSize + payloadSize;
    size_t totalBits = static_cast<size_t>(width) * height * 3;
    size_t capacityBytes = totalBits / 8;

    if (totalRequiredBytes > capacityBytes) {
        errorMsg = "Данные не помещаются в изображение! Требуется " +
                   std::to_string(totalRequiredBytes) + " байт, доступно " +
                   std::to_string(capacityBytes) + " байт.";
        return false;
    }

    uint8_t flags = 0;
    if (!password.empty()) {
        flags |= FLAG_ENCRYPTED;
    }
    if (payload.isFile) {
        flags |= FLAG_IS_FILE;
    }

    // Compute CRC32 of plaintext payload
    uint32_t payloadCrc = crypto::crc32(payload.data.data(), payload.data.size());

    // Prepare encrypted buffer (CRC + Data)
    std::vector<uint8_t> secureBlock(4 + payloadSize);
    secureBlock[0] = static_cast<uint8_t>((payloadCrc >> 24) & 0xFF);
    secureBlock[1] = static_cast<uint8_t>((payloadCrc >> 16) & 0xFF);
    secureBlock[2] = static_cast<uint8_t>((payloadCrc >> 8) & 0xFF);
    secureBlock[3] = static_cast<uint8_t>(payloadCrc & 0xFF);
    std::memcpy(secureBlock.data() + 4, payload.data.data(), payloadSize);

    if (!password.empty()) {
        crypto::crypt_stream(secureBlock.data(), secureBlock.size(), password);
    }

    // Serialize stream to embed
    std::vector<uint8_t> stream;
    stream.reserve(totalRequiredBytes);

    // 1. Magic "PVLT"
    stream.push_back(MAGIC[0]);
    stream.push_back(MAGIC[1]);
    stream.push_back(MAGIC[2]);
    stream.push_back(MAGIC[3]);

    // 2. Version
    stream.push_back(VERSION);

    // 3. Flags
    stream.push_back(flags);

    // 4. Filename length (uint16)
    stream.push_back(static_cast<uint8_t>((filenameLen >> 8) & 0xFF));
    stream.push_back(static_cast<uint8_t>(filenameLen & 0xFF));

    // 5. Filename
    for (char c : payload.filename) {
        stream.push_back(static_cast<uint8_t>(c));
    }

    // 6. Payload size (uint32)
    stream.push_back(static_cast<uint8_t>((payloadSize >> 24) & 0xFF));
    stream.push_back(static_cast<uint8_t>((payloadSize >> 16) & 0xFF));
    stream.push_back(static_cast<uint8_t>((payloadSize >> 8) & 0xFF));
    stream.push_back(static_cast<uint8_t>(payloadSize & 0xFF));

    // 7. Secure block (CRC + Data, encrypted if password given)
    stream.insert(stream.end(), secureBlock.begin(), secureBlock.end());

    // Write bits to pixels
    size_t totalPixels = static_cast<size_t>(width) * height;
    PixelBitWriter writer(pixels, totalPixels, channels);
    if (!writer.writeBytes(stream.data(), stream.size())) {
        errorMsg = "Ошибка записи битов в буфер изображения.";
        return false;
    }

    return true;
}

bool StegoEngine::extract(const uint8_t *pixels, int width, int height, int channels,
                          const std::string &password,
                          StegoPayload &outPayload,
                          std::string &errorMsg) {
    if (!pixels || width <= 0 || height <= 0 || channels < 3) {
        errorMsg = "Некорректное изображение.";
        return false;
    }

    size_t totalPixels = static_cast<size_t>(width) * height;
    PixelBitReader reader(pixels, totalPixels, channels);

    // 1. Magic
    uint8_t magic[4];
    if (!reader.readBytes(magic, 4)) {
        errorMsg = "Не удалось прочитать заголовок изображения.";
        return false;
    }
    if (std::memcmp(magic, MAGIC, 4) != 0) {
        errorMsg = "Скрытые данные PixelVault не обнаружены в этом изображении.";
        return false;
    }

    // 2. Version
    uint8_t version = 0;
    if (!reader.readByte(version)) {
        errorMsg = "Ошибка чтения версии формата.";
        return false;
    }
    if (version != VERSION) {
        errorMsg = "Неподдерживаемая версия формата: " + std::to_string(version);
        return false;
    }

    // 3. Flags
    uint8_t flags = 0;
    if (!reader.readByte(flags)) {
        errorMsg = "Ошибка чтения флагов.";
        return false;
    }
    bool isEncrypted = (flags & FLAG_ENCRYPTED) != 0;
    outPayload.isFile = (flags & FLAG_IS_FILE) != 0;

    // 4. Filename length
    uint8_t fnLenHigh = 0, fnLenLow = 0;
    if (!reader.readByte(fnLenHigh) || !reader.readByte(fnLenLow)) {
        errorMsg = "Ошибка чтения длины имени файла.";
        return false;
    }
    uint16_t filenameLen = (static_cast<uint16_t>(fnLenHigh) << 8) | fnLenLow;

    // 5. Filename
    outPayload.filename.clear();
    if (filenameLen > 0) {
        std::vector<uint8_t> fnBytes(filenameLen);
        if (!reader.readBytes(fnBytes.data(), filenameLen)) {
            errorMsg = "Ошибка чтения имени файла.";
            return false;
        }
        outPayload.filename.assign(reinterpret_cast<char*>(fnBytes.data()), filenameLen);
    }

    // 6. Payload size
    uint8_t szBytes[4];
    if (!reader.readBytes(szBytes, 4)) {
        errorMsg = "Ошибка чтения размера данных.";
        return false;
    }
    uint32_t payloadSize = (static_cast<uint32_t>(szBytes[0]) << 24) |
                           (static_cast<uint32_t>(szBytes[1]) << 16) |
                           (static_cast<uint32_t>(szBytes[2]) << 8) |
                           static_cast<uint32_t>(szBytes[3]);

    size_t capacity = (totalPixels * 3) / 8;
    if (payloadSize > capacity) {
        errorMsg = "Поврежденные данные (указанный размер превышает объем картинки).";
        return false;
    }

    if (isEncrypted && password.empty()) {
        errorMsg = "Данные зашифрованы! Пожалуйста, укажите пароль.";
        return false;
    }

    // 7. Secure block (4 bytes CRC + payloadSize bytes Data)
    std::vector<uint8_t> secureBlock(4 + payloadSize);
    if (!reader.readBytes(secureBlock.data(), secureBlock.size())) {
        errorMsg = "Ошибка чтения блока полезной нагрузки.";
        return false;
    }

    if (isEncrypted) {
        crypto::crypt_stream(secureBlock.data(), secureBlock.size(), password);
    }

    // Verify CRC32
    uint32_t expectedCrc = (static_cast<uint32_t>(secureBlock[0]) << 24) |
                           (static_cast<uint32_t>(secureBlock[1]) << 16) |
                           (static_cast<uint32_t>(secureBlock[2]) << 8) |
                           static_cast<uint32_t>(secureBlock[3]);

    const uint8_t *extractedData = secureBlock.data() + 4;
    uint32_t actualCrc = crypto::crc32(extractedData, payloadSize);

    if (actualCrc != expectedCrc) {
        if (isEncrypted) {
            errorMsg = "Неверный пароль или данные повреждены!";
        } else {
            errorMsg = "Контрольная сумма CRC-32 не совпадает: данные повреждены.";
        }
        return false;
    }

    outPayload.data.assign(extractedData, extractedData + payloadSize);
    return true;
}

void StegoEngine::extractBitPlane(const uint8_t *src, uint8_t *dst,
                                  int width, int height, int channels,
                                  int bitIndex) {
    if (!src || !dst || width <= 0 || height <= 0 || channels < 1) return;
    size_t totalBytes = static_cast<size_t>(width) * height * channels;
    uint8_t mask = 1 << (bitIndex & 7);

    for (size_t i = 0; i < totalBytes; ++i) {
        // If it's an alpha channel in RGBA, keep it 255
        if (channels == 4 && (i % 4 == 3)) {
            dst[i] = 255;
        } else {
            // Amplify LSB to 0 or 255
            dst[i] = (src[i] & mask) ? 255 : 0;
        }
    }
}

void StegoEngine::generateDiffMap(const uint8_t *orig, const uint8_t *stego, uint8_t *dst,
                                  int width, int height, int channels) {
    if (!orig || !stego || !dst || width <= 0 || height <= 0 || channels < 3) return;
    size_t numPixels = static_cast<size_t>(width) * height;

    for (size_t p = 0; p < numPixels; ++p) {
        size_t offset = p * channels;
        bool diff = false;
        for (int c = 0; c < 3; ++c) {
            if (orig[offset + c] != stego[offset + c]) {
                diff = true;
                break;
            }
        }
        if (diff) {
            // Highlight altered pixel in bright fluorescent Red/Magenta
            dst[offset + 0] = 255; // R
            dst[offset + 1] = 0;   // G
            dst[offset + 2] = 120; // B
        } else {
            // Keep greyscale attenuated background
            uint8_t grey = static_cast<uint8_t>((orig[offset + 0] * 299 + orig[offset + 1] * 587 + orig[offset + 2] * 114) / 1000 / 4);
            dst[offset + 0] = grey;
            dst[offset + 1] = grey;
            dst[offset + 2] = grey;
        }
        if (channels == 4) {
            dst[offset + 3] = 255;
        }
    }
}
