/**
 * =====================================================================================
 *
 *       Filename:  judger.cxx
 *
 *    Description:  The judger for Node.js
 *
 *        Version:  1.0
 *        Created:  3/21/2015 6:12:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  XadillaX (admin@xcoder.in)
 *   Organization:  Ningbo University of Technology
 *
 * =====================================================================================
 */
#include <node.h>
#include <v8.h>
#include <nan.h>
#include "exe_runner.h"
#include "proc_watcher.h"
using namespace v8;

NAN_METHOD(RunExe)
{
    NanScope();

    if(args.Length() != 4)
    {
        NanThrowError("Wrong number of arguments.");
        NanReturnUndefined();
    }

    Local<String> exe_path = NanNew(*NanAsciiString(args[0]));
    Local<String> command = NanNew(*NanAsciiString(args[1]));
    Local<String> std_input = NanNew(*NanAsciiString(args[2]));
    Local<String> output = NanNew(*NanAsciiString(args[3]));

    char* s_exe_path = new char[exe_path->Length() + 1];
    char* s_command = command->Length() ? new char[command->Length() + 1] : NULL;
    char* s_std_input = new char[std_input->Length() + 1];
    char* s_output = new char[output->Length() + 1];

    exe_path->WriteAscii(s_exe_path);
    if(s_command) command->WriteAscii(s_command);
    std_input->WriteAscii(s_std_input);
    output->WriteAscii(s_output);

    CodeState code_state;
    code_state.error_code[0] = '\0';
    code_state.exe_time = code_state.exe_memory = 0;
    PROCESS_INFORMATION* proc_info = RunExe(s_exe_path,
            s_command,
            s_std_input,
            s_output,
            code_state);

    delete []s_exe_path;
    if(s_command) delete []s_command;
    delete []s_std_input;
    delete []s_output;

    if(!proc_info)
    {
        NanThrowError(code_state.error_code, (int)code_state.state);
        NanReturnUndefined();
    }

    Local<Integer> proc_handle = NanNew<Integer>((int)proc_info);
    NanReturnValue(proc_handle);
}

NAN_METHOD(WatchCode)
{
    NanScope();

    if(args.Length() != 3)
    {
        NanThrowError("Wrong number of arguments.");
        NanReturnUndefined();
    }

    PROCESS_INFORMATION* proc_handle = (PROCESS_INFORMATION*)args[0]->Uint32Value();
    __int64 time_limit = args[1]->Uint32Value();
    __int64 memo_limit = args[2]->Uint32Value();

    CodeState code_state;
    code_state.error_code[0] = '\0';
    WatchCode(proc_handle->hProcess, time_limit, memo_limit, code_state, *proc_handle);

    Local<Object> obj = NanNew<Object>();
    obj->Set(NanNew<String>("time"), NanNew<Integer>((int)code_state.exe_time));
    obj->Set(NanNew<String>("memo"), NanNew<Integer>((int)code_state.exe_memory));
    obj->Set(NanNew<String>("code"), NanNew<Integer>((int)code_state.state));
    if(code_state.error_code[0] != '\0')
    {
        obj->Set(NanNew<String>("msg"), NanNew<String>(code_state.error_code));
    }

    if(code_state.state != FINISHED && code_state.state != CONTINUE)
    {
        DebugActiveProcessStop(proc_handle->dwProcessId);
        TerminateProcess(proc_handle->hProcess, 4);
    }

    NanReturnValue(obj);
}

NAN_METHOD(ReleaseProcHandle)
{
    NanScope();

    if(args.Length() != 1)
    {
        NanThrowError("Wrong number of arguments.");
        NanReturnUndefined();
    }

    PROCESS_INFORMATION* proc_handle = (PROCESS_INFORMATION*)args[0]->Uint32Value();
   
    try
    {
        CloseHandle(proc_handle->hThread);
        CloseHandle(proc_handle->hProcess);
        delete proc_handle;
    }
    catch(...)
    {
        // ignore...
    }

    NanReturnUndefined();
}

void Init(Handle<Object> exports)
{
    srand((unsigned)time(NULL));

    exports->Set(NanNew<String>("runExe"),
            NanNew<FunctionTemplate>(RunExe)->GetFunction());
    exports->Set(NanNew<String>("watchCode"),
            NanNew<FunctionTemplate>(WatchCode)->GetFunction());
    exports->Set(NanNew<String>("releaseProcHandle"),
            NanNew<FunctionTemplate>(ReleaseProcHandle)->GetFunction());
}

NODE_MODULE(judger, Init);

