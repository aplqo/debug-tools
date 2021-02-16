#ifndef REGEXSEQ_H
#define REGEXSEQ_H

#include <yaml-cpp/yaml.h>

#include <regex>
#include <string>
#include <utility>

#include "include/dynArray.h"

namespace apdebug::regex_seq {
struct RegexSeq {
  RegexSeq(const YAML::Node& node);
  std::pair<bool, std::string> eval(std::string& s) const;

  DynArray::DynArray<struct OperateSeq> seq;
};
}  // namespace apdebug::regex_seq

#endif