#pragma once
#include <sstream>
#include <array>
#include <unordered_map>
#include <vector>

namespace engram {

  typedef std::basic_stringbuf<std::byte> bytebuf;
  typedef std::basic_iostream<std::byte> bytestream;

  // SFINAE test
  template <typename T>
  struct has_type_id {
    typedef char one;
    struct two { char x[2]; };
    template<typename C> static one test(decltype(&C::type_id));
    template<typename C> static two test(...);    
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
  };

  class BinaryEngram : bytestream {
  public:
    BinaryEngram() : bytestream(&buf) {}
  
    // Serialization
    BinaryEngram& operator<<(const char v) { buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(char)); return *this; }
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
    BinaryEngram& operator<<(const char* v) {
      size_t len = 0;
      while (v[len] != '\0') ++len;
      *this << len;
      buf.sputn(reinterpret_cast<const std::byte*>(v), len * sizeof(char));
      return *this;
    }
    BinaryEngram& operator<<(const std::string& v) {
      const size_t len = v.length();
      *this << len;
      buf.sputn(reinterpret_cast<const std::byte*>(&v[0]), len * sizeof(std::string::value_type));
      return *this;
    }
    template<typename Type>
    BinaryEngram& operator<<(const Type* v) {
      static_assert(has_type_id<Type>::value, "Base type must implement `const char* type_id() const` as virtual function");
      if (!v) return *this << false; // nullptr bit
      const std::string type_id = v->type_id();
      *this << true << type_id;
      const auto& polymorphic_map = EngramTypeRegistry::instance().ser_map;
      auto it = polymorphic_map.find(type_id);
      if (it == polymorphic_map.end())
        abort(); // Polymorphic type not yet registered with ENGRAM_REGISTER_TYPE()
      const void* ptr = static_cast<const void*>(v);
      it->second(*this, ptr);
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

    // Deserialization
    BinaryEngram& operator>>(char& v) { buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(char)); return *this; }
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
    BinaryEngram& operator>>(Type*& v) {
      static_assert(has_type_id<Type>::value, "Base type must implement `const char* type_id() const` as virtual function");
      bool not_null; *this >> not_null;
      if (!not_null) return *this;
      std::string type_id; *this >> type_id;
      const auto& polymorphic_map = EngramTypeRegistry::instance().de_map;
      auto it = polymorphic_map.find(type_id);
      if (it == polymorphic_map.end())
        abort(); // Polymorphic type not yet registered with ENGRAM_REGISTER_TYPE()
      void* ptr = nullptr;
      it->second(*this, ptr);
      v = static_cast<Type*>(ptr);
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

  private:
    bytebuf buf;
  };

  struct EngramTypeRegistry {
    typedef void(*SerializeFn)(BinaryEngram&, const void*);
    typedef void(*DeserializeFn)(BinaryEngram&, void*&);
    static EngramTypeRegistry& instance() { static EngramTypeRegistry registry; return registry; }
    std::unordered_map<std::string, SerializeFn> ser_map;
    std::unordered_map<std::string, DeserializeFn> de_map;
  };

  #define ENGRAM_REGISTER_TYPE(Type, ID) \
  struct RegisterType_##Type {\
    RegisterType_##Type() {\
      engram::EngramTypeRegistry::instance().ser_map.insert({ ID, &serialize });\
      engram::EngramTypeRegistry::instance().de_map.insert({ ID, &deserialize });\
    }\
    static void serialize(engram::BinaryEngram& engram, const void* ptr) { engram << *static_cast<const Type*>(ptr); }\
    static void deserialize(engram::BinaryEngram& engram, void*& ptr) { ptr = new Type; engram >> *static_cast<Type*>(ptr); }\
  };\
  RegisterType_##Type _register_type_##Type;

}  // namespace engram