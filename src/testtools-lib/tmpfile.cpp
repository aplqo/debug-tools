#include <cstring>
#include <filesystem>

#include "include/pathFormat.h"
#include "include/testtools.h"
#include "include/utility.h"
namespace fs = std::filesystem;

namespace apdebug::TestTools {
TemporaryFile& TemporaryFile::instantiate(fmt::format_args args)
{
  if (!enable) return *this;
  for (unsigned int i = 0; i < 2; ++i)
    for (unsigned int j = 0; j < 2; ++j)
      for (unsigned int k = 0; k < 2; ++k)
        if (filesTemplate[i][j][k].data) {
          auto& cur = files[i][j][k];
          const auto& pattern = filesTemplate[i][j][k];
          cur.allocate(pattern.size);
          for (unsigned int i = 0; i < pattern.size; ++i)
            new (cur.data + i) fs::path(fmt::vformat(pattern.data[i], args));
        }
  return *this;
}
void TemporaryFile::parseArgument(const YAML::Node& nod)
{
  if (nod.IsNull()) {
    enable = false;
    return;
  }
  enable = true;
  for (const auto& it : nod) {
    const std::string& s = it.first.Scalar();
    filesTemplate[s[0] == 'T'][s[1] == 'P'][s[2] == 'P'].parseArgument(
        it.second, [](const char** dest, const YAML::Node& node) {
          *dest = node.Scalar().c_str();
        });
  }
}
void TemporaryFile::release(const Phase p, const bool pass, const bool accept)
{
  if (!enable) return;
  for (const auto& i : files[p][pass][accept])
    fs::remove(i);
}
TemporaryFile::~TemporaryFile()
{
  for (auto& i : files)
    for (auto& j : i)
      for (auto& k : j)
        k.release();
}
}  // namespace apdebug::TestTools
