#include "env.h"
#include "utils.h"
#include "assets.h"
#include "target.h"
#include <cstdint>
#include <cstring>

uint32_t sigil::utils::asm_ccount(void) {
    uint32_t r = 10;
    //asm volatile ("rsr %0, ccount" : "=r"(r));
    return r;
}

const char* sigil::utils::asset_type_tostr(sigil::asset_type_t tp) {
    if (tp == sigil::asset_type_t::ASSET_SYSTEM) return "system-asset";
    //if (tp == sigil::asset_type_t::ASSET_SIGNAL) return "signal";
    //if (tp == sigil::asset_type_t::ASSET_RESERVED_SIGNAL) return "reserved-signal";
    if (tp == sigil::asset_type_t::ASSET_DEVICE) return "device driver";
    if (tp == sigil::asset_type_t::ASSET_GENERIC_MODULE) return "generic-module";
    if (tp == sigil::asset_type_t::ASSET_WORKTHREAD) return "workthread";
    if (tp == sigil::asset_type_t::ASSET_WORKQUEUE) return "workqueue";
    if (tp == sigil::asset_type_t::ASSET_FILE) return "file";
    return "unknown";
}

void sigil::utils::delay(uint32_t ms, uint32_t ns) {
    if (ms < 1 && ns < 1) return;
#   ifdef TARGET_LINUX
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = 10e6 * (ms % 1000) + ns;

    int res = 0; do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
#   endif
    return;
}

void sigil::utils::delay(uint32_t millis) {
    if (millis < 1) return;
#   ifdef TARGET_LINUX
    int res = 0;
    struct timespec ts;

    ts.tv_sec = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 10e6;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
#   elif defined(TARGET_8266)
    vTaskDelay(millis / portTICK_PERIOD_MS);
#   endif
    return;
}

void sigil::utils::microdelay(double microseconds) {
#   ifdef TARGET_LINUX
    if (microseconds < 0.0f) return;

    int res = 0;
    struct timespec ts;

    ts.tv_sec = 0;
    ts.tv_nsec = (long)(microseconds * 1000);

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
#   endif
    return;
}



int sigil::utils::set_hostname(const char *source_name) {
    int str_siz = 0;

    if (source_name == NULL) goto set_default;
    str_siz = strlen(source_name);
    if (str_siz > 32) goto set_default;

    memcpy(sigil::env::hostname, source_name, str_siz);
    return str_siz;
    
    set_default:
    snprintf(sigil::env::hostname, 32, "SigilVM");
    return 0;
}

void sigil::utils::show_hostname() {
    printf("Hostname set to \"%s\"\n", sigil::env::hostname);
}       

void sigil::utils::set_debug(uint8_t debug) {
    sigil::env::debug_mode = debug;
    //sigil_trace("sigil::utils", "debug set to %u\n", sigil::env::debug_mode);
}

// Show info about host that runs VM
void sigil::utils::show_host_info() {
#ifdef TARGET_LINUX
    printf("Running on Linux\n");
#elif defined(TARGET_8266)
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf("runtime: cores -> %d (rev.%d)\n", chip_info.cores, chip_info.revision);
    printf("runtime: %s flash -> %dkB\n", 
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external",
        spi_flash_get_chip_size() >> 10);
    printf("runtime: free heap -> %ukB\n", esp_get_free_heap_size() >> 10 );
#endif /* TARGET_LINUX */
}


int sigil::utils::set_iteration_delay(uint32_t ms) {
    if (ms > 5000) return -EPERM;

    sigil::env::iter_forced_ms = ms;
    return 0;
}

int sigil::utils::comp_lex(const char *first, const char *second) {
    if (!first || !second) return INT32_MAX;

    const int max_word_len = 128;
    int i = 0;

    while (i < max_word_len && first[i] != '\0' && second[i] != '\0') {
        if (first[i] - second[i]) return first[i] - second[i];
        i++;
    }

    return 0;
}