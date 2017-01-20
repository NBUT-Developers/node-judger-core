#include <nan.h>
#include <string>
#include "common.h"
#include "runner.h"
#include "watcher.h"

extern "C" {
#include "third_party\\uuid4\\src\\uuid4.h"
}

namespace NodeJudger {

std::string temp_dir = GetTempDirectory();

class RunWorker : public Nan::AsyncWorker {
public:
    RunWorker(Nan::Callback* callback,
            const char* exe_path,
            const char* command,
            const char* input,
            __int64 time_limit,
            __size memo_limit) :
        AsyncWorker(callback),
        exe_path(exe_path),
        command(command),
        input(input),
        time_limit(time_limit),
        memo_limit(memo_limit),

        child_process_handle(INVALID_HANDLE_VALUE),
        code_state(),
        errored(false)
    {
    }

    void Execute()
    {
        char uuid[UUID4_LEN];
        uuid4_generate(uuid);
        output = temp_dir + "\\node-judger-" + uuid + ".out";

        child_process_handle = RunExecutable(exe_path.c_str(), command.c_str(),
                input.c_str(),
                output.c_str(),
                code_state);

        if(!VALID_HANDLE(child_process_handle))
        {
            errored = true;
            return;
        }

        bool result = WatchProcess(child_process_handle, time_limit,
                memo_limit, code_state);
        errored = !result;
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        // create result object first
        v8::Local<v8::Object> result = Nan::New<v8::Object>();

        // TODO: let number more accurate
        result->Set(Nan::New<v8::String>("time").ToLocalChecked(),
                Nan::New<v8::Int32>((int)code_state.exe_time));
        result->Set(Nan::New<v8::String>("memory").ToLocalChecked(),
                Nan::New<v8::Int32>((int)code_state.exe_memory));
        result->Set(Nan::New<v8::String>("state").ToLocalChecked(),
                Nan::New<v8::Int32>((int)code_state.state));
        result->Set(Nan::New<v8::String>("message").ToLocalChecked(),
                Nan::New<v8::String>(code_state.error_code).ToLocalChecked());
        result->Set(Nan::New<v8::String>("output").ToLocalChecked(),
                Nan::New<v8::String>(output).ToLocalChecked());

        v8::Local<v8::Value> first;
        if(errored)
        {
            SAFE_CLOSE_HANDLE(child_process_handle);
            v8::Local<v8::Object> error = Nan::Error(code_state.error_code)->ToObject();
            error->Set(Nan::New<v8::String>("code").ToLocalChecked(), Nan::New<v8::Int32>(code_state.state));
            first = error;
        }
        else
        {
            first = Nan::Undefined();
        }

        v8::Local<v8::Value> argv[3] = { first, result };
        callback->Call(2, argv);
    }

private:
    std::string exe_path;
    std::string command;
    std::string input;
    std::string output;
    __int64 time_limit;
    __size memo_limit;

    HANDLE child_process_handle;
    CodeState code_state;
    bool errored;
};

NAN_METHOD(Run)
{
    v8::String::Utf8Value v8_exe_path(info[0]->ToString());
    v8::String::Utf8Value v8_command(info[1]->ToString());
    v8::String::Utf8Value v8_input(info[2]->ToString());
    __int64 time_limit = info[3]->ToInt32()->Int32Value();
    __size memo_limit = info[4]->ToInt32()->Int32Value();
    Nan::Callback* callback = new Nan::Callback(info[5].As<v8::Function>());

    Nan::AsyncQueueWorker(new RunWorker(callback,
            *v8_exe_path, 
            *v8_command, 
            *v8_input,
            time_limit,
            memo_limit));
}

}

void InitJudge(v8::Local<v8::Object> exports)
{
    Nan::Set(exports,
            Nan::New<v8::String>("run").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(NodeJudger::Run)).ToLocalChecked());
}

NODE_MODULE(judger, InitJudge);