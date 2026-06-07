#pragma once
#include <cstdint>
#include <iomanip>
#include <string>

namespace sigilvm::strings {

    inline bool starts_with(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
    }
    
    inline std::string bytes_pretty(uint64_t bytes, uint32_t decimal = 2) {
       static const char* units[] = {
           "B", "KB", "MB", "GB", "TB", "PB"
       };
    
       constexpr uint64_t step = 1024;
       uint64_t value = bytes;
       size_t unit = 0;
    
       /* Promote unit only if full 1024 threshold is met */
       while (value >= step && unit < (sizeof(units) / sizeof(units[0])) - 1) {
           value /= step;
           ++unit;
       }
    
       std::ostringstream out;
       out.setf(std::ios::fixed, std::ios::floatfield);
    
       /* Bytes should never show decimals */
       if (unit == 0) {
           out << value << " " << units[unit];
       } else {
           double precise = static_cast<double>(bytes);
           for (size_t i = 0; i < unit; ++i) {
               precise /= step;
           }
    
           out << std::setprecision(decimal)
               << precise
               << " "
               << units[unit];
       }
    
       return out.str();
    }

} // namespace sigil::format