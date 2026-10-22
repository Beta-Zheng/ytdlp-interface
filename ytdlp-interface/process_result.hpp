#pragma once

#include <Windows.h>
#include <string>

namespace util
{
    struct process_result
    {
        bool started {false};
        bool cancelled {false};
        bool exit_code_available {false};
        DWORD exit_code {0};
        DWORD error_code {0};
        std::string error_message;

        bool succeeded() const noexcept
        {
            return started && !cancelled && exit_code_available && exit_code == 0;
        }
    };
}
