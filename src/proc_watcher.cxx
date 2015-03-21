/**
 * =====================================================================================
 *
 *       Filename:  proc_watcher.cxx
 *
 *    Description:  The process watcher.
 *
 *        Version:  1.0
 *        Created:  3/21/2015 11:27:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  XadillaX (admin@xcoder.in)
 *   Organization:  Ningbo University of Technology
 *
 * =====================================================================================
 */
#include "proc_watcher.h"
#include <psapi.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment (lib, "psapi.lib")

#define MAX_TIME_LIMIT_DELAY 200

void RuntimeErrorCode_(DWORD code, CodeState& code_state)
{
    switch(code)
    {
    case EXCEPTION_ACCESS_VIOLATION:
        {
            strcpy(code_state.error_code, "ACCESS_VIOLATION");
            break;
        }

    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        {
            strcpy(code_state.error_code, "ARRAY_BOUNDS_EXCEEDED");
            break;
        }

    case EXCEPTION_FLT_DENORMAL_OPERAND:
        {
            strcpy(code_state.error_code, "FLOAT_DENORMAL_OPERAND");
            break;
        }

    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        {
            strcpy(code_state.error_code, "FLOAT_DIVIDE_BY_ZERO");
            break;
        }

    case EXCEPTION_FLT_OVERFLOW:
        {
            strcpy(code_state.error_code, "FLOAT_OVERFLOW");
            break;
        }

    case EXCEPTION_FLT_UNDERFLOW:
        {
            strcpy(code_state.error_code, "FLOAT_UNDERFLOW");
            break;
        }

    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        {
            strcpy(code_state.error_code, "INTEGER_DIVIDE_BY_ZERO");
            break;
        }

    case EXCEPTION_INT_OVERFLOW:
        {
            strcpy(code_state.error_code, "INTEGER_OVERFLOW");
            break;
        }

    case EXCEPTION_STACK_OVERFLOW:
        {
            strcpy(code_state.error_code, "STACK_OVERFLOW");
            break;
        }

    default:
        {
            char temp[32];
            sprintf(temp, "OTHER_ERRORS_0x%.8X", code);
            strcpy(code_state.error_code, temp);
            break;
        }
    }
}

__int64 GetRunTime_(HANDLE process)
{
    _FILETIME create_time, exit_time, kernel_time, user_time;
    __int64* ut;

    if(GetProcessTimes(process, &create_time, &exit_time, &kernel_time, &user_time))
    {
        ut = reinterpret_cast<__int64*>(&user_time);
        return (__int64)((*ut) / 10000);
    }

    return -1;
}

__int64 GetRunMemo_(HANDLE process)
{
    PROCESS_MEMORY_COUNTERS memo_counter;
    memo_counter.cb = sizeof(PROCESS_MEMORY_COUNTERS);

    if(GetProcessMemoryInfo(process, &memo_counter, sizeof(PROCESS_MEMORY_COUNTERS)))
    {
        return memo_counter.PagefileUsage / 1024;
    }

    return -1;
}

void SetError_(HANDLE process,
        DWORD process_id,
        CodeState& code_state,
        StateEnum state_enum,
        const char* code = NULL)
{
    DebugActiveProcessStop(process_id);
    TerminateProcess(process, 4);

    code_state.state = state_enum;
    if(code && strlen(code) != 0)
    {
        strcpy(code_state.error_code, code);
    }
}

bool WatchCode(const HANDLE process,
        const __int64 time_limit,
        const __size memo_limit,
        CodeState& code_state,
        const PROCESS_INFORMATION proc_info)
{
    __int64 run_memo = 0;
    __int64 run_time = -1;
    DEBUG_EVENT dbe;

    // wait for debug event
    bool flag = WaitForDebugEvent(&dbe, time_limit + MAX_TIME_LIMIT_DELAY);
    if(!flag)
    {
        code_state.exe_time = GetRunMemo_(process);
        code_state.exe_memory = GetRunTime_(process) + time_limit + (rand() / MAX_TIME_LIMIT_DELAY);
        SetError_(process, proc_info.dwProcessId, code_state, TIME_LIMIT_EXCEEDED_2);
        return true;
    }

    // close some handles
    if(dbe.u.CreateProcessInfo.hFile) CloseHandle(dbe.u.CreateProcessInfo.hFile);
    if(dbe.u.CreateProcessInfo.hProcess) CloseHandle(dbe.u.CreateProcessInfo.hProcess);
    if(dbe.u.CreateProcessInfo.hThread) CloseHandle(dbe.u.CreateProcessInfo.hThread);

    // get the run time
    run_time = GetRunTime_(process);
    if(-1 == run_time)
    {
        code_state.exe_time = -1;
        code_state.exe_memory = GetRunMemo_(process);
        SetError_(process,
                proc_info.dwProcessId,
                code_state,
                SYSTEM_ERROR,
                "Can't get running time.");
        return true;
    }

    // get the run memo
    run_memo = GetRunMemo_(process);
    if(-1 == run_memo)
    {
        code_state.exe_time = run_time;
        code_state.exe_memory = -1;
        SetError_(process,
                proc_info.dwProcessId,
                code_state,
                SYSTEM_ERROR,
                "Can't get occupied memory.");
        return true;
    }

    code_state.exe_time = run_time;
    code_state.exe_memory = run_memo;

    // time limit || memo limit
    if(run_time > time_limit)
    {
        SetError_(process, proc_info.dwProcessId, code_state, TIME_LIMIT_EXCEEDED_1);
        return true;
    }
    if(run_memo > memo_limit)
    {
        SetError_(process, proc_info.dwProcessId, code_state, MEMORY_LIMIT_EXCEEDED);
    }

    // finished
    if(dbe.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
    {
        ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, DBG_CONTINUE);
        if(dbe.u.CreateProcessInfo.hFile) CloseHandle(dbe.u.CreateProcessInfo.hFile);
        if(dbe.u.CreateProcessInfo.hProcess) CloseHandle(dbe.u.CreateProcessInfo.hProcess);
        if(dbe.u.CreateProcessInfo.hThread) CloseHandle(dbe.u.CreateProcessInfo.hThread);
        code_state.state = FINISHED;
        return true;
    }

    // runtime error
    if(dbe.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
    {
        if(dbe.u.Exception.ExceptionRecord.ExceptionCode != 0x80000003 &&
                dbe.u.Exception.ExceptionRecord.ExceptionCode != 0x4000001F)
        {
            RuntimeErrorCode_(dbe.u.Exception.ExceptionRecord.ExceptionCode, code_state);
            SetError_(process, proc_info.dwProcessId, code_state, RUNTIME_ERROR);
            return true;
        }
    }

    // may continue debug
    ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, DBG_CONTINUE);
    if(dbe.u.CreateProcessInfo.hFile) CloseHandle(dbe.u.CreateProcessInfo.hFile);
    if(dbe.u.CreateProcessInfo.hProcess) CloseHandle(dbe.u.CreateProcessInfo.hProcess);
    if(dbe.u.CreateProcessInfo.hThread) CloseHandle(dbe.u.CreateProcessInfo.hThread); 
    code_state.state = CONTINUE;
    return false;
}
