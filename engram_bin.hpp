#pragma once
#include <sstream>
#include <array>
#include <optional>
#include <unordered_map>
#include <vector>

#ifdef _WINDLL
#define ENGRAM_EXTERN_REGISTRY
/* Remember to put this definition somewhere in your dll.
 * engram::EngramTypeRegistry& engram::registry_instance() {
 *   static engram::EngramTypeRegistry registry;
 *   return registry;
 * }
 */
#endif

namespace engram {

  typedef std::basic_stringbuf<std::byte> bytebuf;
  typedef std::basic_iostream<std::byte> bytestream;

  class BinaryEngram : bytestream {
    template <typename T> struct dependent_false { static constexpr bool value = false; };
    // SFINAE test
    template <typename T>
    struct has_type_id {
      typedef char one;
      struct two { char x[2]; };
      template<typename C> static one test(decltype(&C::type_id));
      template<typename C> static two test(...);    
      enum { value = sizeof(test<T>(0)) == sizeof(char) };
    };

  public:
    BinaryEngram() : bytestream(&buf) {}
  
    // Serialization
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
    template<typename Key, typename Value>
    BinaryEngram& operator<<(const std::unordered_map<Key, Value>& v) {
      const size_t len = v.size();
      *this << len;
      for (const auto& pair : v)
        *this << pair.first << pair.second;
      return *this;
    }
    template<typename Type>
    BinaryEngram& operator<<(const std::optional<Type>& v) {
      *this << v.has_value();
      if (v.has_value())
        *this << v.value();
      return *this;
    }
    template<typename Type>
    BinaryEngram& operator<<(const Type& v) {
      if constexpr      (std::is_integral_v<Type>)        return serialize_prims(v);
      else if constexpr (std::is_floating_point_v<Type>)  return serialize_prims(v);
      else if constexpr (std::is_enum_v<Type>)            return serialize_enum(v);
      else if constexpr (std::is_pointer_v<Type>)         return serialize_ptr(v);
      else static_assert(dependent_false<Type>::value, "This type did not overload with operator<<.");
    }
    BinaryEngram& serialize_bytes(const std::byte* ptr, const size_t size) {
      *this << size;
      if (size > 0)
        buf.sputn(ptr, size);
      return *this;
    }

  private:
    template<typename Type>
    BinaryEngram& serialize_prims(const Type& v) {
      buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(Type));
      return *this;
    }
    template<typename Type>
    BinaryEngram& serialize_enum(const Type v) {
      buf.sputn(reinterpret_cast<const std::byte*>(&v), sizeof(std::underlying_type_t<Type>));
      return *this;
    }
    template<typename Type>
    BinaryEngram& serialize_ptr(const Type* v) {
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

  public:
    // Deserialization
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
    template<typename Key, typename Value>
    BinaryEngram& operator>>(std::unordered_map<Key, Value>& v) {
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
    BinaryEngram& operator>>(std::optional<Type>& v) {
      bool has_value; *this >> has_value;
      if (has_value)
        *this >> *v;
      return *this;
    }
    template<typename Type>
    BinaryEngram& operator>>(Type& v) {
      if constexpr      (std::is_integral_v<Type>)        return deserialize_prims(v);
      else if constexpr (std::is_floating_point_v<Type>)  return deserialize_prims(v);
      else if constexpr (std::is_enum_v<Type>)            return deserialize_enum(v);
      else if constexpr (std::is_pointer_v<Type>)         return deserialize_ptr(v);
      else static_assert(dependent_false<Type>::value, "This type did not overload with operator>>.");
    }
    BinaryEngram& deserialize_bytes(std::byte*& ptr, size_t& size) {
      *this >> size;
      if (size > 0) {
        if (ptr) delete[] ptr;
        ptr = new std::byte[size];
        buf.sgetn(ptr, size);
      }
      return *this;
    }

  private:
    template<typename Type>
    BinaryEngram& deserialize_prims(Type& v) {
      buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(Type));
      return *this;
    }
    template<typename Type>
    BinaryEngram& deserialize_enum(Type& v) {
      buf.sgetn(reinterpret_cast<std::byte*>(&v), sizeof(std::underlying_type_t<Type>));
      return *this;
    }
    template<typename Type>
    BinaryEngram& deserialize_ptr(Type*& v) {
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

  private:
    bytebuf buf;
  };

#ifdef ENGRAM_EXTERN_REGISTRY
  extern struct EngramTypeRegistry& registry_instance();
#endif

  struct EngramTypeRegistry {
    typedef void(*SerializeFn)(BinaryEngram&, const void*);
    typedef void(*DeserializeFn)(BinaryEngram&, void*&);
    static EngramTypeRegistry& instance() {
#ifdef ENGRAM_EXTERN_REGISTRY
      return registry_instance();
#else
      static EngramTypeRegistry registry;
      return registry;
#endif
    }
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
  RegisterType_##Type __engram_register_type_##Type;

}  // namespace engram