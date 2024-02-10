#include "assets.h"
#include <string>

namespace sigil {
    namespace network {
        struct wifi_entry_t {
            uint8_t encryption_type;
            std::string password;
            std::string ssid;
            int32_t channel;
            char bssid[6]; // MAC address
            int32_t rssi;
            bool hidden;
        };
    }
}