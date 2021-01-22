#include "system.h"

namespace apdebug::System
{
    void consoleInit()
    {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
}