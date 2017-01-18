#include "common.h"
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
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message_buffer, 0, NULL);

    std::string message(message_buffer, size);

    // Free the buffer.
    LocalFree(message_buffer);

    return message;
}

}