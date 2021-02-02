#include "include/regexseq.h"
#include <cstring>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>

namespace apdebug::regex_seq
{
    static constexpr auto regexFlag = std::regex_constants::ECMAScript | std::regex_constants::optimize;

    struct operate
    {
        enum Type
        {
            Replace = 0x01,
            Extract = 0x02,
            Match = 0x03
        } typ;
        union
        {
            struct
            {
                bool exact;
            } match;
            struct
            {
                bool exact;
                DynArray::DynArray<unsigned int> pos;
            } extract;
            struct
            {
                const char* fmt;
                unsigned int num;
            } replace;
        };
    };
    struct OperateSeq
    {
        OperateSeq(const YAML::Node& node);
        std::pair<bool, std::string> eval(std::string& s) const;

        std::regex reg;
        DynArray::DynArray<operate> seq;
    };

    namespace
    {
        enum class Param
        {
            Type,
            Exact,
            Format,
            Time,
            Position
        };
        static const std::unordered_map<std::string, Param> operatePar {
            { "type", Param::Type },
            { "exact", Param::Exact },
            { "format", Param::Format },
            { "time", Param::Time },
            { "position", Param::Position }
        };
        static const std::unordered_map<std::string, operate::Type> typMap {
            { "replace", operate::Type::Replace },
            { "extract", operate::Type::Extract },
            { "match", operate::Type::Match }
        };
    }
    OperateSeq::OperateSeq(const YAML::Node& node)
    {
        for (const auto& it : node)
            if (it.first.Scalar() == "regex")
                reg = std::regex(it.second.Scalar(), regexFlag);
            else if (it.first.Scalar() == "operate")
            {
                seq.allocate(it.second.size());
                operate* ptr = seq.data;
                for (const auto& sit : it.second)
                {
                    for (const auto& key : sit)
                        switch (operatePar.at(key.first.Scalar()))
                        {
                        case Param::Type:
                            ptr->typ = typMap.at(key.second.Scalar());
                            break;
                        case Param::Exact:
                            if (ptr->typ == operate::Type::Match)
                                ptr->match.exact = key.second.as<bool>();
                            else
                                ptr->extract.exact = key.second.as<bool>();
                            break;
                        case Param::Format:
                            ptr->replace.fmt = key.second.Scalar().c_str();
                            break;
                        case Param::Time:
                            ptr->replace.num = key.second.as<unsigned int>();
                            break;
                        case Param::Position:
                            ptr->extract.pos.parseArgument(key.second);
                            break;
                        }
                    ++ptr;
                }
            }
    }
    std::pair<bool, std::string> OperateSeq::eval(std::string& s) const
    {
        std::string ret;
        for (const auto& i : seq)
        {
            switch (i.typ)
            {
            case operate::Extract:
            {
                std::smatch sm;
                if (!(i.extract.exact ? std::regex_match(s, sm, reg) : std::regex_search(s, sm, reg)))
                    return std::make_pair(false, "");
                for (unsigned int it : i.extract.pos)
                    ret += R"(<:>)" + sm[it].str();
                break;
            }
            case operate::Match:
                if (!(i.match.exact ? std::regex_match(s, reg) : std::regex_search(s, reg)))
                    return std::make_pair(false, "");
                break;
            case operate::Replace:
                if (!i.replace.num)
                    s = std::regex_replace(s, reg, i.replace.fmt, std::regex_constants::match_any);
                else
                    for (unsigned int j = i.replace.num; j; --j)
                        s = std::regex_replace(s, reg, i.replace.fmt, std::regex_constants::format_first_only);
                break;
            }
        }
        return std::make_pair(true, ret);
    }

    RegexSeq::RegexSeq(const YAML::Node& node)
    {
        seq.parseArgument(node, [](OperateSeq* op, const YAML::Node& it) { new (op) OperateSeq { it }; });
    }
    std::pair<bool, std::string> RegexSeq::eval(std::string& s) const
    {
        std::string ret;
        for (const auto& i : seq)
            if (const auto [suc, val] = i.eval(s); !suc)
                return std::make_pair(false, "");
            else
                ret += val;
        return std::make_pair(true, ret);
    }
}