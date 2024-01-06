#include <engram.hpp>
#include <assert.h>
#include <iostream>

struct Integers {
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;
  uint64_t u64;
  template<typename Engram>
  void serialize(Engram& engram, uint32_t version) const {
    engram << u8 << u16 << u32 << u64;
  }
  template<typename Engram>
  void deserialize(Engram& engram, uint32_t version) {
    engram >> u8 >> u16 >> u32 >> u64;
  }
  bool operator==(const Integers& other) const {
    if (u8  != other.u8)  { printf("uint8_t roundtrip failed\n");  return false; }
    if (u16 != other.u16) { printf("uint16_t roundtrip failed\n"); return false; }
    if (u32 != other.u32) { printf("uint32_t roundtrip failed\n"); return false; }
    if (u64 != other.u64) { printf("uint64_t roundtrip failed\n"); return false; }
    return true;
  }
};

struct FloatingPoint {
  float f32;
  double f64;
  template<typename Engram>
  void serialize(Engram& engram, uint32_t version) const {
    engram << f32 << f64;
  }
  template<typename Engram>
  void deserialize(Engram& engram, uint32_t version) {
    engram >> f32 >> f64;
  }
  bool operator==(const FloatingPoint& other) const {
    if (f32 != other.f32) { printf("float roundtrip failed\n"); return false; }
    if (f64 != other.f64) { printf("double roundtrip failed\n"); return false; }
    return true;
  }
};

template<typename T>
bool roundtrip(const T& obj) {
  // Serialize
  engram::Engram engram;
  engram << obj;
  std::cout << engram << std::endl;

  // Deserialize
  T deserialized_obj;
  engram >> deserialized_obj;
  return deserialized_obj == obj;
}

int main() {
  // Boolean
  bool boolean = true;
  assert(roundtrip(boolean));

  // Integers
  Integers integers = { 1, 2, 4, 8 };
  assert(roundtrip(integers));

  // Floating-point
  FloatingPoint floating_points = { 123.f, 4.56 };
  assert(roundtrip(floating_points));

  // Enumerators
  enum class Enumerators { None, A, B };
  Enumerators enumerators = Enumerators::A;
  assert(roundtrip(enumerators));

  printf("Tests Completed\n");
  return 0;
}