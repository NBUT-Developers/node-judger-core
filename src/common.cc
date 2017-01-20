#include "common.h"
#include <v8.h>
using namespace std;

namespace NodeJudger {

string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD error_message_id = GetLastError();

    // No error message has been recorded
    if(error_message_id == 0) return std::string();

    LPSTR message_buffer = nullptr;
    size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error_message_id,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&message_buffer, 0, NULL);

    std::string message(message_buffer, size);

    // Free the buffer.
    LocalFree(message_buffer);

    return message;
}

string GetEnvironmentVar(LPCSTR key)
{
    // use GetEnvironmentVariable
    // refer to https://github.com/nodejs/node/blob/v6.9.4/src/node.cc#L2736
    char buffer[32767];
    DWORD result = GetEnvironmentVariableA(key, buffer, sizeof(buffer));

    // If result >= sizeof buffer the buffer was too small. That should never
    // happen. If result == 0 and result != ERROR_SUCCESS the variable was not
    // not found.
    if((result > 0 || GetLastError() == ERROR_SUCCESS) &&
            result < sizeof(buffer))
    {
        return buffer;
    }

    return "";
}

string GetTempDirectory()
{
    // refet to https://github.com/nodejs/node/blob/v6.9.4/lib/os.js#L34
    string path = "";
    path = GetEnvironmentVar("TEMP");
    if(!path.size()) path = GetEnvironmentVar("TMP");
    if(!path.size()) path = GetEnvironmentVar("SystemRoot") + "\\temp";
    if(!path.size() || path == "\\temp") path = GetEnvironmentVar("windir") + "\\temp";

    if(path.size() && path[path.size() - 1] == '\\' && (path.size() < 2 || path[path.size() - 2] != ':'))
    {
        path = path.substr(0, path.size() - 1);
    }

    return path;
}

}