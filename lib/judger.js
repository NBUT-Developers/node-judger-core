/**
 * XadillaX created at 2015-03-21 20:03:18
 *
 * Copyright (c) 2015 Huaban.com, all rights
 * reserved
 */
var _judger = require("../build/Release/judger");

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

    console.log(exePath, command, stdInput, output, callback);

    process.nextTick(function() {
        var buf;
        try {
            buf = _judger.runExe(exePath, command, stdInput, output);
        } catch(e) {
            return callback(e);
        }
        callback(undefined, buf);
    });
};

