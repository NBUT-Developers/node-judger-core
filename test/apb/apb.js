/**
 * XadillaX created at 2015-03-21 20:11:05
 *
 * Copyright (c) 2015 Huaban.com, all rights
 * reserved
 */
var judger = require("../../lib/judger");

judger.runExe(
    __dirname + "\\apb.exe",
    __dirname + "\\std.in",
    __dirname + "\\std.out", function(err, buf) {
    console.log(err);
    console.log(buf);
});
