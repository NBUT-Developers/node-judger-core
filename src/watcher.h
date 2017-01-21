#ifndef __WATCHER_H__
#define __WATCHER_H__
#include "common.h"

namespace NodeJudger {

// Some useful code
//
// https://www.codeproject.com/Articles/43682/Writing-a-basic-Windows-debugger
bool WatchProcess(const HANDLE process,
        const __int64 time_limit,
        const __size memo_limit,
        CodeState& code_state);

};

#endif