//===- unittest/AST/ASTImporterTest.cpp - AST node import test ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Type-parameterized tests for the correct import of redecl chains.
//
//===----------------------------------------------------------------------===//

#include "ASTImporterFixtures.h"

namespace clang {
namespace ast_matchers {

using internal::BindableMatcher;

// DeclTy: Type of the Decl to check.
// Prototype: "Prototype" (forward declaration) of the Decl.
// Definition: A definition for the Prototype.
// ConflictingPrototype: A prototype with the same name but different
// declaration.
// ConflictingDefinition: A different definition for Prototype.
// ConflictingProtoDef: A definition for ConflictingPrototype.
// getPattern: Return a matcher that matches any of Prototype, Definition,
// ConflictingPrototype, ConflictingDefinition, ConflictingProtoDef.

struct Function {
  using DeclTy = FunctionDecl;
  static constexpr auto *Prototype = "void X();";
  static constexpr auto *Definition = "void X() {}";
  BindableMatcher<Decl> getPattern() {
    return functionDecl(hasName("X"), unless(isImplicit()));
  }
};

struct Class {
  using DeclTy = CXXRecordDecl;
  static constexpr auto *Prototype = "class X;";
  static constexpr auto *Definition = "class X {};";
  static constexpr auto *ConflictingDefinition = "class X { int A; };";
  BindableMatcher<Decl> getPattern() {
    return cxxRecordDecl(hasName("X"), unless(isImplicit()));
  }
};

struct Variable {
  using DeclTy = VarDecl;
  static constexpr auto *Prototype = "extern int X;";
  static constexpr auto *ConflictingPrototype = "extern float X;";
  static constexpr auto *Definition = "int X;";
  static constexpr auto *ConflictingDefinition = "float X;";
  BindableMatcher<Decl> getPattern() { return varDecl(hasName("X")); }
};

struct FunctionTemplate {
  using DeclTy = FunctionTemplateDecl;
  static constexpr auto *Prototype = "template <class T> void X();";
  static constexpr auto *Definition =
      R"(
      template <class T> void X() {};
      // Explicit instantiation is a must because of -fdelayed-template-parsing:
      template void X<int>();
      )";
  BindableMatcher<Decl> getPattern() {
    return functionTemplateDecl(hasName("X"), unless(isImplicit()));
  }
};

struct ClassTemplate {
  using DeclTy = ClassTemplateDecl;
  static constexpr auto *Prototype = "template <class> class X;";
  static constexpr auto *ConflictingPrototype = "template <int> class X;";
  static constexpr auto *Definition = "template <class> class X {};";
  static constexpr auto *ConflictingDefinition =
      "template <class> class X { int A; };";
  static constexpr auto *ConflictingProtoDef = "template <int> class X { };";
  BindableMatcher<Decl> getPattern() {
    return classTemplateDecl(hasName("X"), unless(isImplicit()));
  }
};

struct VariableTemplate {
  using DeclTy = VarTemplateDecl;
  static constexpr auto *Prototype = "template <class T> extern T X;";
  static constexpr auto *ConflictingPrototype =
      "template <class T> extern float X;";
  static constexpr auto *Definition =
      R"(
      template <class T> T X;
      template <> int X<int>;
      )";
  static constexpr auto *ConflictingDefinition =
      R"(
      template <class T> T X;
      template <> float X<int>;
      )";
  static constexpr auto *ConflictingProtoDef =
      R"(
      template <class T> float X;
      template <> float X<int>;
      )";
  // There is no matcher for varTemplateDecl so use a work-around.
  BindableMatcher<Decl> getPattern() {
    return namedDecl(hasName("X"), unless(isImplicit()),
                     has(templateTypeParmDecl()));
  }
};

struct FunctionTemplateSpec {
  using DeclTy = FunctionDecl;
  static constexpr auto *Prototype =
      R"(
      // Proto of the primary template.
      template <class T>
      void X();
      // Proto of the specialization.
      template <>
      void X<int>();
      )";
  static constexpr auto *Definition =
      R"(
      // Proto of the primary template.
      template <class T>
      void X();
      // Specialization and definition.
      template <>
      void X<int>() {}
      )";
  BindableMatcher<Decl> getPattern() {
    return functionDecl(hasName("X"), isExplicitTemplateSpecialization());
  }
};

struct ClassTemplateSpec {
  using DeclTy = ClassTemplateSpecializationDecl;
  static constexpr auto *Prototype =
      R"(
      template <class T> class X;
      template <> class X<int>;
      )";
  static constexpr auto *Definition =
      R"(
      template <class T> class X;
      template <> class X<int> {};
      )";
  static constexpr auto *ConflictingDefinition =
      R"(
      template <class T> class X;
      template <> class X<int> { int A; };
      )";
  BindableMatcher<Decl> getPattern() {
    return classTemplateSpecializationDecl(hasName("X"), unless(isImplicit()));
  }
};

template <typename TypeParam, ASTImporter::ODRHandlingType ODRHandlingParam>
struct RedeclChain : ASTImporterOptionSpecificTestBase {

  using DeclTy = typename TypeParam::DeclTy;

  RedeclChain() { ODRHandling = ODRHandlingParam; }

  static std::string getPrototype() { return TypeParam::Prototype; }
  static std::string getConflictingPrototype() {
    return TypeParam::ConflictingPrototype;
  }
  static std::string getDefinition() { return TypeParam::Definition; }
  static std::string getConflictingDefinition() {
    return TypeParam::ConflictingDefinition;
  }
  static std::string getConflictingProtoDef() {
    return TypeParam::ConflictingProtoDef;
  }
  static BindableMatcher<Decl> getPattern() { return TypeParam().getPattern(); }

  void CheckPreviousDecl(Decl *Prev, Decl *Current) {
    ASSERT_NE(Prev, Current);
    ASSERT_EQ(&Prev->getASTContext(), &Current->getASTContext());
    EXPECT_EQ(Prev->getCanonicalDecl(), Current->getCanonicalDecl());

    // Templates.
    if (auto *PrevT = dyn_cast<TemplateDecl>(Prev)) {
      EXPECT_EQ(Current->getPreviousDecl(), Prev);
      auto *CurrentT = cast<TemplateDecl>(Current);
      ASSERT_TRUE(PrevT->getTemplatedDecl());
      ASSERT_TRUE(CurrentT->getTemplatedDecl());
      EXPECT_EQ(CurrentT->getTemplatedDecl()->getPreviousDecl(),
                PrevT->getTemplatedDecl());
      return;
    }

    // Specializations.
    if (auto *PrevF = dyn_cast<FunctionDecl>(Prev)) {
      if (PrevF->getTemplatedKind() ==
          FunctionDecl::TK_FunctionTemplateSpecialization) {
        // There may be a hidden fwd spec decl before a spec decl.
        // In that case the previous visible decl can be reached through that
        // invisible one.
        EXPECT_THAT(Prev, testing::AnyOf(
                              Current->getPreviousDecl(),
                              Current->getPreviousDecl()->getPreviousDecl()));
        auto *ToTU = Prev->getTranslationUnitDecl();
        auto *TemplateD = FirstDeclMatcher<FunctionTemplateDecl>().match(
            ToTU, functionTemplateDecl());
        auto *FirstSpecD = *(TemplateD->spec_begin());
        EXPECT_EQ(FirstSpecD->getCanonicalDecl(), PrevF->getCanonicalDecl());
        return;
      }
    }

    // The rest: Classes, Functions, etc.
    EXPECT_EQ(Current->getPreviousDecl(), Prev);
  }

  // ========================================
  // Tests when no ODR conflict should occur.
  // ========================================

  void
  TypedTest_PrototypeShouldBeImportedAsAPrototypeWhenThereIsNoDefinition() {
    Decl *FromTU = getTuDecl(getPrototype(), Lang_CXX);
    auto *FromD = FirstDeclMatcher<DeclTy>().match(FromTU, getPattern());
    ASSERT_FALSE(FromD->isThisDeclarationADefinition());

    Decl *ImportedD = Import(FromD, Lang_CXX);
    Decl *ToTU = ImportedD->getTranslationUnitDecl();

    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 1u);
    auto *ToD = LastDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_TRUE(ImportedD == ToD);
    EXPECT_FALSE(ToD->isThisDeclarationADefinition());
    if (auto *ToT = dyn_cast<TemplateDecl>(ToD)) {
      EXPECT_TRUE(ToT->getTemplatedDecl());
    }
  }

  void TypedTest_DefinitionShouldBeImportedAsADefinition() {
    Decl *FromTU = getTuDecl(getDefinition(), Lang_CXX);
    auto *FromD = FirstDeclMatcher<DeclTy>().match(FromTU, getPattern());
    ASSERT_TRUE(FromD->isThisDeclarationADefinition());

    Decl *ImportedD = Import(FromD, Lang_CXX);
    Decl *ToTU = ImportedD->getTranslationUnitDecl();

    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 1u);
    auto *ToD = LastDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_TRUE(ToD->isThisDeclarationADefinition());
    if (auto *ToT = dyn_cast<TemplateDecl>(ToD)) {
      EXPECT_TRUE(ToT->getTemplatedDecl());
    }
  }

  void TypedTest_ImportPrototypeAfterImportedPrototype() {
    Decl *FromTU = getTuDecl(getPrototype() + getPrototype(), Lang_CXX);
    auto *From0 = FirstDeclMatcher<DeclTy>().match(FromTU, getPattern());
    auto *From1 = LastDeclMatcher<DeclTy>().match(FromTU, getPattern());
    ASSERT_FALSE(From0->isThisDeclarationADefinition());
    ASSERT_FALSE(From1->isThisDeclarationADefinition());

    Decl *Imported0 = Import(From0, Lang_CXX);
    Decl *Imported1 = Import(From1, Lang_CXX);
    Decl *ToTU = Imported0->getTranslationUnitDecl();

    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 2u);
    auto *To0 = FirstDeclMatcher<DeclTy>().match(ToTU, getPattern());
    auto *To1 = LastDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_TRUE(Imported0 == To0);
    EXPECT_TRUE(Imported1 == To1);
    EXPECT_FALSE(To0->isThisDeclarationADefinition());
    EXPECT_FALSE(To1->isThisDeclarationADefinition());

    CheckPreviousDecl(To0, To1);
  }

  void TypedTest_ImportDefinitionAfterImportedPrototype() {
    Decl *FromTU = getTuDecl(getPrototype() + getDefinition(), Lang_CXX);
    auto *FromProto = FirstDeclMatcher<DeclTy>().match(FromTU, getPattern());
    auto *FromDef = LastDeclMatcher<DeclTy>().match(FromTU, getPattern());
    ASSERT_FALSE(FromProto->isThisDeclarationADefinition());
    ASSERT_TRUE(FromDef->isThisDeclarationADefinition());

    Decl *ImportedProto = Import(FromProto, Lang_CXX);
    Decl *ImportedDef = Import(FromDef, Lang_CXX);
    Decl *ToTU = ImportedProto->getTranslationUnitDecl();

    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 2u);
    auto *ToProto = FirstDeclMatcher<DeclTy>().match(ToTU, getPattern());
    auto *ToDef = LastDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_TRUE(ImportedProto == ToProto);
    EXPECT_TRUE(ImportedDef == ToDef);
    EXPECT_FALSE(ToProto->isThisDeclarationADefinition());
    EXPECT_TRUE(ToDef->isThisDeclarationADefinition());

    CheckPreviousDecl(ToProto, ToDef);
  }

  void TypedTest_ImportPrototypeAfterImportedDefinition() {
    Decl *FromTU = getTuDecl(getDefinition() + getPrototype(), Lang_CXX);
    auto *FromDef = FirstDeclMatcher<DeclTy>().match(FromTU, getPattern());
    auto *FromProto = LastDeclMatcher<DeclTy>().match(FromTU, getPattern());
    ASSERT_TRUE(FromDef->isThisDeclarationADefinition());
    ASSERT_FALSE(FromProto->isThisDeclarationADefinition());

    Decl *ImportedDef = Import(FromDef, Lang_CXX);
    Decl *ImportedProto = Import(FromProto, Lang_CXX);
    Decl *ToTU = ImportedDef->getTranslationUnitDecl();

    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 2u);
    auto *ToDef = FirstDeclMatcher<DeclTy>().match(ToTU, getPattern());
    auto *ToProto = LastDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_TRUE(ImportedDef == ToDef);
    EXPECT_TRUE(ImportedProto == ToProto);
    EXPECT_TRUE(ToDef->isThisDeclarationADefinition());
    EXPECT_FALSE(ToProto->isThisDeclarationADefinition());

    CheckPreviousDecl(ToDef, ToProto);
  }

  void TypedTest_ImportPrototypes() {
    Decl *FromTU0 = getTuDecl(getPrototype(), Lang_CXX, "input0.cc");
    Decl *FromTU1 = getTuDecl(getPrototype(), Lang_CXX, "input1.cc");
    auto *From0 = FirstDeclMatcher<DeclTy>().match(FromTU0, getPattern());
    auto *From1 = FirstDeclMatcher<DeclTy>().match(FromTU1, getPattern());
    ASSERT_FALSE(From0->isThisDeclarationADefinition());
    ASSERT_FALSE(From1->isThisDeclarationADefinition());

    Decl *Imported0 = Import(From0, Lang_CXX);
    Decl *Imported1 = Import(From1, Lang_CXX);
    Decl *ToTU = Imported0->getTranslationUnitDecl();

    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 2u);
    auto *To0 = FirstDeclMatcher<DeclTy>().match(ToTU, getPattern());
    auto *To1 = LastDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_TRUE(Imported0 == To0);
    EXPECT_TRUE(Imported1 == To1);
    EXPECT_FALSE(To0->isThisDeclarationADefinition());
    EXPECT_FALSE(To1->isThisDeclarationADefinition());

    CheckPreviousDecl(To0, To1);
  }

  void TypedTest_ImportDefinitions() {
    Decl *FromTU0 = getTuDecl(getDefinition(), Lang_CXX, "input0.cc");
    Decl *FromTU1 = getTuDecl(getDefinition(), Lang_CXX, "input1.cc");
    auto *From0 = FirstDeclMatcher<DeclTy>().match(FromTU0, getPattern());
    auto *From1 = FirstDeclMatcher<DeclTy>().match(FromTU1, getPattern());
    ASSERT_TRUE(From0->isThisDeclarationADefinition());
    ASSERT_TRUE(From1->isThisDeclarationADefinition());

    Decl *Imported0 = Import(From0, Lang_CXX);
    Decl *Imported1 = Import(From1, Lang_CXX);
    Decl *ToTU = Imported0->getTranslationUnitDecl();

    EXPECT_EQ(Imported0, Imported1);
    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 1u);
    auto *To0 = FirstDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_TRUE(Imported0 == To0);
    EXPECT_TRUE(To0->isThisDeclarationADefinition());
    if (auto *ToT0 = dyn_cast<TemplateDecl>(To0)) {
      EXPECT_TRUE(ToT0->getTemplatedDecl());
    }
  }

  void TypedTest_ImportDefinitionThenPrototype() {
    Decl *FromTUDef = getTuDecl(getDefinition(), Lang_CXX, "input0.cc");
    Decl *FromTUProto = getTuDecl(getPrototype(), Lang_CXX, "input1.cc");
    auto *FromDef = FirstDeclMatcher<DeclTy>().match(FromTUDef, getPattern());
    auto *FromProto =
        FirstDeclMatcher<DeclTy>().match(FromTUProto, getPattern());
    ASSERT_TRUE(FromDef->isThisDeclarationADefinition());
    ASSERT_FALSE(FromProto->isThisDeclarationADefinition());

    Decl *ImportedDef = Import(FromDef, Lang_CXX);
    Decl *ImportedProto = Import(FromProto, Lang_CXX);
    Decl *ToTU = ImportedDef->getTranslationUnitDecl();

    EXPECT_NE(ImportedDef, ImportedProto);
    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 2u);
    auto *ToDef = FirstDeclMatcher<DeclTy>().match(ToTU, getPattern());
    auto *ToProto = LastDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_TRUE(ImportedDef == ToDef);
    EXPECT_TRUE(ImportedProto == ToProto);
    EXPECT_TRUE(ToDef->isThisDeclarationADefinition());
    EXPECT_FALSE(ToProto->isThisDeclarationADefinition());

    CheckPreviousDecl(ToDef, ToProto);
  }

  void TypedTest_ImportPrototypeThenDefinition() {
    Decl *FromTUProto = getTuDecl(getPrototype(), Lang_CXX, "input0.cc");
    Decl *FromTUDef = getTuDecl(getDefinition(), Lang_CXX, "input1.cc");
    auto *FromProto =
        FirstDeclMatcher<DeclTy>().match(FromTUProto, getPattern());
    auto *FromDef = FirstDeclMatcher<DeclTy>().match(FromTUDef, getPattern());
    ASSERT_TRUE(FromDef->isThisDeclarationADefinition());
    ASSERT_FALSE(FromProto->isThisDeclarationADefinition());

    Decl *ImportedProto = Import(FromProto, Lang_CXX);
    Decl *ImportedDef = Import(FromDef, Lang_CXX);
    Decl *ToTU = ImportedDef->getTranslationUnitDecl();

    EXPECT_NE(ImportedDef, ImportedProto);
    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 2u);
    auto *ToProto = FirstDeclMatcher<DeclTy>().match(ToTU, getPattern());
    auto *ToDef = LastDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_TRUE(ImportedDef == ToDef);
    EXPECT_TRUE(ImportedProto == ToProto);
    EXPECT_TRUE(ToDef->isThisDeclarationADefinition());
    EXPECT_FALSE(ToProto->isThisDeclarationADefinition());

    CheckPreviousDecl(ToProto, ToDef);
  }

  void TypedTest_WholeRedeclChainIsImportedAtOnce() {
    Decl *FromTU = getTuDecl(getPrototype() + getDefinition(), Lang_CXX);
    auto *FromD = // Definition
        LastDeclMatcher<DeclTy>().match(FromTU, getPattern());
    ASSERT_TRUE(FromD->isThisDeclarationADefinition());

    Decl *ImportedD = Import(FromD, Lang_CXX);
    Decl *ToTU = ImportedD->getTranslationUnitDecl();

    // The whole redecl chain is imported at once.
    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 2u);
    EXPECT_TRUE(cast<DeclTy>(ImportedD)->isThisDeclarationADefinition());
  }

  void TypedTest_ImportPrototypeThenProtoAndDefinition() {
    {
      Decl *FromTU = getTuDecl(getPrototype(), Lang_CXX, "input0.cc");
      auto *FromD = FirstDeclMatcher<DeclTy>().match(FromTU, getPattern());
      Import(FromD, Lang_CXX);
    }
    {
      Decl *FromTU =
          getTuDecl(getPrototype() + getDefinition(), Lang_CXX, "input1.cc");
      auto *FromD = FirstDeclMatcher<DeclTy>().match(FromTU, getPattern());
      Import(FromD, Lang_CXX);
    }

    Decl *ToTU = ToAST->getASTContext().getTranslationUnitDecl();

    ASSERT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 3u);
    DeclTy *ProtoD = FirstDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_FALSE(ProtoD->isThisDeclarationADefinition());

    DeclTy *DefinitionD = LastDeclMatcher<DeclTy>().match(ToTU, getPattern());
    EXPECT_TRUE(DefinitionD->isThisDeclarationADefinition());

    EXPECT_TRUE(DefinitionD->getPreviousDecl());
    EXPECT_FALSE(
        DefinitionD->getPreviousDecl()->isThisDeclarationADefinition());

    CheckPreviousDecl(ProtoD, DefinitionD->getPreviousDecl());
  }

  // =============================
  // Tests for ODR conflict cases.
  // =============================

  template <std::string (*ToTUContent)(), std::string (*FromTUContent)(),
            void (*ResultChecker)(llvm::Expected<Decl *> &, Decl *, Decl *)>
  void TypedTest_ImportAfter() {
    Decl *ToTU = getToTuDecl(ToTUContent(), Lang_CXX);
    auto *ToD = FirstDeclMatcher<DeclTy>().match(ToTU, getPattern());

    Decl *FromTU = getTuDecl(FromTUContent(), Lang_CXX);
    auto *FromD = FirstDeclMatcher<DeclTy>().match(FromTU, getPattern());

    auto Result = importOrError(FromD, Lang_CXX);

    ResultChecker(Result, ToTU, ToD);
  }

  template <std::string (*FromTU1Content)(), std::string (*FromTU2Content)(),
            void (*ResultChecker)(llvm::Expected<Decl *> &, Decl *, Decl *)>
  void TypedTest_ImportAfterImported() {
    Decl *FromTU1 = getTuDecl(FromTU1Content(), Lang_CXX, "input1.cc");
    auto *FromD1 = FirstDeclMatcher<DeclTy>().match(FromTU1, getPattern());
    auto Result1 = importOrError(FromD1, Lang_CXX);

    ASSERT_TRUE(isSuccess(Result1));
    Decl *ImportedD1 = *Result1;

    Decl *FromTU2 = getTuDecl(FromTU2Content(), Lang_CXX, "input2.cc");
    auto *FromD2 = FirstDeclMatcher<DeclTy>().match(FromTU2, getPattern());
    auto Result2 = importOrError(FromD2, Lang_CXX);

    ResultChecker(Result2, ImportedD1->getTranslationUnitDecl(), ImportedD1);
  }

  static void CheckImportedAsNew(llvm::Expected<Decl *> &Result, Decl *ToTU,
                                 Decl *ToD) {
    ASSERT_TRUE(isSuccess(Result));
    Decl *ImportedD = *Result;
    ASSERT_TRUE(ImportedD);
    EXPECT_NE(ImportedD, ToD);
    EXPECT_FALSE(ImportedD->getPreviousDecl());
    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 2u);
  }

  static void CheckImportNameConflict(llvm::Expected<Decl *> &Result,
                                      Decl *ToTU, Decl *ToD) {
    EXPECT_TRUE(isImportError(Result, ImportError::NameConflict));
    EXPECT_EQ(DeclCounter<DeclTy>().match(ToTU, getPattern()), 1u);
  }

  void TypedTest_ImportConflictingDefAfterDef() {
    TypedTest_ImportAfter<getDefinition, getConflictingDefinition,
                          CheckImportedAsNew>();
  }
  void TypedTest_ImportConflictingProtoAfterProto() {
    TypedTest_ImportAfter<getPrototype, getConflictingPrototype,
                          CheckImportedAsNew>();
  }
  void TypedTest_ImportConflictingProtoAfterDef() {
    TypedTest_ImportAfter<getDefinition, getConflictingPrototype,
                          CheckImportedAsNew>();
  }
  void TypedTest_ImportConflictingDefAfterProto() {
    TypedTest_ImportAfter<getConflictingPrototype, getDefinition,
                          CheckImportedAsNew>();
  }
  void TypedTest_ImportConflictingProtoDefAfterProto() {
    TypedTest_ImportAfter<getPrototype, getConflictingProtoDef,
                          CheckImportedAsNew>();
  }
  void TypedTest_ImportConflictingProtoAfterProtoDef() {
    TypedTest_ImportAfter<getConflictingProtoDef, getPrototype,
                          CheckImportedAsNew>();
  }
  void TypedTest_ImportConflictingProtoDefAfterDef() {
    TypedTest_ImportAfter<getDefinition, getConflictingProtoDef,
                          CheckImportedAsNew>();
  }
  void TypedTest_ImportConflictingDefAfterProtoDef() {
    TypedTest_ImportAfter<getConflictingProtoDef, getDefinition,
                          CheckImportedAsNew>();
  }

  void TypedTest_DontImportConflictingProtoAfterProto() {
    TypedTest_ImportAfter<getPrototype, getConflictingPrototype,
                          CheckImportNameConflict>();
  }
  void TypedTest_DontImportConflictingDefAfterDef() {
    TypedTest_ImportAfter<getDefinition, getConflictingDefinition,
                          CheckImportNameConflict>();
  }
  void TypedTest_DontImportConflictingProtoAfterDef() {
    TypedTest_ImportAfter<getDefinition, getConflictingPrototype,
                          CheckImportNameConflict>();
  }
  void TypedTest_DontImportConflictingDefAfterProto() {
    TypedTest_ImportAfter<getConflictingPrototype, getDefinition,
                          CheckImportNameConflict>();
  }
  void TypedTest_DontImportConflictingProtoDefAfterProto() {
    TypedTest_ImportAfter<getPrototype, getConflictingProtoDef,
                          CheckImportNameConflict>();
  }
  void TypedTest_DontImportConflictingProtoAfterProtoDef() {
    TypedTest_ImportAfter<getConflictingProtoDef, getPrototype,
                          CheckImportNameConflict>();
  }
  void TypedTest_DontImportConflictingProtoDefAfterDef() {
    TypedTest_ImportAfter<getDefinition, getConflictingProtoDef,
                          CheckImportNameConflict>();
  }
  void TypedTest_DontImportConflictingDefAfterProtoDef() {
    TypedTest_ImportAfter<getConflictingProtoDef, getDefinition,
                          CheckImportNameConflict>();
  }
};

// ==============================
// Define the parametrized tests.
// ==============================

#define ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(BaseTemplate, TypeParam,       \
                                                NamePrefix, TestCase)          \
  using BaseTemplate##TypeParam##Conservative =                                \
      BaseTemplate<TypeParam, ASTImporter::ODRHandlingType::Conservative>;     \
  TEST_P(BaseTemplate##TypeParam##Conservative, NamePrefix##TestCase) {        \
    TypedTest_##TestCase();                                                    \
  }

#define ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(                           \
    BaseTemplate, TypeParam, ODRHandlingParam, NamePrefix, TestCase)           \
  using BaseTemplate##TypeParam##ODRHandlingParam =                            \
      BaseTemplate<TypeParam, ASTImporter::ODRHandlingType::ODRHandlingParam>; \
  TEST_P(BaseTemplate##TypeParam##ODRHandlingParam, NamePrefix##TestCase) {    \
    TypedTest_##TestCase();                                                    \
  }

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Function, ,
    PrototypeShouldBeImportedAsAPrototypeWhenThereIsNoDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Class, ,
    PrototypeShouldBeImportedAsAPrototypeWhenThereIsNoDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Variable, ,
    PrototypeShouldBeImportedAsAPrototypeWhenThereIsNoDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, FunctionTemplate, ,
    PrototypeShouldBeImportedAsAPrototypeWhenThereIsNoDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, ,
    PrototypeShouldBeImportedAsAPrototypeWhenThereIsNoDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, ,
    PrototypeShouldBeImportedAsAPrototypeWhenThereIsNoDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, FunctionTemplateSpec, ,
    PrototypeShouldBeImportedAsAPrototypeWhenThereIsNoDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplateSpec, ,
    PrototypeShouldBeImportedAsAPrototypeWhenThereIsNoDefinition)

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Function, ,
                                        DefinitionShouldBeImportedAsADefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Class, ,
                                        DefinitionShouldBeImportedAsADefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Variable, ,
                                        DefinitionShouldBeImportedAsADefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplate, ,
                                        DefinitionShouldBeImportedAsADefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplate, ,
                                        DefinitionShouldBeImportedAsADefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, VariableTemplate, ,
                                        DefinitionShouldBeImportedAsADefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplateSpec, ,
                                        DefinitionShouldBeImportedAsADefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplateSpec, ,
                                        DefinitionShouldBeImportedAsADefinition)

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Function, ,
                                        ImportPrototypeAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Class, ,
                                        ImportPrototypeAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Variable, ,
                                        ImportPrototypeAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplate, ,
                                        ImportPrototypeAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplate, ,
                                        ImportPrototypeAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, VariableTemplate, ,
                                        ImportPrototypeAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplateSpec, ,
                                        ImportPrototypeAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplateSpec, ,
                                        ImportPrototypeAfterImportedPrototype)

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Function, ,
                                        ImportDefinitionAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Class, ,
                                        ImportDefinitionAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Variable, ,
                                        ImportDefinitionAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplate, ,
                                        ImportDefinitionAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplate, ,
                                        ImportDefinitionAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, VariableTemplate, ,
                                        ImportDefinitionAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplateSpec, ,
                                        ImportDefinitionAfterImportedPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplateSpec, ,
                                        ImportDefinitionAfterImportedPrototype)

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Function, ,
                                        ImportPrototypeAfterImportedDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Class, ,
                                        ImportPrototypeAfterImportedDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Variable, ,
                                        ImportPrototypeAfterImportedDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplate, ,
                                        ImportPrototypeAfterImportedDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplate, ,
                                        ImportPrototypeAfterImportedDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, VariableTemplate, ,
                                        ImportPrototypeAfterImportedDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplateSpec, ,
                                        ImportPrototypeAfterImportedDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplateSpec, ,
                                        ImportPrototypeAfterImportedDefinition)

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Function, ,
                                        ImportPrototypes)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Class, , ImportPrototypes)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Variable, ,
                                        ImportPrototypes)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplate, ,
                                        ImportPrototypes)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplate, ,
                                        ImportPrototypes)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, VariableTemplate, ,
                                        ImportPrototypes)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplateSpec, ,
                                        ImportPrototypes)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplateSpec, ,
                                        ImportPrototypes)

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Function, ,
                                        ImportDefinitions)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Class, , ImportDefinitions)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Variable, ,
                                        ImportDefinitions)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplate, ,
                                        ImportDefinitions)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplate, ,
                                        ImportDefinitions)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, VariableTemplate, ,
                                        ImportDefinitions)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplateSpec, ,
                                        ImportDefinitions)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplateSpec, ,
                                        ImportDefinitions)

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Function, ,
                                        ImportDefinitionThenPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Class, ,
                                        ImportDefinitionThenPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Variable, ,
                                        ImportDefinitionThenPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplate, ,
                                        ImportDefinitionThenPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplate, ,
                                        ImportDefinitionThenPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, VariableTemplate, ,
                                        ImportDefinitionThenPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplateSpec, ,
                                        ImportDefinitionThenPrototype)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplateSpec, ,
                                        ImportDefinitionThenPrototype)

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Function, ,
                                        ImportPrototypeThenDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Class, ,
                                        ImportPrototypeThenDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Variable, ,
                                        ImportPrototypeThenDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplate, ,
                                        ImportPrototypeThenDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplate, ,
                                        ImportPrototypeThenDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, VariableTemplate, ,
                                        ImportPrototypeThenDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplateSpec, ,
                                        ImportPrototypeThenDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, ClassTemplateSpec, ,
                                        ImportPrototypeThenDefinition)

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Function, ,
                                        WholeRedeclChainIsImportedAtOnce)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Variable, ,
                                        WholeRedeclChainIsImportedAtOnce)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplate, ,
                                        WholeRedeclChainIsImportedAtOnce)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, VariableTemplate, ,
                                        WholeRedeclChainIsImportedAtOnce)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplateSpec, ,
                                        WholeRedeclChainIsImportedAtOnce)

ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Function, ,
                                        ImportPrototypeThenProtoAndDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, Variable, ,
                                        ImportPrototypeThenProtoAndDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplate, ,
                                        ImportPrototypeThenProtoAndDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, VariableTemplate, ,
                                        ImportPrototypeThenProtoAndDefinition)
ASTIMPORTER_INSTANTIATE_TYPED_TEST_CASE(RedeclChain, FunctionTemplateSpec, ,
                                        ImportPrototypeThenProtoAndDefinition)

// clang-format off

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Class, Liberal, ,
    ImportConflictingDefAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Variable, Liberal, ,
    ImportConflictingDefAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Liberal, ,
    ImportConflictingDefAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Liberal, ,
    ImportConflictingDefAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplateSpec, Liberal, DISABLED_,
    ImportConflictingDefAfterDef)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Class, Conservative, ,
    DontImportConflictingDefAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Variable, Conservative, ,
    DontImportConflictingDefAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Conservative, ,
    DontImportConflictingDefAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Conservative, DISABLED_,
    DontImportConflictingDefAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplateSpec, Conservative, ,
    DontImportConflictingDefAfterDef)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Variable, Liberal, ,
    ImportConflictingProtoAfterProto)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Liberal, ,
    ImportConflictingProtoAfterProto)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Liberal, ,
    ImportConflictingProtoAfterProto)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Variable, Conservative, ,
    DontImportConflictingProtoAfterProto)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Conservative, ,
    DontImportConflictingProtoAfterProto)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Conservative, ,
    DontImportConflictingProtoAfterProto)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Variable, Liberal, ,
    ImportConflictingProtoAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Liberal, ,
    ImportConflictingProtoAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Liberal, ,
    ImportConflictingProtoAfterDef)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Variable, Conservative, ,
    DontImportConflictingProtoAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Conservative, ,
    DontImportConflictingProtoAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Conservative, ,
    DontImportConflictingProtoAfterDef)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Variable, Liberal, ,
    ImportConflictingDefAfterProto)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Liberal, ,
    ImportConflictingDefAfterProto)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Liberal, ,
    ImportConflictingDefAfterProto)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, Variable, Conservative, ,
    DontImportConflictingDefAfterProto)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Conservative, ,
    DontImportConflictingDefAfterProto)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Conservative, DISABLED_,
    DontImportConflictingDefAfterProto)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Liberal, ,
    ImportConflictingProtoDefAfterProto)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Liberal, ,
    ImportConflictingProtoDefAfterProto)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Conservative, ,
    DontImportConflictingProtoDefAfterProto)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Conservative, ,
    DontImportConflictingProtoDefAfterProto)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Liberal, ,
    ImportConflictingProtoAfterProtoDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Liberal, ,
    ImportConflictingProtoAfterProtoDef)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Conservative, ,
    DontImportConflictingProtoAfterProtoDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Conservative, ,
    DontImportConflictingProtoAfterProtoDef)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Liberal, ,
    ImportConflictingProtoDefAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Liberal, ,
    ImportConflictingProtoDefAfterDef)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Conservative, ,
    DontImportConflictingProtoDefAfterDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Conservative, DISABLED_,
    DontImportConflictingProtoDefAfterDef)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Liberal, ,
    ImportConflictingDefAfterProtoDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Liberal, ,
    ImportConflictingDefAfterProtoDef)

ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, ClassTemplate, Conservative, ,
    DontImportConflictingDefAfterProtoDef)
ASTIMPORTER_ODR_INSTANTIATE_TYPED_TEST_CASE(
    RedeclChain, VariableTemplate, Conservative, DISABLED_,
    DontImportConflictingDefAfterProtoDef)

// ======================
// Instantiate the tests.
// ======================

INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainFunctionConservative,
    DefaultTestValuesForRunOptions, );
INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainClassConservative,
    DefaultTestValuesForRunOptions, );
INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainVariableConservative,
    DefaultTestValuesForRunOptions, );
INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainFunctionTemplateConservative,
    DefaultTestValuesForRunOptions, );
INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainClassTemplateConservative,
    DefaultTestValuesForRunOptions, );
INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainVariableTemplateConservative,
    DefaultTestValuesForRunOptions, );
INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainFunctionTemplateSpecConservative,
    DefaultTestValuesForRunOptions, );
INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainClassTemplateSpecConservative,
    DefaultTestValuesForRunOptions, );

INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainClassLiberal,
    DefaultTestValuesForRunOptions, );
INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainVariableLiberal,
    DefaultTestValuesForRunOptions, );
INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainClassTemplateLiberal,
    DefaultTestValuesForRunOptions, );
// FIXME: Make these tests all work.
// INSTANTIATE_TEST_CASE_P(
//     ParameterizedTests, RedeclChainVariableTemplateLiberal,
//     DefaultTestValuesForRunOptions, );
INSTANTIATE_TEST_CASE_P(
    ParameterizedTests, RedeclChainClassTemplateSpecLiberal,
    DefaultTestValuesForRunOptions, );

// clang-format on

} // end namespace ast_matchers
} // end namespace clang
