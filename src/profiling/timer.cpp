#include <sigil/profiling/timer.h>

namespace sigil::util {
    long long unix_time() {
        return duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
}
