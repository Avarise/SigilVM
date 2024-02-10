#pragma once
#include <cstdint>

/* Memory helpers */
#define HASH_MASK 0xDEADBEEF
#define MSIZE_4K 4096
#define MMASK_4K 0xFFFFF000
#define MSIZE_PAGE 512
#define MMASK_PAGE 0xFFFFFE00
#define MSIZE_STRING 64
#define MMASK_STRING 0xFFFFFFC0
#define MSIZE_CHUNK 16
#define MMASK_CHUNK 0xFFFFFFF0
#define MAX_THREADS 128
#ifndef BIT
#define BIT(x) ((uint32_t)1 << x)
#endif /* BIT */

namespace sigil {
    namespace memory {
        struct memstat_t {
            unsigned long size, resident, share, text, lib, data, dt;
        };

        // Print next 16 bytes in memory overwiev style
        void view_chunk(const void *mem_start);

        void view(const void *mem_start, uint32_t num_bytes, bool align);

        void get_memstats(memstat_t *result) ;
    }
}