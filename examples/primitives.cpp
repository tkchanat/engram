#include <stdio.h>
#include <iostream>
#include <engram_bin.hpp>
using namespace engram;

struct Bar {
  Bar(int v) : v(v) {}
  int v = 0;
  friend BinaryEngram& operator<<(BinaryEngram& engram, const Bar& self) {
    return engram << self.v;
  }
  friend BinaryEngram& operator>>(BinaryEngram& engram, Bar& self) {
    return engram >> self.v;
  }
  friend std::ostream& operator<<(std::ostream& os, const Bar& self) {
    return os << "Bar(v=" << self.v << ")";
  }
};

struct Foo {
  bool a = true;
  int b = 123;
  float c = 4.56f;
  std::string d = "Hello Engram";
  Bar e = Bar(-1);
  std::vector<int> f = { 7, 8, 9 };

 public:
  friend BinaryEngram& operator<<(BinaryEngram& engram, const Foo& self) {
    return engram << self.a << self.b << self.c << self.d << self.e << self.f;
  }
  friend BinaryEngram& operator>>(BinaryEngram& engram, Foo& self) {
    return engram >> self.a >> self.b >> self.c >> self.d >> self.e >> self.f;
  }
  friend std::ostream& operator<<(std::ostream& os, const Foo& self) {
    os << "Foo(a=" << self.a << ", b=" << self.b << ", c=" << self.c << ", d=" << self.d << ", e=" << self.e << ", f=[";
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