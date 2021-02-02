#ifndef REGEXSEQ_H
#define REGEXSEQ_H

#include "include/dynArray.h"

#include <regex>
#include <string>
#include <utility>

#include <yaml-cpp/yaml.h>

namespace apdebug::regex_seq
{
    struct RegexSeq
    {
        RegexSeq(const YAML::Node& node);
        std::pair<bool, std::string> eval(std::string& s) const;

        DynArray::DynArray<struct OperateSeq> seq;
    };
}

#endif