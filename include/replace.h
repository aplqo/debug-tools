#ifndef REPLACE_H
#define REPLACE_H

#include <fmt/format.h>

namespace apdebug::Replace {
template <class T>
concept Replaceable = requires(fmt::format_args pat, T&& a)
{
  {a.instantiate(pat)};
};
template <Replaceable T>
inline void ReplaceMaybe(fmt::format_args pat, T&& a)
{
  a.instantiate(pat);
}
template <class T>
inline void ReplaceMaybe(fmt::format_args, T&&)
{
}
template <class T, class... Args>
inline void ReplaceMaybe(fmt::format_args pat, T&& one, Args&&... other)
{
  ReplaceMaybe(pat, one);
  ReplaceMaybe(pat, other...);
}
template <Replaceable... Args>
inline void ReplaceStrict(fmt::format_args pat, Args&&... other)
{
  ReplaceMaybe(pat, other...);
}
}  // namespace apdebug::Replace

#endif