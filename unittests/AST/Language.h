//===- unittest/AST/Language.h - AST unit test support ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_UNITTESTS_AST_LANGUAGE_H
#define LLVM_CLANG_UNITTESTS_AST_LANGUAGE_H

#include <vector>
#include <string>

namespace clang {
namespace ast_matchers {

enum Language {
    Lang_C,
    Lang_C89,
    Lang_CXX,
    Lang_CXX11,
    Lang_CXX14,
    Lang_OpenCL,
    Lang_OBJCXX
};

typedef std::vector<std::string> StringVector;

inline
void getLangArgs(Language Lang, StringVector &Args) {
  switch (Lang) {
  case Lang_C:
    Args.insert(Args.end(), { "-x", "c", "-std=c99" });
    break;
  case Lang_C89:
    Args.insert(Args.end(), { "-x", "c", "-std=c89" });
    break;
  case Lang_CXX:
    Args.push_back("-std=c++98");
    break;
  case Lang_CXX11:
    Args.push_back("-std=c++11");
    break;
  case Lang_CXX14:
    Args.push_back("-std=c++14");
    break;
  case Lang_OpenCL:
  case Lang_OBJCXX:
    break;
  }
}

} // end namespace ast_matchers
} // end namespace clang

#endif
