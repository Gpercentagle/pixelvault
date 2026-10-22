#include "crypto.hpp"
#include <cstring>

namespace crypto {

// CRC32 IEEE 802.3
uint32_t crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        uint8_t byte = data[i];
        crc ^= byte;
        for (int j = 0; j < 8; ++j) {
            uint32_t mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}

// SHA-256 implementation
namespace {

inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

inline uint32_t sig0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

inline uint32_t sig1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

inline uint32_t theta0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

inline uint32_t theta1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

} // anonymous namespace

std::vector<uint8_t> sha256(const uint8_t *data, size_t length) {
    uint32_t H[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    uint64_t totalBits = static_cast<uint64_t>(length) * 8;
    size_t paddedLen = ((length + 9 + 63) / 64) * 64;
    std::vector<uint8_t> padded(paddedLen, 0);
    if (length > 0) {
        std::memcpy(padded.data(), data, length);
    }
    padded[length] = 0x80;

    for (int i = 0; i < 8; ++i) {
        padded[paddedLen - 1 - i] = static_cast<uint8_t>(totalBits >> (i * 8));
    }

    for (size_t chunk = 0; chunk < paddedLen; chunk += 64) {
        uint32_t W[64];
        for (int t = 0; t < 16; ++t) {
            size_t idx = chunk + t * 4;
            W[t] = (static_cast<uint32_t>(padded[idx]) << 24) |
                   (static_cast<uint32_t>(padded[idx + 1]) << 16) |
                   (static_cast<uint32_t>(padded[idx + 2]) << 8) |
                   (static_cast<uint32_t>(padded[idx + 3]));
        }
        for (int t = 16; t < 64; ++t) {
            W[t] = theta1(W[t - 2]) + W[t - 7] + theta0(W[t - 15]) + W[t - 16];
        }

        uint32_t a = H[0], b = H[1], c = H[2], d = H[3];
        uint32_t e = H[4], f = H[5], g = H[6], h = H[7];

        for (int t = 0; t < 64; ++t) {
            uint32_t T1 = h + sig1(e) + ch(e, f, g) + K[t] + W[t];
            uint32_t T2 = sig0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        H[0] += a; H[1] += b; H[2] += c; H[3] += d;
        H[4] += e; H[5] += f; H[6] += g; H[7] += h;
    }

    std::vector<uint8_t> hash(32);
    for (int i = 0; i < 8; ++i) {
        hash[i * 4 + 0] = static_cast<uint8_t>(H[i] >> 24);
        hash[i * 4 + 1] = static_cast<uint8_t>(H[i] >> 16);
        hash[i * 4 + 2] = static_cast<uint8_t>(H[i] >> 8);
        hash[i * 4 + 3] = static_cast<uint8_t>(H[i]);
    }
    return hash;
}

std::vector<uint8_t> sha256(const std::string &str) {
    return sha256(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

void crypt_stream(uint8_t *data, size_t length, const std::string &password, uint64_t salt) {
    if (password.empty() || length == 0) return;

    // Master key derived from password + salt
    std::string keyInput = password;
    for (int i = 0; i < 8; ++i) {
        keyInput.push_back(static_cast<char>((salt >> (i * 8)) & 0xFF));
    }
    std::vector<uint8_t> masterKey = sha256(keyInput);

    // Keystream generation using Counter Mode: SHA-256(masterKey || counter)
    uint64_t counter = 0;
    size_t offset = 0;
    while (offset < length) {
        std::vector<uint8_t> blockInput(32 + 8);
        std::memcpy(blockInput.data(), masterKey.data(), 32);
        for (int i = 0; i < 8; ++i) {
            blockInput[32 + i] = static_cast<uint8_t>((counter >> (i * 8)) & 0xFF);
        }
        std::vector<uint8_t> blockKey = sha256(blockInput.data(), blockInput.size());

        size_t chunk = std::min(static_cast<size_t>(32), length - offset);
        for (size_t i = 0; i < chunk; ++i) {
            data[offset + i] ^= blockKey[i];
        }
        offset += chunk;
        counter++;
    }
}

} // namespace crypto
