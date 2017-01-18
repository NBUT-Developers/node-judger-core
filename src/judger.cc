#include <nan.h>
#include <string>
#include "common.h"
#include "runner.h"

namespace NodeJudger {

class JudgeWorker : public Nan::AsyncWorker {
public:
    JudgeWorker(Nan::Callback* callback,
            const char* exe_path,
		    const char* command,
            const char* input,
            const char* std_output,
            __int64 time_limit,
            __size memo_limit) :
        AsyncWorker(callback),
        exe_path(exe_path),
		command(command),
        input(input),
        std_output(std_output),
        time_limit(time_limit),
        memo_limit(memo_limit),

        child_process_handle(INVALID_HANDLE_VALUE)
    {
    }

    void Execute()
    {
        child_process_handle = RunExecutable(exe_path.c_str(), command.c_str(),
                input.c_str(),
                "temp.out",
                code_state);

        if(!VALID_HANDLE(child_process_handle))
        {
            return;
        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;
        callback->Call(0, NULL);
    }

private:
    std::string exe_path;
	std::string command;
	std::string input;
	std::string std_output;
    __int64 time_limit;
    __size memo_limit;

    HANDLE child_process_handle;
    CodeState code_state;
};

NAN_METHOD(Judge)
{

}

}

void InitJudge(v8::Local<v8::Object> exports)
{
    Nan::Set(exports,
            Nan::New<v8::String>("judge").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(NodeJudger::Judge)).ToLocalChecked());
}

NODE_MODULE(judger, InitJudge);