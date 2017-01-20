#include "common.h"
#include "runner.h"
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fcntl.h>
#include <psapi.h>
#include <sys/stat.h>
#include <TlHelp32.h>
#pragma comment(lib, "psapi.lib")

namespace NodeJudger {

HANDLE RunExecutable(
        const char* exe_path,
        const char* command,
        const char* input,
        const char* output,
        CodeState& code_state)
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
    HANDLE input_handle, output_handle;
    input_handle = output_handle = INVALID_HANDLE_VALUE;
    input_handle = CreateFile(input, GENERIC_READ, 0, &sa, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL);
    output_handle = CreateFile(output, GENERIC_WRITE, 0, &sa, OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);

    if(!VALID_HANDLE(input_handle) || !VALID_HANDLE(output_handle))
    {
        std::string error = "Cannot open input file or output file: " + GetLastErrorAsString();
        code_state.state = SYSTEM_ERROR;
        strcpy(code_state.error_code, error.c_str());
        SAFE_CLOSE_HANDLE(input_handle);
        SAFE_CLOSE_HANDLE(output_handle);
        input_handle = INVALID_HANDLE_VALUE;
        output_handle = INVALID_HANDLE_VALUE;
        return INVALID_HANDLE_VALUE;
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
    // If the function succeeds, be sure to call the CloseHandle function to close the
    // hProcess and hThread handles when you are finished with them. Otherwise, when
    // the child process exits, the system cannot clean up the process structures for
    // the child process because the parent process still has open handles to the child
    // process. However, the system will close these handles when the parent process
    // terminates, so the structures related to the child process object would be
    // cleaned up at this point.
    //
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms682425(v=vs.85).aspx
    PROCESS_INFORMATION proc_info;
    bool flag = CreateProcessA(exe_path, (LPSTR)command, NULL, NULL, true,
            DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &startup_info, &proc_info);

    SAFE_CLOSE_HANDLE(proc_info.hThread);
    SAFE_CLOSE_HANDLE(input_handle);
    SAFE_CLOSE_HANDLE(output_handle);

    if(!flag)
    {
        std::string error = "Cannot run executable: " + GetLastErrorAsString();
        code_state.state = SYSTEM_ERROR;
        strcpy(code_state.error_code, error.c_str());
        return INVALID_HANDLE_VALUE;
    }

    return proc_info.hProcess;
}

}