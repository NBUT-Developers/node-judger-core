/**
 * XadillaX created at 2015-03-21 20:03:18
 *
 * Copyright (c) 2015 Huaban.com, all rights
 * reserved
 */
var _judger = require("../build/Release/judger");

exports.WATCH_CODE = [
    "FINISHED",
    "CONTINUE",
    "TIME_LIMIT_EXCEEDED_1",
    "TIME_LIMIT_EXCEEDED_2",
    "MEMORY_LIMIT_EXCEEDED",
    "OUTPUT_LIMIT_EXCEEDED",
    "RUNTIME_ERROR",
    "SYSTEM_ERROR"
];

/**
 * get the answer state
 * @param {String} stdOutput the standard output filename
 * @param {String} output the output filename
 * @return {Object} the answer state
 */
exports.answerState = function(stdOutput, output) {
    var state = _judger.answerState(stdOutput, output);
    if(state.indexOf(":") < 0) return { code: state };
    else {
        state = state.split(":").map(function(v) {
            return v.trim();
        });

        return {
            code: state[0],
            msg: state[1]
        };
    }
};

/**
 * run an executable file
 * @param {String} exePath the executable file's path
 * @param {String} [command] the command
 * @param {String} stdInput the standard input file path
 * @param {String} output the output file path
 * @param {Function} the callback function
 * @return {Buffer} the process information buffer
 */
exports.runExe = function(exePath, command, stdInput, output, callback) {
    if(arguments.length < 4) {
        throw new Error("Error arguments number.");
    }

    if(arguments.length === 4) {
        callback = output;
        output = stdInput;
        stdInput = command;
        command = "";
    }

    process.nextTick(function() {
        var buf;
        try {
            handle = _judger.runExe(exePath, command, stdInput, output);
        } catch(e) {
            return callback(e);
        }
        callback(undefined, handle);
    });
};

/**
 * watch the process
 * @param {Number} handle the handle of process information
 * @param {Number} timeLimit time limit
 * @param {Number} memoLimit memory limit
 * @param {Function} callback the callback function
 */
exports.watchProcess = function(handle, timeLimit, memoLimit, callback) {
    var maxMemo = -1;

    function _watch() {
        var obj = _judger.watchCode(handle, timeLimit, memoLimit);
        var state = exports.WATCH_CODE[obj.code];
        if(state === "CONTINUE") {
            maxMemo = Math.max(maxMemo, obj.memo);
            return process.nextTick(_watch);
        }

        _judger.releaseProcHandle(handle);
        obj.memo = Math.max(maxMemo, obj.memo);
        obj.code = state;

        callback(undefined, obj);
    }

    process.nextTick(_watch);
};

