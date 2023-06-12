#include <stdio.h>
#include <iostream>
#include <engram_bin.hpp>
using namespace engram;

int main() {
  BinaryEngram engram;
  // Serialize
  {
    bool a = true;
    int b = 123;
    float c = 4.56f;
    engram << a << b << c;
  }
  // Deserialize
  {
    bool a;
    int b;
    float c;
    engram >> a >> b >> c;
    printf("a=%s, b=%d, c=%.2f\n", a ? "true" : "false", b, c);
  }
  return 0;
}