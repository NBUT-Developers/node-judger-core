#ifndef __RUNNER_H__
#define __RUNNER_H__

namespace NodeJudger {
HANDLE RunExecutable(
    const char* exe_path,
    const char* command,
    const char* input,
    const char* output,
    CodeState& code_state);
}

#endif