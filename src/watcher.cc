#include "common.h"
#include "watcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <Psapi.h>
#pragma comment (lib, "psapi.lib")

namespace NodeJudger {

#define EXCEPTION_WX86_BREAKPOINT 0x4000001F
#define MAX_TIME_LIMIT_DELAY 200
const bool __WATCHER_PRINT_DEBUG = (GetEnvironmentVar("JUDGE_DEBUG") == "true");
#define MAX(a, b) ((a) > (b) ? (a) : (b)

inline __int64 GetRunTime_(HANDLE process)
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

void ExitAndSetError(HANDLE process, CodeState& code_state, StateEnum state_enum, const char* code = NULL)
{
    DWORD process_id = GetProcessId(process);
    if(process_id) DebugActiveProcessStop(process_id);

    // refer to https://msdn.microsoft.com/en-us/library/windows/desktop/ms686714(v=vs.85).aspx
    //
    // TerminateProcess is asynchronous; it initiates termination and returns immediately. If you
    // need to be sure the process has terminated, call the WaitForSingleObject function with a
    // handle to the process.
    TerminateProcess(process, 4);
    WaitForSingleObject(process, 2000);

    code_state.state = state_enum;
    if(code && strlen(code) != 0)
    {
        strcpy(code_state.error_code, code);
    }
}

void SetRuntimeErrorCode_(CodeState& code_state, DWORD code)
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

bool WatchProcess(const HANDLE process,
        const __int64 time_limit,
        const __size memo_limit,
        CodeState& code_state)
{
    __int64 run_time = 0;
    __size run_memo = 0;

    // refer to https://msdn.microsoft.com/en-us/library/windows/desktop/ms679308(v=vs.85).aspx
    //
    // Describes a debugging event.
    //
    // typedef struct _DEBUG_EVENT {
    //     DWORD dwDebugEventCode;
    //     DWORD dwProcessId;
    //     DWORD dwThreadId;
    //     union {
    //         EXCEPTION_DEBUG_INFO      Exception;
    //         CREATE_THREAD_DEBUG_INFO  CreateThread;
    //         CREATE_PROCESS_DEBUG_INFO CreateProcessInfo;
    //         EXIT_THREAD_DEBUG_INFO    ExitThread;
    //         EXIT_PROCESS_DEBUG_INFO   ExitProcess;
    //         LOAD_DLL_DEBUG_INFO       LoadDll;
    //         UNLOAD_DLL_DEBUG_INFO     UnloadDll;
    //         OUTPUT_DEBUG_STRING_INFO  DebugString;
    //         RIP_INFO                  RipInfo;
    //     } u;
    // } DEBUG_EVENT, *LPDEBUG_EVENT;
    DEBUG_EVENT dbe;

    while(true)
    {
        // refer to https://msdn.microsoft.com/en-us/library/windows/desktop/ms681423(v=vs.85).aspx
        //
        // Waits for a debugging event to occur in a process being debugged.
        //
        // If the function succeeds, the return value is nonzero.
        // If the function fails, the return value is zero.To get extended error information, call GetLastError.
        BOOL flag = WaitForDebugEvent(&dbe, (DWORD)time_limit + MAX_TIME_LIMIT_DELAY - run_time);
        if(!flag)
        {
            std::string error = "Cannot wait for debug event: " + GetLastErrorAsString();
            code_state.exe_time = time_limit + MAX_TIME_LIMIT_DELAY;
            code_state.exe_memory = MAX(GetRunMemo_(process), code_state.exe_memory);
            ExitAndSetError(process, code_state, TIME_LIMIT_EXCEEDED_2, error.c_str());
            return false;
        }

        // refer to http://www.debuginfo.com/examples/src/DebugEvents.cpp
        //
        // close some handles inside dbe
        switch(dbe.dwDebugEventCode)
        {
        case CREATE_PROCESS_DEBUG_EVENT:
            if(__WATCHER_PRINT_DEBUG)
            {
                printf("Event: Process creation\n");
                printf("  CREATE_PROCESS_DEBUG_INFO members:\n");
                printf("    hFile:                  %08p\n", dbe.u.CreateProcessInfo.hFile);
                printf("    hProcess:               %08p\n", dbe.u.CreateProcessInfo.hProcess);
                printf("    hThread                 %08p\n", dbe.u.CreateProcessInfo.hThread);
            }

            // With this event, the debugger receives the following handles:
            //   CREATE_PROCESS_DEBUG_INFO.hProcess - debuggee process handle
            //   CREATE_PROCESS_DEBUG_INFO.hThread  - handle to the initial thread of the debuggee process
            //   CREATE_PROCESS_DEBUG_INFO.hFile    - handle to the executable file that was 
            //                                        used to create the debuggee process (.EXE file)
            // 
            // hProcess and hThread handles will be closed by the operating system 
            // when the debugger calls ContinueDebugEvent after receiving 
            // EXIT_PROCESS_DEBUG_EVENT for the given process
            // 
            // hFile handle should be closed by the debugger, when the handle 
            // is no longer needed
            SAFE_CLOSE_HANDLE(dbe.u.CreateProcessInfo.hFile);
            break;

        case CREATE_THREAD_DEBUG_EVENT:
            // With this event, the debugger receives the following handle:
            //   CREATE_THREAD_DEBUG_INFO.hThread  - handle to the thread that has been created
            // 
            // This handle will be closed by the operating system 
            // when the debugger calls ContinueDebugEvent after receiving 
            // EXIT_THREAD_DEBUG_EVENT for the given thread
            break;

        case LOAD_DLL_DEBUG_EVENT:
        {
            std::string dll_name = GetDLLNameFromDebug(dbe.u.LoadDll);
            if(__WATCHER_PRINT_DEBUG)
            {
                printf("Event: DLL loaded\n");
                printf("  LOAD_DLL_DEBUG_INFO members:\n");
                printf("    hFile:                  %08p\n", dbe.u.LoadDll.hFile);
                printf("    lpBaseOfDll:            %08p\n", dbe.u.LoadDll.lpBaseOfDll);
                printf("    DLLName:                %s\n", dll_name.c_str());
            }

            // With this event, the debugger receives the following handle:
            //   LOAD_DLL_DEBUG_INFO.hFile    - handle to the DLL file 
            // 
            // This handle should be closed by the debugger, when the handle 
            // is no longer needed
            SAFE_CLOSE_HANDLE(dbe.u.LoadDll.hFile);

            // And what's more
            // users are not allowed to load DLL
            if(dll_name.find("\\ntdll.dll") == std::string::npos &&
                    dll_name.find("\\kernel32.dll") == std::string::npos &&
                    dll_name.find("\\KernelBase.dll") == std::string::npos &&
                    dll_name.find("\\msvcrt.dll") == std::string::npos &&
                    dll_name.find("\\wow64.dll") == std::string::npos &&
                    dll_name.find("\\wow64win.dll") == std::string::npos &&
                    dll_name.find("\\wow64cpu.dll") == std::string::npos &&
                    dll_name.find("\\user32.dll") == std::string::npos)
            {
                std::string error = "Code is up to load DLL.";
                code_state.exe_time = 0;
                code_state.exe_memory = 0;
                ExitAndSetError(process, code_state, DANGEROUS_CODE, error.c_str());
                return false;
            }
        }

        default: break;
        }

        // get the run time
        run_time = GetRunTime_(process);
        if(-1 == run_time)
        {
            std::string error = "Cannot get running time: " + GetLastErrorAsString();
            code_state.exe_time = 0;
            code_state.exe_memory = MAX(GetRunMemo_(process), code_state.exe_memory);
            ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, DBG_CONTINUE);
            ExitAndSetError(process, code_state, SYSTEM_ERROR, error.c_str());
            return false;
        }

        // get the run memory
        run_memo = GetRunMemo_(process);
        if(-1 == run_memo)
        {
            std::string error = "Cannot get occupied memory: " + GetLastErrorAsString();
            code_state.exe_time = run_time;
            code_state.exe_memory = 0;
            ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, DBG_CONTINUE);
            ExitAndSetError(process, code_state, SYSTEM_ERROR, error.c_str());
            return false;
        }

        code_state.exe_time = run_time;
        code_state.exe_memory = MAX(code_state.exe_memory, run_memo);

        if(run_time > time_limit)
        {
            ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, DBG_CONTINUE);
            ExitAndSetError(process, code_state, TIME_LIMIT_EXCEEDED_1);
            return false;
        }

        if(run_memo > memo_limit)
        {
            ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, DBG_CONTINUE);
            ExitAndSetError(process, code_state, MEMORY_LIMIT_EXCEEDED);
            return false;
        }

        // do some reaction via each DEBUG_EVENT
        switch(dbe.dwDebugEventCode)
        {
        case EXIT_PROCESS_DEBUG_EVENT:
            code_state.state = FINISHED;
            ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, DBG_CONTINUE);
            return true;

        case EXCEPTION_DEBUG_EVENT:
            if(dbe.u.Exception.ExceptionRecord.ExceptionCode != EXCEPTION_BREAKPOINT &&
                    dbe.u.Exception.ExceptionRecord.ExceptionCode != EXCEPTION_WX86_BREAKPOINT)
            {
                SetRuntimeErrorCode_(code_state, dbe.u.Exception.ExceptionRecord.ExceptionCode);
                ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
                ExitAndSetError(process, code_state, RUNTIME_ERROR);
                return false;
            }
            break;

        default: break;
        }

        ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, DBG_CONTINUE);
        code_state.state = CONTINUE;
    }
}

};