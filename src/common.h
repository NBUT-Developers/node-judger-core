#ifndef __COMMON_H__
#define __COMMON_H__

#include <time.h>
#include <stdlib.h>
#include <string>
#include <Windows.h>

namespace NodeJudger {

typedef SIZE_T __size;
#define VALID_HANDLE(handle) (handle != INVALID_HANDLE_VALUE && handle != NULL)
#define SAFE_CLOSE_HANDLE(handle) { if(VALID_HANDLE(handle)) CloseHandle(handle); }

enum StateEnum {
    FINISHED = 0,
    CONTINUE = 1,
    TIME_LIMIT_EXCEEDED_1 = 2,
    TIME_LIMIT_EXCEEDED_2 = 3,
    MEMORY_LIMIT_EXCEEDED = 4,
    OUTPUT_LIMIT_EXCEEDED = 5,
    RUNTIME_ERROR = 6,
    SYSTEM_ERROR = 7,
    DANGEROUS_CODE = 8
};

struct CodeState {
    __int64 exe_time;
    __size exe_memory;
    StateEnum state;
    char error_code[128];

    CodeState()
    {
        exe_time = exe_memory = 0;
        state = CONTINUE;
        memset(error_code, 0, sizeof(error_code));
    }
};

std::string GetLastErrorAsString();
std::string GetEnvironmentVar(LPCSTR key);
std::string GetTempDirectory();

}
#endif