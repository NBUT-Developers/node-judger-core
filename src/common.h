#ifndef __COMMON_H__
#define __COMMON_H__

#include <windows.h>
#include <time.h>
#include <stdlib.h>
typedef SIZE_T __size;

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
#endif