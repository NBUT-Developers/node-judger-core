/**
 * =====================================================================================
 *
 *       Filename:  exe_runner.cxx
 *
 *    Description:  The *.exe runner.
 *
 *        Version:  1.0
 *        Created:  3/21/2015 6:44:12 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  XadillaX (admin@xcoder.in)
 *   Organization:  Ningbo University of Technology
 *
 * =====================================================================================
 */
#include "exe_runner.h"
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <psapi.h>
#include <sys/stat.h>
#include <tlhelp32.h>
#pragma comment (lib, "psapi.lib")

HANDLE RunExe_(const char* exe_path,
        const char* command,
        const char* input,
        const char* output,
        CodeState& code_state,
        HANDLE& input_handle,
        HANDLE& output_handle,
        PROCESS_INFORMATION& proc_info)
{
    SECURITY_ATTRIBUTES sa;
    sa.bInheritHandle = true;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;

    // create file handles
    input_handle = output_handle = NULL;
    input_handle = CreateFile(input, GENERIC_READ, NULL, &sa, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL);
    output_handle = CreateFile(output, GENERIC_READ | GENERIC_WRITE, NULL, &sa,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // can't open file corretly
    if(!input_handle || !output_handle || 
            input_handle == INVALID_HANDLE_VALUE ||
            output_handle == INVALID_HANDLE_VALUE)
    {
        code_state.state = SYSTEM_ERROR;
        strcpy(code_state.error_code, "File error.");
        return NULL;
    }

    PROCESS_INFORMATION proc_info_;
    STARTUPINFO startup_info = { sizeof(startup_info) };
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startup_info.hStdInput = input_handle;
    startup_info.hStdOutput = output_handle;
    startup_info.wShowWindow = SW_HIDE;

    // Create the child process
    bool flag = CreateProcessA(exe_path, (LPSTR)command, NULL, NULL, true,
            DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &startup_info, &proc_info_);

    if(!flag)
    {
        code_state.state = SYSTEM_ERROR;
        strcpy(code_state.error_code, "Can't create child process.");
        return NULL;
    }

    proc_info = proc_info_;
    return proc_info.hProcess;
}

void Terminate_(PROCESS_INFORMATION proc_info)
{
    try
    {
        if(proc_info.dwProcessId)
        {
            DebugActiveProcessStop(proc_info.dwProcessId);
        }

        if(proc_info.hProcess)
        {
            TerminateProcess(proc_info.hProcess, 0);
        }

        if(proc_info.hThread)
        {
            CloseHandle(proc_info.hThread);
        }

        if(proc_info.hProcess)
        {
            CloseHandle(proc_info.hProcess);
        }
    }
    catch(...)
    {
        // ignore...
    }
}

PROCESS_INFORMATION* RunExe(const char* exe_path,
        const char* command,
        const char* std_input_path,
        const char* output_path,
        CodeState& code_state)
{
    PROCESS_INFORMATION proc_info;
    HANDLE input_handle, output_handle;
    HANDLE proc_handle = RunExe_(exe_path,
            command,
            std_input_path,
            output_path,
            code_state,
            input_handle,
            output_handle,
            proc_info);

    if(input_handle) CloseHandle(input_handle);
    if(output_handle) CloseHandle(output_handle);

    if(NULL == proc_handle)
    {
        Terminate_(proc_info);
        return NULL;
    }

    PROCESS_INFORMATION* result = new PROCESS_INFORMATION();
    memcpy(result, &proc_info, sizeof(PROCESS_INFORMATION));

    return result;
}
