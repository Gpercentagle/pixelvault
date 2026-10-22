#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace crypto {

// Compute standard CRC-32 checksum
uint32_t crc32(const uint8_t *data, size_t length);

// SHA-256 hash
std::vector<uint8_t> sha256(const uint8_t *data, size_t length);
std::vector<uint8_t> sha256(const std::string &str);

// Fast and secure stream cipher (CTR-mode SHA-256 keystream)
// Encrypts or decrypts in-place with a password
void crypt_stream(uint8_t *data, size_t length, const std::string &password, uint64_t salt = 0x5445474F53544547ULL);

} // namespace crypto

#endif // CRYPTO_HPP
