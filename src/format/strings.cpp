#include <sigil/format/strings.h>
#include <iomanip>

namespace sigil::format {

std::string bytes_pretty(uint64_t bytes, uint32_t decimal) {
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

}
