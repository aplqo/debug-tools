#ifndef UTILITY_H
#define UTILITY_H

#include <filesystem>

namespace apdebug::Utility {
inline bool removeFile(std::filesystem::path& s)
{
  if (std::filesystem::remove(s)) {
    s = "<released>";
    return true;
  }
  else {
    s = "<unused>";
    return false;
  }
}

}  // namespace apdebug::Utility

#endif
