#pragma once
#include <cstring>
#include <mutex>

namespace sigil {
    enum asset_type_t {
        ASSET_UNKNOWN,
        ASSET_SYSTEM,
        ASSET_DEVICE,
        ASSET_EVENT,
        ASSET_FILE,
        ASSET_SERVICE,
        ASSET_GENERIC_MODULE,
        ASSET_WORKTHREAD,
        ASSET_WORKQUEUE,
        ASSET_ENGINE,
    };

    struct asset_t {
        sigil::asset_type_t type;
        std::mutex asset_mtx;
        uint32_t refcount;
        void *data;
        char *path;

        asset_t(void) {
            type = ASSET_UNKNOWN;
            refcount = 0;
            data = NULL;
            path = NULL;
        }

        asset_t(const char *path) {
            type = ASSET_FILE;
            refcount = 0;
            data = NULL;
            path = path;
        }

        asset_t(const asset_t &source) {
            type = source.type;
            refcount = 0;
            data = NULL;
            strcpy(path, source.path);
        }

        // TODO: Add copy constructor, which allocates new mem region to clone data
    };
}