#include <sigil/platform/filesystem.h>
#include <thaumaturgy/thaumaturgy.h>
#include <sigil/data/fileinfo.h>

// namespace sigil::data {

// ::thaumaturgy::yield fileinfo_from_path(const std::filesystem::path &p, file_info_t &out) {
//     ::sigil::data::filesystem_stat_t st;
//     ::thaumaturgy::yield s = filesystem_stat(p, st);
//     if (s != VM_OK)
//         return s;

//     out.path       = p;
//     out.size       = st.size;
//     out.mtime      = st.mtime;
//     out.is_regular = st.is_regular;
//     out.is_symlink = st.is_symlink;

//     return VM_OK;
// }

// }
