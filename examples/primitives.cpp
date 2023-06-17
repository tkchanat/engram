#include <stdio.h>
#include <iostream>
#include <engram_bin.hpp>

struct Base {
  std::string type = "base";
  virtual const char* type_id() const { return "Base"; }
  virtual std::ostream& print(std::ostream& os) const { return os << "Base(type=" << type << ")"; }
  friend engram::BinaryEngram& operator<<(engram::BinaryEngram& engram, const Base& self) { return engram << self.type; }
  friend engram::BinaryEngram& operator>>(engram::BinaryEngram& engram, Base& self) { return engram >> self.type; }
  friend std::ostream& operator<<(std::ostream& os, const Base& self) { return self.print(os); }
};
ENGRAM_REGISTER_TYPE(Base, "Base");

struct Derived : public Base {
  Derived() { type = "derived"; }
  std::array<char, 6> payload = { 's', 'e', 'c', 'r', 'e', 't' };
  const char* type_id() const override { return "Derived"; }
  std::ostream& print(std::ostream& os) const override {
    os << "Derived(type=" << type << ", payload=";
    for (char c : payload)
      os << c;
    return os << ")";
  }
  friend engram::BinaryEngram& operator<<(engram::BinaryEngram& engram, const Derived& self) { return engram << (const Base&)self << self.payload; }
  friend engram::BinaryEngram& operator>>(engram::BinaryEngram& engram, Derived& self) { return engram >> (Base&)self >> self.payload; }
  friend std::ostream& operator<<(std::ostream& os, const Derived& self) { return self.print(os); }
};
ENGRAM_REGISTER_TYPE(Derived, "Derived");

struct Foo {
  bool a = false;
  int b = 0;
  float c = 0.f;
  std::string d;
  Base* e = nullptr;
  std::vector<int> f;
  enum class Enum { None, A, B } g = Enum::None;
  std::unordered_map<std::string, int> h;

 public:
  friend engram::BinaryEngram& operator<<(engram::BinaryEngram& engram, const Foo& self) {
    return engram << self.a << self.b << self.c << self.d << self.e << self.f << self.g << self.h;
  }
  friend engram::BinaryEngram& operator>>(engram::BinaryEngram& engram, Foo& self) {
    return engram >> self.a >> self.b >> self.c >> self.d >> self.e >> self.f >> self.g >> self.h;
  }
  friend std::ostream& operator<<(std::ostream& os, const Foo& self) {
    os << "Foo(a=" << self.a << ", b=" << self.b << ", c=" << self.c << ", d=" << self.d << ", e=";
    if (self.e) os << *self.e;
    else os << "(null)";
    os << ", f=[";
    for (const auto& i : self.f)
      os << i << ',';
    os << "], g=" << (int)self.g << ", h={";
    for (const auto& i : self.h)
      os << '{' << i.first << ',' << i.second << "},";
    return os << "})";
  }
};

int main() {
  engram::BinaryEngram engram;
  // Serialize
  {
    Foo foo;
    foo.a = true;
    foo.b = 123;
    foo.c = 4.56f;
    foo.d = "Hello Engram";
    foo.e = new Derived;
    foo.f = { 7, 8, 9 };
    foo.g = Foo::Enum::A;
    foo.h["MagicNumber"] = 45510;
    engram << foo;
  }
  // Deserialize
  {
    Foo foo;
    engram >> foo;
    std::cout << foo << std::endl;
  }
  return 0;
}