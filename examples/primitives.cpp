#include <stdio.h>
#include <iostream>
#include <engram_bin.hpp>
using namespace engram;

struct Base {
  std::string type = "base";
  virtual const char* type_id() const { return "Base"; }
  virtual std::ostream& print(std::ostream& os) const { return os << "Base(type=" << type << ")"; }
  friend BinaryEngram& operator<<(BinaryEngram& engram, const Base& self) { return engram << self.type; }
  friend BinaryEngram& operator>>(BinaryEngram& engram, Base& self) { return engram >> self.type; }
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
  friend BinaryEngram& operator<<(BinaryEngram& engram, const Derived& self) { return engram << (const Base&)self << self.payload; }
  friend BinaryEngram& operator>>(BinaryEngram& engram, Derived& self) { return engram >> (Base&)self >> self.payload; }
  friend std::ostream& operator<<(std::ostream& os, const Derived& self) { return self.print(os); }
};
ENGRAM_REGISTER_TYPE(Derived, "Derived");

struct Foo {
  bool a = true;
  int b = 123;
  float c = 4.56f;
  std::string d = "Hello Engram";
  Base* e = new Derived;
  std::vector<int> f = { 7, 8, 9 };

 public:
  friend BinaryEngram& operator<<(BinaryEngram& engram, const Foo& self) {
    return engram << self.a << self.b << self.c << self.d << self.e << self.f;
  }
  friend BinaryEngram& operator>>(BinaryEngram& engram, Foo& self) {
    return engram >> self.a >> self.b >> self.c >> self.d >> self.e >> self.f;
  }
  friend std::ostream& operator<<(std::ostream& os, const Foo& self) {
    os << "Foo(a=" << self.a << ", b=" << self.b << ", c=" << self.c << ", d=" << self.d << ", e=";
    if (self.e) os << *self.e;
    else os << "(null)";
    os << ", f=[";
    for (const auto& i : self.f)
      os << i << ',';
    return os << "])";
  }
};

int main() {
  BinaryEngram engram;
  // Serialize
  {
    Foo foo;
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