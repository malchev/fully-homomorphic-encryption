#ifndef TRANSPILER_EXAMPLES_STRUCTS_STRUCT_WITH_ARRAY_H_
#define TRANSPILER_EXAMPLES_STRUCTS_STRUCT_WITH_ARRAY_H_

// Example demonstrating successful manipulation of arrays inside
// structs.
#define A_COUNT 3
#define B_COUNT 3
#define C_COUNT 3

struct Inner {
  int c[C_COUNT];
  short q;
};

struct StructWithArray {
  char c;
#if 1
  Inner i;
  int a[A_COUNT];
  short b[B_COUNT];
  short z;
#endif
};

void NegateStructWithArray(StructWithArray &input, int &other, Inner &inner);

#endif  // TRANSPILER_EXAMPLES_STRUCTS_STRUCT_WITH_ARRAY_H_
