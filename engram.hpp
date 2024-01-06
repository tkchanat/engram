#pragma once
#include <sstream>
#include <vector>

#ifdef ENGRAM_DEFINE_STD
#define ENGRAM_DEFINE_STD_CONTAINERS
#define ENGRAM_DEFINE_STD_OPTIONAL
#define ENGRAM_DEFINE_STD_STRINGS
#endif

#ifdef ENGRAM_DEFINE_STD_CONTAINERS
#include <array>
#include <unordered_map>
#endif
#ifdef ENGRAM_DEFINE_STD_OPTIONAL
#include <optional>
#endif

namespace engram {

  template <typename T> struct dependent_false { static constexpr bool value = false; };

  class OEngram : public std::ostream {
    // SFINAE test
    template <typename T>
    struct has_serialize {
      template<typename C> static std::true_type test(decltype(&C::template serialize<OEngram&, uint32_t>));
      template<typename C> static std::false_type test(...);
      static constexpr bool value = sizeof(test<T>(nullptr)) == sizeof(std::true_type);
    };

  public:
    OEngram(std::stringbuf* buf) : std::ostream(buf) {}
    OEngram& operator<<(const char* v) {
      size_t len = 0;
      while (v[len] != '\0') ++len;
      *this << len;
      write(reinterpret_cast<const char*>(v), len * sizeof(char));
      return *this;
    }
#ifdef ENGRAM_DEFINE_STD_STRINGS
    OEngram& operator<<(const std::string& v) {
      const size_t len = v.length(); *this << len;
      write(reinterpret_cast<const char*>(&v[0]), len * sizeof(std::string::value_type));
      return *this;
    }
    OEngram& operator<<(const std::wstring& v) {
      const size_t len = v.length(); *this << len;
      write(reinterpret_cast<const char*>(&v[0]), len * sizeof(std::wstring::value_type));
      return *this;
    }
    OEngram& operator<<(const std::u16string& v) {
      const size_t len = v.length(); *this << len;
      write(reinterpret_cast<const char*>(&v[0]), len * sizeof(std::u16string::value_type));
      return *this;
    }
    OEngram& operator<<(const std::u32string& v) {
      const size_t len = v.length(); *this << len;
      write(reinterpret_cast<const char*>(&v[0]), len * sizeof(std::u32string::value_type));
      return *this;
    }
#endif
#ifdef ENGRAM_DEFINE_STD_CONTAINERS
    template<typename Type, size_t N>
    OEngram& operator<<(const std::array<Type, N>& v) {
      for (size_t i = 0; i < N; ++i)
        *this << v[i];
      return *this;
    }
    template<typename Type>
    OEngram& operator<<(const std::vector<Type>& v) {
      const size_t len = v.size();
      *this << len;
      for (size_t i = 0; i < len; ++i)
        *this << v[i];
      return *this;
    }
    template<typename Key, typename Value>
    OEngram& operator<<(const std::unordered_map<Key, Value>& v) {
      const size_t len = v.size();
      *this << len;
      for (const auto& pair : v)
        *this << pair.first << pair.second;
      return *this;
    }
#endif
#ifdef ENGRAM_DEFINE_STD_OPTIONAL
    template<typename Type>
    OEngram& operator<<(const std::optional<Type>& v) {
      *this << v.has_value();
      if (v.has_value())
        *this << v.value();
      return *this;
    }
#endif
    template<typename Type>
    OEngram& operator<<(const Type& v) {
      if      constexpr (std::is_integral_v<Type>)        return serialize_prim(v);
      else if constexpr (std::is_floating_point_v<Type>)  return serialize_prim(v);
      else if constexpr (std::is_enum_v<Type>)            return serialize_enum(v);
      else if constexpr (has_serialize<Type>::value) { v.serialize(*this, version); return *this; }
      else static_assert(dependent_false<Type>::value, "This type did not overload with operator<<.");
    }
    OEngram& serialize_bytes(const std::byte* ptr, const size_t size) {
      *this << size;
      if (size > 0)
        write(reinterpret_cast<const char*>(ptr), size);
      return *this;
    }

  private:
    template<typename Type>
    OEngram& serialize_prim(const Type& v) {
      write(reinterpret_cast<const char*>(&v), sizeof(Type));
      return *this;
    }
    template<typename Type>
    OEngram& serialize_enum(const Type v) {
      write(reinterpret_cast<const char*>(&v), sizeof(std::underlying_type_t<Type>));
      return *this;
    }

  private:
    uint32_t version = 0;
  };

  class IEngram : public std::istream {
    // SFINAE test
    template <typename T>
    struct has_deserialize {
      template<typename C> static std::true_type test(decltype(&C::template deserialize<IEngram&, uint32_t>));
      template<typename C> static std::false_type test(...);
      static constexpr bool value = sizeof(test<T>(nullptr)) == sizeof(std::true_type);
    };

  public:
    IEngram(std::stringbuf* buf) : std::istream(buf) {}

    // Deserialization
#ifdef ENGRAM_DEFINE_STD_STRINGS
    IEngram& operator>>(std::string& v) {
      size_t len; *this >> len; v.resize(len);
      read(reinterpret_cast<char*>(&v[0]), len * sizeof(std::string::value_type));
      return *this;
    }
    IEngram& operator>>(std::wstring& v) {
      size_t len; *this >> len; v.resize(len);
      read(reinterpret_cast<char*>(&v[0]), len * sizeof(std::wstring::value_type));
      return *this;
    }
    IEngram& operator>>(std::u16string& v) {
      size_t len; *this >> len; v.resize(len);
      read(reinterpret_cast<char*>(&v[0]), len * sizeof(std::u16string::value_type));
      return *this;
    }
    IEngram& operator>>(std::u32string& v) {
      size_t len; *this >> len; v.resize(len);
      read(reinterpret_cast<char*>(&v[0]), len * sizeof(std::u32string::value_type));
      return *this;
    }
#endif
#ifdef ENGRAM_DEFINE_STD_CONTAINERS
    template<typename Type, size_t N>
    IEngram& operator>>(std::array<Type, N>& v) {
      for (size_t i = 0; i < N; ++i)
        *this >> v[i];
      return *this;
    }
    template<typename Type>
    IEngram& operator>>(std::vector<Type>& v) {
      size_t len; *this >> len;
      if (len) {
        v.resize(len);
        if constexpr (std::is_trivially_copyable_v<Type>)
          read(reinterpret_cast<char*>(v.data()), len * sizeof(Type));
        else {
          for (size_t i = 0; i < len; ++i)
            *this >> v[i];
        }
      }
      else
        v.clear();
      return *this;
    }
    template<typename Key, typename Value>
    IEngram& operator>>(std::unordered_map<Key, Value>& v) {
      size_t len; *this >> len;
      v.clear();
      v.reserve(len);
      for (size_t i = 0; i < len; ++i) {
        Key key;
        *this >> key;
        *this >> v[key];
      }
      return *this;
    }
#endif
#ifdef ENGRAM_DEFINE_STD_OPTIONAL
    template<typename Type>
    IEngram& operator>>(std::optional<Type>& v) {
      bool has_value; *this >> has_value;
      if (has_value)
        *this >> v.emplace();
      return *this;
    }
#endif
    template<typename Type>
    IEngram& operator>>(Type& v) {
      if constexpr      (std::is_integral_v<Type>)        return deserialize_prim(v);
      else if constexpr (std::is_floating_point_v<Type>)  return deserialize_prim(v);
      else if constexpr (std::is_enum_v<Type>)            return deserialize_enum(v);
      else if constexpr (has_deserialize<Type>::value) { v.deserialize(*this, version); return *this; }
      else static_assert(dependent_false<Type>::value, "This type did not overload with operator<<.");
    }
    IEngram& deserialize_bytes(std::byte*& ptr, size_t& size) {
      *this >> size;
      if (size > 0) {
        if (ptr) delete[] ptr;
        ptr = new std::byte[size];
        read(reinterpret_cast<char*&>(ptr), size);
      }
      return *this;
    }

  private:
    template<typename Type>
    IEngram& deserialize_prim(Type& v) {
      read(reinterpret_cast<char*>(&v), sizeof(Type));
      return *this;
    }
    template<typename Type>
    IEngram& deserialize_enum(Type& v) {
      read(reinterpret_cast<char*>(&v), sizeof(std::underlying_type_t<Type>));
      return *this;
    }

  protected:
    uint32_t version = 0;
  };

  class Engram : public IEngram, public OEngram {
    static constexpr auto openmode = std::ios::in | std::ios::out | std::ios::binary;

  public:
    Engram() : str_buf(openmode), IEngram(&str_buf), OEngram(&str_buf) {}
    std::vector<std::byte> bytes() const {
      const std::string str = str_buf.str();
      std::vector<std::byte> result(str.size());
      std::transform(str.begin(), str.end(), result.begin(), [](char c) { return std::byte(c); });
      return result;
    }
    friend inline std::ostream& operator<<(std::ostream& os, Engram& engram){
      constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
      const std::vector<std::byte> bytes = engram.bytes();
      os << std::hex;
      for (size_t i = 0, nbytes = bytes.size(); i < nbytes; i++)
        os << hexmap[char(bytes[i] >> 4)] << hexmap[(char)bytes[i] & 0x0F] << ' ';
      return os << std::dec;
    }

  private:
    std::stringbuf str_buf;
  };

}  // namespace engram