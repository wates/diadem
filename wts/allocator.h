
#ifndef WTS_ALLOCATOR_H_
#define WTS_ALLOCATOR_H_

#include <memory>
#include <stdlib.h>

#include "meta_container.h"

namespace wts
{
    template<typename T,int Size=
        (sizeof(T) < 64 ? 1 :
        (sizeof(T) < 512 ? 2 :
        (sizeof(T) < 2048 ? 3 :
        (0))))>struct AllocatorSelector
    {
        inline static void *Alloc(size_t size)
        {
            return ::malloc(size);
        }
        inline static void Free(void *p)
        {
            ::free(p);
        }
    };
}

#endif

