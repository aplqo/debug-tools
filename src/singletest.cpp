#include "include/color.h"
#include "include/define.h"
#include "include/io.h"
#include "include/testcase.h"
#include <cstring>
#include <filesystem>
#include <iostream>
using namespace apdebug;
using std::cout;
using std::strcmp;
namespace Escape = apdebug::Output::Escape;

typedef Testcase::BasicTemplate TestTemplate;
typedef Testcase::BasicTest TestcaseType;

Testcase::Platform plat;

int main(int argc, char* argv[])
{
    System::consoleInit();
    if (strcmp(argv[1], "-no-version"))
        Output::PrintVersion("Single test runner", cout);
    TestTemplate tmpl;
    std::string input, answer;
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-in"))
            input = argv[++i];
        else if (!strcmp(argv[i], "-ans"))
            answer = argv[++i];
        else if (tmpl.parseArgument(i, argv))
            continue;
    }
    tmpl.platform = &plat;
    plat.init();
    tmpl.init();
    TestcaseType test(std::move(input), std::move(answer), tmpl);
    test.printRunInfo(cout);
    cout << Escape::TextCyan << static_cast<Testcase::LimitInfo&>(tmpl) << Escape::TextBlue << "\n";
    cout << "[info] Start program" << Escape::None << std::endl;
    test.run();
    for (unsigned int i = 0; test.runResult[i]; ++i)
        std::cout << test.runResult[i]->color << test.runResult[i]->verbose << "\n";
    if (test.runPass && !test.tester.path.empty())
    {
        cout << Escape::TextBlue << "[info] Start testing" << Escape::None << std::endl;
        test.printTestInfo(cout);
        test.test();
    }
    if (test.testResult)
        std::cout << test.testResult->color << test.testResult->verbose << "\n";
    test.release();
    std::cout << Escape::None << "\n";
    return 0;
}