#include <nan.h>
#include <string>
#include "common.h"
#include "runner.h"

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
            const char* input) :
        AsyncWorker(callback),
        exe_path(exe_path),
		command(command),
        input(input),

        child_process_handle(INVALID_HANDLE_VALUE)
    {
    }

    void Execute()
    {
		char uuid[UUID4_LEN];
		uuid4_generate(uuid);
		output = temp_dir + "\\" + uuid + ".out";

        child_process_handle = RunExecutable(exe_path.c_str(), command.c_str(),
                input.c_str(),
				output.c_str(),
                code_state);
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

		if(!VALID_HANDLE(child_process_handle))
		{
			v8::Local<v8::Value> argv[1] = {
				Nan::ErrnoException(code_state.state, "ERROR_RUN", code_state.error_code, "src/runner.cc")
			};
			callback->Call(1, argv);
			return;
		}

		v8::Local<v8::Value> argv[3] = {
			Nan::Undefined(),
			Nan::New<v8::Uint32>((unsigned int)child_process_handle),
			Nan::New<v8::String>(output).ToLocalChecked()
		};
        callback->Call(3, argv);
    }

private:
    std::string exe_path;
	std::string command;
	std::string input;
	std::string output;

    HANDLE child_process_handle;
    CodeState code_state;
};

NAN_METHOD(Judge)
{
	v8::String::Utf8Value v8_exe_path(info[0]->ToString());
	v8::String::Utf8Value v8_command(info[1]->ToString());
	v8::String::Utf8Value v8_input(info[2]->ToString());
	Nan::Callback* callback = new Nan::Callback(info[3].As<v8::Function>());

	Nan::AsyncQueueWorker(new RunWorker(callback, *v8_exe_path, *v8_command, *v8_input));
}

}

void InitJudge(v8::Local<v8::Object> exports)
{
    Nan::Set(exports,
            Nan::New<v8::String>("run").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(NodeJudger::Judge)).ToLocalChecked());
}

NODE_MODULE(judger, InitJudge);