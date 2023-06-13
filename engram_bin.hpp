#pragma once
#include <sstream>
#include <array>
#include <vector>

namespace engram {

typedef std::basic_stringbuf<std::byte> bytebuf;
typedef std::basic_iostream<std::byte> bytestream;

class BinaryEngram : bytestream {
 public:
  BinaryEngram() : bytestream(&buf) {}

  // Input
  BinaryEngram& operator>>(bool& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(bool)); return *this; }
  BinaryEngram& operator>>(float& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(float)); return *this; }
  BinaryEngram& operator>>(double& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(double)); return *this; }
  BinaryEngram& operator>>(int8_t& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(int8_t)); return *this; }
  BinaryEngram& operator>>(int16_t& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(int16_t)); return *this; }
  BinaryEngram& operator>>(int32_t& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(int32_t)); return *this; }
  BinaryEngram& operator>>(int64_t& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(int64_t)); return *this; }
  BinaryEngram& operator>>(uint8_t& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(uint8_t)); return *this; }
  BinaryEngram& operator>>(uint16_t& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(uint16_t)); return *this; }
  BinaryEngram& operator>>(uint32_t& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(uint32_t)); return *this; }
  BinaryEngram& operator>>(uint64_t& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(uint64_t)); return *this; }
  BinaryEngram& operator>>(std::string& v) {
    size_t len; *this >> len;
    v.resize(len);
    buf.sgetn(reinterpret_cast<std::byte*>(&v[0]), len * sizeof(std::string::value_type));
    return *this;
  }
  template<typename Type>
  BinaryEngram& operator>>(std::vector<Type>& v) {
    size_t len; *this >> len;
    v.resize(len);
    for (size_t i = 0; i < len; ++i)
      *this >> v[i];
    return *this;
  }
  template<typename Type, size_t N>
  BinaryEngram& operator>>(std::array<Type, N>& v) {
    for (size_t i = 0; i < N; ++i)
      *this >> v[i];
    return *this;
  }

  // Output
  BinaryEngram& operator<<(const bool v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(bool)); return *this; }
  BinaryEngram& operator<<(const float v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(float)); return *this; }
  BinaryEngram& operator<<(const double v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(double)); return *this; }
  BinaryEngram& operator<<(const int8_t v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(int8_t)); return *this; }
  BinaryEngram& operator<<(const int16_t v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(int16_t)); return *this; }
  BinaryEngram& operator<<(const int32_t v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(int32_t)); return *this; }
  BinaryEngram& operator<<(const int64_t v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(int64_t)); return *this; }
  BinaryEngram& operator<<(const uint8_t v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(uint8_t)); return *this; }
  BinaryEngram& operator<<(const uint16_t v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(uint16_t)); return *this; }
  BinaryEngram& operator<<(const uint32_t v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(uint32_t)); return *this; }
  BinaryEngram& operator<<(const uint64_t v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(uint64_t)); return *this; }
  BinaryEngram& operator<<(const std::string& v) {
    const size_t len = v.length();
    *this << len;
    buf.sputn(reinterpret_cast<const std::byte*>(&v[0]), len * sizeof(std::string::value_type));
    return *this;
  }
  template<typename Type>
  BinaryEngram& operator<<(const std::vector<Type>& v) {
    const size_t len = v.size();
    *this << len;
    for (size_t i = 0; i < len; ++i)
      *this << v[i];
    return *this;
  }
  template<typename Type, size_t N>
  BinaryEngram& operator<<(const std::array<Type, N>& v) {
    for (size_t i = 0; i < N; ++i)
      *this << v[i];
    return *this;
  }

 private:
  bytebuf buf;
};

}  // namespace engram