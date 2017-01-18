#ifndef __COMMON_H__
#define __COMMON_H__

#include <time.h>
#include <stdlib.h>
#include <string>
#include <Windows.h>

namespace NodeJudger {

typedef SIZE_T __size;
#define VALID_HANDLE(handle) (handle != INVALID_HANDLE_VALUE)
#define SAFE_CLOSE_HANDLE(handle) { if(VALID_HANDLE(handle)) CloseHandle(handle); }

enum StateEnum {
    FINISHED,
    CONTINUE,
    TIME_LIMIT_EXCEEDED_1,
    TIME_LIMIT_EXCEEDED_2,
    MEMORY_LIMIT_EXCEEDED,
    OUTPUT_LIMIT_EXCEEDED,
    RUNTIME_ERROR,
    SYSTEM_ERROR
};

struct CodeState {
    __int64 exe_time;
    __size exe_memory;
    StateEnum state;
    char error_code[32];
};

std::string GetLastErrorAsString();

}
#endif