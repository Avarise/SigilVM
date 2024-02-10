#pragma once
#include "assets.h"
/*
    Filesystem header
*/

namespace sigil {
    namespace fs {
        class file : asset_t {
            public:
                char read_char();
        };

        inline int file_to_buffer(void *mem_start, int max_len, char *path) {
            int ret = 0; // Return num bytes read on success (can be 0), or negative num on error
            return ret;
        }
    }
}