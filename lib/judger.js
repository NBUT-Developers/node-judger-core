/**
 * XadillaX created at 2015-03-22 22:00:30
 *
 * Copyright (c) 2015 Huaban.com, all rights
 * reserved
 */
var Judger = function(compiler) {
    this.compiler = compiler;
    if(typeof compiler === "string") {
        this.compiler = require(compiler);
    }
};

module.exports = Judger;

