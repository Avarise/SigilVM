#pragma once

namespace sigil::platform {

inline constexpr bool has_avx2() {
#   if defined(__AVX2__)
    return true;
#   else
    return false;
#   endif
}
    
    
}