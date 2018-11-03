#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

    #include <cstdint>
    #define INLINE __inline__ __attribute__((always_inline))

    typedef float float32_t;

    #define bool_t int32_t
    
    #ifndef TRUE
        #define TRUE    1
    #endif

    #ifndef FALSE
        #define FALSE   0
    #endif

#endif