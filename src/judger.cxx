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

    Local<Object> proc_buffer = NanNewBufferHandle((char*)proc_info, sizeof(PROCESS_INFORMATION));
    delete proc_info;

    NanReturnValue(proc_buffer);
}

void Init(Handle<Object> exports)
{
    exports->Set(NanNew<String>("runExe"),
            NanNew<FunctionTemplate>(RunExe)->GetFunction());
}

NODE_MODULE(judger, Init);

