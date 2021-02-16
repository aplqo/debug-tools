#ifndef pathFormat_H
#define pathFormat_h

#include <fmt/format.h>

#include <filesystem>

template <>
struct fmt::formatter<std::filesystem::path> {
  inline constexpr auto parse(fmt::format_parse_context& c)
  {
    return c.begin();
  }
  template <class FormatContext>
  inline auto format(const std::filesystem::path& str, FormatContext& ctx)
  {
    auto p = str.c_str();
    while (*p)
      *ctx.out() = *(p++);
    return ctx.out();
  }
};

#endif