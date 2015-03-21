/**
 * =====================================================================================
 *
 *       Filename:  proc_watcher.h
 *
 *    Description:  The process watcher.
 *
 *        Version:  1.0
 *        Created:  3/21/2015 11:27:02 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  XadillaX (admin@xcoder.in)
 *   Organization:  Ningbo University of Technology
 *
 * =====================================================================================
 */
#ifndef __PROC_WATCHER_H__
#define __PROC_WATCHER_H__
#pragma once
#include "common.h"

bool WatchCode(const HANDLE process, 
        const __int64 time_limit,
        const __size memo_limit,
        CodeState& code_state,
        const PROCESS_INFORMATION proc_info);

#endif

