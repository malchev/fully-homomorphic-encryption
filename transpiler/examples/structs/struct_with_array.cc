#include "struct_with_array.h"

#pragma hls_top
void NegateStructWithArray(StructWithArray &input) {
#pragma hls_unroll yes
  for (int i = 0; i < A_COUNT; i++) {
    input.a[i] = -input.a[i];
  }

#pragma hls_unroll yes
  for (int i = 0; i < B_COUNT; i++) {
    input.b[i] = -input.b[i];
  }
  input.c = -input.c;
}
