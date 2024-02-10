#pragma once
#include "assets.h"
#include <cstdint>

namespace sigil {
    namespace utils {
        uint32_t asm_ccount(void);
        const char* asset_type_tostr(sigil::asset_type_t tp);
        void delay(uint32_t ms, uint32_t ns);
        void delay(uint32_t millis);
        void microdelay(double microseconds);
        int set_hostname(const char *source_name);
        void show_hostname();
        void show_host_info();
        void set_debug(uint8_t debug);
        int set_iteration_delay(uint32_t ms);
        int comp_lex(const char *first, const char *second);
        int comp_len();
        int comp_num_uniques();
    } /* End of namespace utils */
}