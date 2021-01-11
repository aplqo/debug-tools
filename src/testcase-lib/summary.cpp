#include "include/output.h"
#include "include/testcase.h"

#include <cmath>
#include <memory>

#include <fmt/format.h>

namespace SGR = apdebug::Output::SGR;

namespace apdebug::Testcase
{
    static const double roundWidth = 1e6;
    static const std::array<const char*, 6> header {
        "State", "Count", "Real time", "User time", "System Time", "Memory"
    };

    template <class T>
    void Summary::ValSummary<T>::update(T val, std::shared_ptr<std::string>& from)
    {
        if (val <= min.val)
            min = ValFrom { .val = val, .from = from };
        if (val >= max.val)
            max = ValFrom { .val = val, .from = from };
        sum += val;
    }
    template <class T>
    void Summary::ValSummary<T>::mergeData(const ValSummary& other)
    {
        sum += other.sum;
        if (other.min.val <= min.val)
            min = other.min;
        if (other.max.val >= max.val)
            max = other.max;
    }
    template <class T>
    template <unsigned long long unit>
    std::string Summary::ValSummary<T>::format(const double cnt) const
    {
        static constexpr double u = unit;
        return fmt::format(FMT_STRING("#{} {}({}) / {}({}) / #{} {}({})"),
            *min.from, min.val / u, min.val,
            round(roundWidth * sum / cnt / u) / roundWidth, round(roundWidth * sum / cnt) / roundWidth,
            *max.from, max.val / u, max.val);
    }
    void Summary::SummaryEntry::update(std::shared_ptr<std::string>& s, const TestResult& tst)
    {
        ++count;
        timeReal.update(tst.runTime.real, s);
        timeUser.update(tst.runTime.user, s);
        timeSys.update(tst.runTime.sys, s);
        memory.update(tst.runMemory, s);
    }
    void Summary::mergeData(const Summary& other)
    {
        for (unsigned int i = 0; i <= ResultConstant::TypeNumber; ++i)
        {
            auto& ours = entries[i];
            const auto& theirs = other.entries[i];
            ours.count += theirs.count;
            ours.timeReal.mergeData(theirs.timeReal);
            ours.timeUser.mergeData(theirs.timeUser);
            ours.timeSys.mergeData(theirs.timeSys);
            ours.memory.mergeData(theirs.memory);
        }
    }
    void Summary::insert(std::string&& s, const TestResult& tst)
    {
        std::shared_ptr<std::string> p = std::make_shared<std::string>(s);
        if (tst.runPass)
            entries[ResultConstant::TypeNumber].update(p, tst);
        for (unsigned int i = 0; tst.runResult[i]; ++i)
            entries[static_cast<unsigned int>(tst.runResult[i]->type)].update(p, tst);
    }
    void Summary::print(std::ostream& os)
    {
        using namespace ResultConstant;
        Output::Table tab(header, "");
        for (unsigned int i = 0; i <= TypeNumber; ++i)
        {
            if (!entries[i].count)
                continue;
            tab.newColumn(i != TypeNumber ? color[i] : SGR::None);
            tab.writeColumnList<int, std::string&&>({ { 0, i == TypeNumber ? std::string("All") : std::string(name[i]) },
                { 1, fmt::format(FMT_STRING("{} ({}%)"), entries[i].count, (entries[i].count * 100.0) / entries[TypeNumber].count) },
                { 2, entries[i].timeReal.format<1000>(entries[i].count) },
                { 3, entries[i].timeUser.format<1000>(entries[i].count) },
                { 4, entries[i].timeSys.format<1000>(entries[i].count) },
                { 5, entries[i].memory.format<1024>(entries[i].count) } });
        }
        tab.printHeader(os);
        tab.printAll(os);
    }
}
