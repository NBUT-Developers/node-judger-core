#include "common.h"
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fcntl.h>
#include <psapi.h>
#include <sys/stat.h>
#include <tlhelp32.h>
#pragma comment(ib, "psapi.lib")

std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD error_message_id = GetLastError();
    if(error_message_id == 0) return std::string(); // No error message has been recorded

    LPSTR message_buffer = nullptr;
    size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error_message_id,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message_buffer, 0, NULL);

    std::string message(message_buffer, size);

    // Free the buffer.
    LocalFree(message_buffer);

    return message;
}

HANDLE CreateChildProcess_(
        const char* exe_path,
        const char* command,
        const char* input,
        const char* output,
        CodeState& code_state,
        HANDLE& input_handle,
        HANDLE& output_handle,
        PROCESS_INFORMATION& proc_info)
{
    // The SECURITY_ATTRIBUTES structure contains the security descriptor for an object
    // and specifies whether the handle retrieved by specifying this structure is
    // inheritable. This structure provides security settings for objects created by
    // various functions, such as CreateFile, CreatePipe, CreateProcess, RegCreateKeyEx,
    // or RegSaveKeyEx.
    //
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa379560(v=vs.85).aspx
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor= NULL;
    sa.bInheritHandle = true;

    // create file handles use SECURITY_ATRIBUTES
    //
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx
    input_handle = output_handle = NULL;
    input_handle = CreateFile(input, GENERIC_READ, 0, &sa, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL);
    output_handle = CreateFile(output, GENERIC_WRITE, 0, &sa, OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);

    if(input_handle == INVALID_HANDLE_VALUE ||
            output_handle == INVALID_HANDLE_VALUE)
    {
        code_state.state = SYSTEM_ERROR;
        strcpy(code_state.error_code, GetLastErrorAsString().c_str());
        return NULL;
    }

    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms686331(v=vs.85).aspx
    //
    // + dwFlags
    //   1. STARTF_USESTDHANDLES
    //     The hStdInput, hStdOutput, and hStdError members contain additional information.
    //     If this flag is specified when calling one of the process creation functions, the
    //     handles must be inheritable and the function's bInheritHandles parameter must be
    //     set to TRUE. For more information, see Handle Inheritance.
    //     If this flag is specified when calling the GetStartupInfo function, these members
    //     are either the handle value specified during process creation or
    //     INVALID_HANDLE_VALUE.
    //     Handles must be closed with CloseHandle when they are no longer needed.
    //   2. STARTF_USESHOWWINDOW
    //     The wShowWindow member contains additional information.
    STARTUPINFO startup_info = { sizeof(startup_info) };
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startup_info.hStdInput = input_handle;
    startup_info.hStdOutput = output_handle;
    startup_info.hStdError = NULL;
    startup_info.wShowWindow = SW_HIDE;

    // create the child process
    //
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms682425(v=vs.85).aspx
    PROCESS_INFORMATION proc_info_;
    bool flag = CreateProcessA(exec_path, (LPSTR)command, NULL, NULL, true,
            DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &startup_info, &proc_info_);

    if(!flg)
    {
        code_state.state = SYSTEM_ERROR;
        strcpy(code_state.error_code, GetLastErrorAsString().c_str());
        return NULL;
    }

    proc_info = proc_info_;
    return proc_info.hProcess;
}