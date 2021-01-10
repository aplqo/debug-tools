#include "include/regexseq.h"
#include <cstring>
#include <regex>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace apdebug::regex_seq
{
    using std::atoi;
    using std::get;
    using std::make_pair;
    using std::pair;
    using std::regex;
    using std::regex_match;
    using std::regex_replace;
    using std::regex_search;
    using std::string;

    static constexpr auto regexFlag = std::regex_constants::ECMAScript | std::regex_constants::optimize;
    namespace operation
    {
        match::match(int& pos, const char* const argv[])
            : algo(argv[pos][2] == 'm' ? Match : Search)
        {
        }
        bool match::eval(const string& s, const regex& r) const
        {
            return algo == Match ? regex_match(s, r) : regex_search(s, r);
        }

        extract::extract(int& pos, const char* const argv[])
        {
            algo = argv[pos++][2] == 'm' ? Match : Search;
            if (strcmp(argv[pos], "["))
                position.push_back(atoi(argv[pos]));
            else
                for (++pos; strcmp(argv[pos], "]"); ++pos)
                    position.push_back(atoi(argv[pos]));
        }
        pair<bool, string> extract::eval(const string& s, const regex& r) const
        {
            std::smatch m;
            pair<bool, string> ret;
            ret.first = algo == Match ? regex_match(s, m, r) : regex_search(s, m, r);
            if (!ret.first)
                return ret;
            for (auto i : position)
                ret.second += R"(<:>)" + m[i].str();
            return ret;
        }

        replace::replace(int& pos, const char* const argv[])
        {
            num = std::atoi(argv[++pos]);
            fmt = argv[++pos];
        }
        void replace::eval(string& s, const regex& r) const
        {
            using namespace std::regex_constants;
            if (!num)
            {
                s = regex_replace(s, r, fmt, std::regex_constants::match_any);
                return;
            }
            bool flag = true;
            for (unsigned int i = 0; i < num && flag; ++i)
                s = regex_replace(s, r, fmt, std::regex_constants::format_first_only);
        }
    }

    OperateSeq::OperateSeq(int& pos, const char* const argv[])
    {
        reg = regex(argv[pos++], regexFlag);
        for (; true; ++pos)
        {
            if (!strcmp(argv[pos], "-ms") || !strcmp(argv[pos], "-mm"))
                seq.push_back(operate { operate::Match, operation::match(pos, argv) });
            else if (!strcmp(argv[pos], "-es") || !strcmp(argv[pos], "-em"))
                seq.push_back(operate { operate::Extract, operation::extract(pos, argv) });
            else if (!strcmp(argv[pos], "-r"))
                seq.push_back(operate { operate::Replace, operation::replace(pos, argv) });
            else if (!strcmp(argv[pos], ";"))
                break;
        }
    }
    pair<bool, string> OperateSeq::eval(string& s) const
    {
        string ret;
        for (const auto& i : seq)
        {
            switch (i.typ)
            {
            case operate::Extract:
            {
                const auto [suc, val] = get<operation::extract>(i.val).eval(s, reg);
                if (!suc)
                    return make_pair(false, "");
                ret += val;
                break;
            }
            case operate::Match:
                if (!get<operation::match>(i.val).eval(s, reg))
                    return make_pair(false, "");
                break;
            case operate::Replace:
                get<operation::replace>(i.val).eval(s, reg);
                break;
            }
        }
        return make_pair(true, ret);
    }

    RegexSeq::RegexSeq(int& pos, const char* const argv[])
    {
        ++pos;
        for (; strcmp(argv[pos], "]"); ++pos)
            reg.emplace_back(pos, argv);
    }
    pair<bool, string> RegexSeq::eval(string& s) const
    {
        string ret;
        for (const auto& i : reg)
        {
            auto [suc, val] = i.eval(s);
            if (!suc)
                return make_pair(false, "");
            ret += val;
        }
        return make_pair(true, ret);
    }
}