
#ifndef WTS_WTS_TYPES_H_
#define WTS_WTS_TYPES_H_

#include <stdint.h>
#include <stdlib.h>

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)
#endif

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#define INLINE_TEXT(n) #n

#ifdef _WIN32

// unnamed struct , for union.
#pragma warning(disable:4201)
#endif

#ifndef WTS_CONSTEXPR
#define WTS_CONSTEXPR
#endif

#define null_ptr ((void*)0)

#endif
