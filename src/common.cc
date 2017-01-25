#include "common.h"
#include <v8.h>
#include <Psapi.h>
#include <vector>
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

string GetDLLNameFromDebug(const LOAD_DLL_DEBUG_INFO& load_dll_info)
{
    BOOL success = FALSE;
    char filename[MAX_PATH + 1];
    HANDLE file_map;

    DWORD file_size_hi = 0;
    DWORD file_size_lo = GetFileSize(load_dll_info.hFile, &file_size_hi);

    if(file_size_lo == 0 && file_size_hi == 0)
    {
        return "";
    }

    file_map = CreateFileMapping(load_dll_info.hFile,
            NULL,
            PAGE_READONLY,
            0,
            1,
            NULL);

    if(file_map)
    {
        void* mem = MapViewOfFile(file_map, FILE_MAP_READ, 0, 0, 1);

        if(mem)
        {
            if(GetMappedFileName(GetCurrentProcess(),
                    mem,
                    filename,
                    MAX_PATH))
            {
                UnmapViewOfFile(mem);
                CloseHandle(file_map);
                return filename;
            }
        }

        UnmapViewOfFile(mem);
        CloseHandle(file_map);
    }

    return "";
}

vector<string> __dlls;
void AddSupportedDLLName(string name)
{
    __dlls.push_back(name);
}

bool IsDLLSupported(string dll_path)
{
    for(int i = 0; i < __dlls.size(); i++)
    {
        if(dll_path.indexOf(__dlls[i]) != string::npos)
        {
            return true;
        }
    }

    return false;
}

}