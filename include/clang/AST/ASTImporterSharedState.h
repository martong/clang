//===- ASTImporterSharedState.h - ASTImporter specific state --*- C++ -*---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the ASTImporter specific state, which may be shared
//  amongst several ASTImporter objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_ASTIMPORTERSHAREDSTATE_H
#define LLVM_CLANG_AST_ASTIMPORTERSHAREDSTATE_H

#include "clang/AST/ASTImporterLookupTable.h"
#include "llvm/ADT/DenseMap.h"
// FIXME We need this because of ImportError.
#include "clang/AST/ASTImporter.h"

namespace clang {

class TranslationUnitDecl;

/// Importer specific state, which may be shared amongst several ASTImporter
/// objects.
class ASTImporterSharedState {

  /// Pointer to the import specific lookup table.  This is an externally
  /// managed resource (and should exist during the lifetime of the
  /// ASTImporter object) If not set then the original C/C++ lookup is used.
  ASTImporterLookupTable LookupTable;

  /// Mapping from the already-imported declarations in the "to"
  /// context to the error status of the import of that declaration.
  /// This map contains only the declarations that were not correctly
  /// imported. The same declaration may or may not be included in
  /// ImportedFromDecls. This map is updated continuously during imports and
  /// never cleared (like ImportedFromDecls).
  llvm::DenseMap<Decl *, ImportError> ImportErrors;

  // FIXME put ImportedFromDecls here!
  // And from that point we can better encapsulate the lookup table.

public:
  ASTImporterSharedState(TranslationUnitDecl &ToTU) : LookupTable(ToTU) {}
  ASTImporterLookupTable &getLookupTable() { return LookupTable; }

  llvm::Optional<ImportError> getImportDeclErrorIfAny(Decl *ToD) const {
    auto Pos = ImportErrors.find(ToD);
    if (Pos != ImportErrors.end())
      return Pos->second;
    else
      return Optional<ImportError>();
  }

  void setImportDeclError(Decl *To, ImportError Error) {
    ImportErrors[To] = Error;
  }
};

} // namespace clang
#endif // LLVM_CLANG_AST_ASTIMPORTERSHAREDSTATE_H
