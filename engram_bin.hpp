#pragma once
#include <array>
#include <istream>
#include <functional>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <streambuf>
#include <iomanip>

namespace engram {

  typedef std::basic_string<std::byte> bytestr;
  typedef std::basic_stringbuf<std::byte> bytebuf;
  typedef std::basic_istream<std::byte> ibytestream;
  typedef std::basic_ostream<std::byte> obytestream;
  typedef uint32_t type_id;

  template <typename T> struct dependent_false { static constexpr bool value = false; };

  class OBinaryEngram : public obytestream {
    // SFINAE test
    template <typename T>
    struct has_serialize {
      template<typename C> static std::true_type test(decltype(&C::template serialize<OBinaryEngram&, uint32_t>));
      template<typename C> static std::false_type test(...);    
      static constexpr bool value = sizeof(test<T>(nullptr)) == sizeof(std::true_type);
    };

  public:
    OBinaryEngram() : obytestream(&buf) {}
    OBinaryEngram& operator<<(const char* v) {
      size_t len = 0;
      while (v[len] != '\0') ++len;
      *this << len;
      buf.sputn(reinterpret_cast<const std::byte*>(v), len * sizeof(char));
      return *this;
    }
    OBinaryEngram& operator<<(const std::string& v) {
      const size_t len = v.length();
      *this << len;
      buf.sputn(reinterpret_cast<const std::byte*>(&v[0]), len * sizeof(std::string::value_type));
      return *this;
    }
    template<typename Type>
    OBinaryEngram& operator<<(const std::vector<Type>& v) {
      const size_t len = v.size();
      *this << len;
      for (size_t i = 0; i < len; ++i)
        *this << v[i];
      return *this;
    }
    template<typename Type, size_t N>
    OBinaryEngram& operator<<(const std::array<Type, N>& v) {
      for (size_t i = 0; i < N; ++i)
        *this << v[i];
      return *this;
    }
    template<typename Key, typename Value>
    OBinaryEngram& operator<<(const std::unordered_map<Key, Value>& v) {
      const size_t len = v.size();
      *this << len;
      for (const auto& pair : v)
        *this << pair.first << pair.second;
      return *this;
    }
    template<typename Type>
    OBinaryEngram& operator<<(const std::optional<Type>& v) {
      *this << v.has_value();
      if (v.has_value())
        *this << v.value();
      return *this;
    }
    template<typename Type>
    OBinaryEngram& operator<<(const Type& v) {
      if      constexpr (std::is_integral_v<Type>)        return serialize_prim(v);
      else if constexpr (std::is_floating_point_v<Type>)  return serialize_prim(v);
      else if constexpr (std::is_enum_v<Type>)            return serialize_enum(v);
      else if constexpr (has_serialize<Type>::value) { v.serialize(*this, version); return *this; }
      else static_assert(dependent_false<Type>::value, "This type did not overload with operator<<.");
    }
    OBinaryEngram& serialize_bytes(const std::byte* ptr, const size_t size) {
      *this << size;
      if (size > 0)
        buf.sputn(ptr, size);
      return *this;
    }
    friend inline std::ostream& operator<<(std::ostream& os, const OBinaryEngram& engram){
      constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
      const bytestr bytes = engram.buf.str();
      os << std::hex;
      for (size_t i = 0, nbytes = bytes.size(); i < nbytes; i++)
        os << hexmap[char(bytes[i] >> 4)] << hexmap[(char)bytes[i] & 0x0F] << ' ';
      return os << std::dec;
    }

  private:
    template<typename Type>
    OBinaryEngram& serialize_prim(const Type& v) {
      buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(Type));
      return *this;
    }
    template<typename Type>
    OBinaryEngram& serialize_enum(const Type v) {
      buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(std::underlying_type_t<Type>));
      return *this;
    }

  private:
    uint32_t version = 0;
    bytebuf buf;
  };

  class IBinaryEngram : public ibytestream {
    // SFINAE test
    template <typename T>
    struct has_deserialize {
      template<typename C> static std::true_type test(decltype(&C::template deserialize<IBinaryEngram&, uint32_t>));
      template<typename C> static std::false_type test(...);    
      static constexpr bool value = sizeof(test<T>(nullptr)) == sizeof(std::true_type);
    };

  public:
    IBinaryEngram() : ibytestream(&buf) {}

    // Deserialization
    IBinaryEngram& operator>>(std::string& v) {
      size_t len; *this >> len;
      v.resize(len);
      buf.sgetn(reinterpret_cast<std::byte*>(&v[0]), len * sizeof(std::string::value_type));
      return *this;
    }
    template<typename Type>
    IBinaryEngram& operator>>(std::vector<Type>& v) {
      size_t len; *this >> len;
      if (len) {
        v.resize(len);
        if constexpr (std::is_trivially_copyable_v<Type>)
          buf.sgetn(reinterpret_cast<std::byte*>(v.data()), len * sizeof(Type));
        else {
          for (size_t i = 0; i < len; ++i)
            *this >> v[i];
        }
      }
      else
        v.clear();
      return *this;
    }
    template<typename Type, size_t N>
    IBinaryEngram& operator>>(std::array<Type, N>& v) {
      for (size_t i = 0; i < N; ++i)
        *this >> v[i];
      return *this;
    }
    template<typename Key, typename Value>
    IBinaryEngram& operator>>(std::unordered_map<Key, Value>& v) {
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
    template<typename Type>
    IBinaryEngram& operator>>(std::optional<Type>& v) {
      bool has_value; *this >> has_value;
      if (has_value)
        *this >> v.emplace();
      return *this;
    }
    template<typename Type>
    IBinaryEngram& operator>>(Type& v) {
      if constexpr      (std::is_integral_v<Type>)        return deserialize_prim(v);
      else if constexpr (std::is_floating_point_v<Type>)  return deserialize_prim(v);
      else if constexpr (std::is_enum_v<Type>)            return deserialize_enum(v);
      else if constexpr (has_deserialize<Type>::value) { v.deserialize(*this, version); return *this; }
      else static_assert(dependent_false<Type>::value, "This type did not overload with operator<<.");
    }
    IBinaryEngram& deserialize_bytes(std::byte*& ptr, size_t& size) {
      *this >> size;
      if (size > 0) {
        if (ptr) delete[] ptr;
        ptr = new std::byte[size];
        buf.sgetn(ptr, size);
      }
      return *this;
    }

    friend inline std::ostream& operator<<(std::ostream& os, const IBinaryEngram& engram){
      constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
      const bytestr bytes = engram.buf.str();
      os << std::hex;
      for (size_t i = 0, nbytes = bytes.size(); i < nbytes; i++)
        os << hexmap[char(bytes[i] >> 4)] << hexmap[(char)bytes[i] & 0x0F] << ' ';
      return os << std::dec;
    }

  private:
    template<typename Type>
    IBinaryEngram& deserialize_prim(Type& v) {
      buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(Type));
      return *this;
    }
    template<typename Type>
    IBinaryEngram& deserialize_enum(Type& v) {
      buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(std::underlying_type_t<Type>));
      return *this;
    }

  private:
    uint32_t version = 0;
    bytebuf buf;
  };

}  // namespace engram