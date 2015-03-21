/**
 * =====================================================================================
 *
 *       Filename:  exe_runner.h
 *
 *    Description:  The *.exe runner.
 *
 *        Version:  1.0
 *        Created:  3/21/2015 6:33:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  XadillaX (admin@xcoder.in)
 *   Organization:  Ningbo University of Technology
 *
 * =====================================================================================
 */
#ifndef __EXE_RUNNER_H__
#define __EXE_RUNNER_H__
#pragma once

#include <windows.h>

typedef SIZE_T __size;

enum StateEnum {
    FINISHED,
    TIME_LIMIT_EXCEEDED,
    MEMORY_LIMIT_EXCEEDED,
    OUTPUT_LIMIT_EXCEEDED,
    SYSTEM_ERROR
};

struct CodeState {
    __int64 exe_time;
    __size exe_memory;
    StateEnum state;
    char error_code[32];
};

PROCESS_INFORMATION* RunExe(const char* exe_path,
        const char* command,
        const char* std_input_path,
        const char* output_path,
        CodeState& code_state);

#endif

