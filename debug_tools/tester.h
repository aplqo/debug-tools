#ifndef TESTER_H
#define TESTER_H

#include <fstream>
#include <ostream>

namespace apdebug {
namespace tester {
namespace exceptions {
class Exception {
};
class ReadFail : public Exception {
};
}  // namespace exceptions

#if __cplusplus >= 201703L
template <class... Args>
std::ostream& print(std::ostream& os, Args... arg)
{
  (os << ... << arg) << std::endl;
  return os;
}
#endif

template <class T>
T ReadOutput(std::ifstream& is)
{
  T ret;
  is >> ret;
  if (is.fail()) throw exceptions::ReadFail();
  return ret;
}
}  // namespace tester
}  // namespace apdebug

#endif