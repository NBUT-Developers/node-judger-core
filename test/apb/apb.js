/**
 * XadillaX created at 2015-03-21 20:11:05
 *
 * Copyright (c) 2015 Huaban.com, all rights
 * reserved
 */
//var wrapper = require("../../lib/cppwrapper");

// wrapper.runExe(
//     __dirname + "\\apb.exe",
//     __dirname + "\\std.in",
//     __dirname + "\\out.out", function(err, handle) {
//     wrapper.watchProcess(handle, 1000, 65536, function(err, obj) {
//         console.log(err);
//         console.log(obj);

//         var res = wrapper.answerState(__dirname + "\\std.out", __dirname + "\\out.out");
//         console.log(res);
//     });
// });
const fs = require("fs");

const Judger = require("../../build/Release/judger.node");

Judger.run(
    __dirname + "\\apb.exe",
    "",
    __dirname + "\\std.in",
    1000,
    65535,
    function(err, result) {
        console.log(err, result);
    });