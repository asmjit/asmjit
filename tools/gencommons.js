// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

let VERBOSE = false;

function DEBUG(msg) {
  if (VERBOSE)
    console.log(msg);
}
exports.DEBUG = DEBUG;

function WARN(msg) {
  console.log(msg);
}
exports.WARN = WARN;

function FATAL(msg) {
  console.log(`FATAL ERROR: ${msg}`);
  throw new Error(msg);
}
exports.FATAL = FATAL;
