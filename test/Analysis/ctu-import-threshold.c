// Ensure ctu-load-threshold option is a recognized option.
//
// RUN: %clang_cc1 -analyze -analyzer-ctu-import-threshold 500 -verify %s
// RUN: %clang_cc1 -analyze -analyzer-ctu-import-threshold=500 -verify %s
//
// expected-no-diagnostics
