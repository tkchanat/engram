#define ENGRAM_DEFINE_STD
#include <engram.hpp>
#include <assert.h>
#include <iostream>
#include <codecvt>

struct Strings {
  std::string string;
  std::wstring wstring;
  std::u16string u16string;
  std::u32string u32string;
  template<typename Engram>
  void serialize(Engram& engram, uint32_t version) const {
    engram << string << wstring << u16string << u32string;
  }
  template<typename Engram>
  void deserialize(Engram& engram, uint32_t version) {
    engram >> string >> wstring >> u16string >> u32string;
  }
  bool operator==(const Strings& other) const {
    if (string    != other.string)    { printf("string roundtrip failed\n");  return false; }
    if (wstring   != other.wstring)   { printf("wstring roundtrip failed\n"); return false; }
    if (u16string != other.u16string) { printf("u16string roundtrip failed\n"); return false; }
    if (u32string != other.u32string) { printf("u32string roundtrip failed\n"); return false; }
    return true;
  }
};

struct Containers {
  std::array<char, 4> array;
  std::vector<int> vector;
  std::unordered_map<int8_t, std::string> unordered_map;
  template<typename Engram>
  void serialize(Engram& engram, uint32_t version) const {
    engram << array << vector << unordered_map;
  }
  template<typename Engram>
  void deserialize(Engram& engram, uint32_t version) {
    engram >> array >> vector >> unordered_map;
  }
  bool operator==(const Containers& other) const {
    if (array  != other.array)  { printf("array roundtrip failed\n"); return false; }
    if (vector != other.vector) { printf("vector roundtrip failed\n");  return false; }
    if (unordered_map != other.unordered_map) { printf("unordered_map roundtrip failed\n");  return false; }
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
  // Strings
  Strings strings = { "Hello", L"你好", u"こんにちは", U"\U0001F642" };
  assert(roundtrip(strings));
  // std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
  // std::cout << converter.to_bytes(strings.u32string) << std::endl;

  // Containers
  Containers containers = {
    { 'a', 'b', 'c', 'd' },
    { 1, 2, 3, 4 },
    { { 1, "One" }, { 2, "Two" } },
  };
  assert(roundtrip(containers));

  // Optional
  std::optional<double> optional = { 4096.0 };
  assert(roundtrip(optional));

  printf("Tests Completed\n");
  return 0;
}