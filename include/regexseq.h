#ifndef REGEXSEQ_H
#define REGEXSEQ_H

#include <regex>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace apdebug::regex_seq
{
    namespace operation
    {
        struct match
        {
            match(int& pos, const char* const argv[]);
            bool eval(const std::string& s, const std::regex& r) const;

            enum
            {
                Search,
                Match
            } algo;
        };
        struct extract
        {
            extract(int& pos, const char* const argv[]);
            std::pair<bool, std::string> eval(const std::string& s, const std::regex& r) const;

            enum
            {
                Search,
                Match
            } algo;
            std::vector<unsigned int> position;
        };
        struct replace
        {
            replace(int& pos, const char* const argv[]);
            void eval(std::string& s, const std::regex& r) const;

            const char* fmt;
            unsigned int num = 0;
        };
    }
    struct operate
    {
        enum
        {
            Replace = 0x01,
            Extract = 0x02,
            Match = 0x03
        } typ;
        std::variant<operation::extract, operation::match, operation::replace> val;
    };
    struct OperateSeq
    {
        OperateSeq(int& pos, const char* const argv[]);
        std::pair<bool, std::string> eval(std::string& s) const;

        std::regex reg;
        std::vector<operate> seq;
    };
    struct RegexSeq
    {
        RegexSeq(int&, const char* const argv[]);
        std::pair<bool, std::string> eval(std::string& s) const;

        std::vector<OperateSeq> reg;
    };
}

#endif