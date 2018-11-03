#ifndef _CMATH_H_
#define _CMATH_H_

#include "typedef.h"

typedef struct _matrix33_t
{
  float32_t m11;
  float32_t m12;
  float32_t m13;
  float32_t m21;
  float32_t m22;
  float32_t m23;
  float32_t m31;
  float32_t m32;
  float32_t m33;
} matrix33_t;

typedef struct _vector31_t {
  union {
    float32_t v[3];
    struct
    {
      float32_t x;
      float32_t y;
      float32_t z;
    };
  };
} vector31_t;

typedef union _vector21_t {
  float32_t v[2];
  struct
  {
    float32_t x;
    float32_t y;
  };
} vector21_t;


#endif // _MATH_H_