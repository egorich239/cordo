#include <iostream>
#include <unordered_map>

// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

static cl::OptionCategory CordocCategory("cordoc options");
// static cl::opt<std::string> InputFilename(cl::Positional, cl::Required,
//                                           cl::desc("<input file>"));
// static cl::opt<std::string> OutputFilename("o", cl::Required,
//                                            cl::desc("Output filename"),
//                                            cl::value_desc("filename"));

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

struct StructTraits final {
  std::string fullName;
  std::vector<std::string> fields;
};

struct CordocState final {
  std::vector<StructTraits> structs;
};

class CordocVisitor : public RecursiveASTVisitor<CordocVisitor> {
 public:
  explicit CordocVisitor(ASTContext *context, CordocState *state)
      : context_(context), state_(state) {}

  bool VisitCXXRecordDecl(CXXRecordDecl *decl) {
    // if (context_->getFullLoc(decl->getBeginLoc()).getFileID() !=
    //     context_->getFullLoc(context_->getTranslationUnitDecl()->getBeginLoc())
    //         .getFileID()) {
    //   return true;
    // }
    bool attrFound = false;
    for (auto &&attr : decl->attrs()) {
      attrFound |= (attr->getKind() == clang::attr::Annotate);
      // TODO: check the syntax of the annotation.
    }
    if (!attrFound) {
      return true;
    }
    if (!decl->isStruct()) {
      // llvm::errs() << "only structs are currently supported";
      return true;
    }
    if (!decl->isAggregate() || decl->getNumBases() ||
        !decl->hasAttr<FinalAttr>()) {
      // llvm::errs()
      //     << "struct must be aggregate, without base classes, and final";
      return true;
    }

    FullSourceLoc fullLocation = context_->getFullLoc(decl->getBeginLoc());
    if (fullLocation.isValid()) {
      // llvm::outs() << "Found declaration " <<
      // decl->getQualifiedNameAsString()
      //              << " at " << fullLocation.getSpellingLineNumber() << ":"
      //              << fullLocation.getSpellingColumnNumber() << "\n";
    }

    state_->structs.push_back(StructTraits{
        .fullName = decl->getQualifiedNameAsString(),
        .fields = {},
    });
    for (auto &&fld : decl->fields()) {
      state_->structs.back().fields.push_back(fld->getNameAsString());
    }
    return true;
  }

 private:
  ASTContext *context_;
  CordocState *state_;
};

class CordocAstConsumer : public clang::ASTConsumer {
 public:
  explicit CordocAstConsumer(ASTContext *context, CordocState *state)
      : visitor_(context, state) {}

  virtual void HandleTranslationUnit(clang::ASTContext &context) {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
  }

 private:
  CordocVisitor visitor_;
};

class CordocFrontendAction : public clang::ASTFrontendAction {
 public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance &compiler, llvm::StringRef InFile) override {
    auto &sourceMgr = compiler.getSourceManager();
    // TODO: proper filename
    outFile_ =
        std::string(
            sourceMgr.getFileEntryForID(sourceMgr.getMainFileID())->getName()) +
        ".cordo";

    return std::make_unique<CordocAstConsumer>(&compiler.getASTContext(),
                                               &state_);
  }

  void EndSourceFileAction() override {
    std::error_code error_code;
    llvm::raw_fd_ostream outFile(outFile_, error_code, llvm::sys::fs::OF_None);
    for (auto &s : state_.structs) {
      outFile << "constexpr auto cordo_cpo(\n"
              << "    ::cordo::mirror_cpo,\n"
              << "    ::cordo::tag_t<::" << s.fullName << ">) noexcept {\n";
      outFile << "  constexpr struct {\n"
              << "    using tuple_t = " << "::" << s.fullName << ";\n";
      outFile << "    using fields_t = ::cordo::values_t<\n";
      bool firstField = true;
      for (const auto &f : s.fields) {
        if (!firstField) outFile << ",\n";
        firstField = false;
        outFile << "      (\"" << f << "\"_key <= &::" << s.fullName
                << "::" << f << ")";
      }
      outFile << ">;\n";
      outFile << "    constexpr auto name() const noexcept { return \""
              << s.fullName << "\"_key; }\n";
      outFile << "  } result{};\n"
              << "  return result;\n"
              << "}\n";
    }
    outFile.close();
  }

 private:
  CordocState state_;
  std::string outFile_;
};

#if 0
    const auto fqname =decl->getQualifiedNameAsString();
    std::cout << "constexpr auto cordo_cpo(\n"
              << "    ::cordo::mirror_cpo,\n"
              << "    ::cordo::tag_t<::" << fqname << ">) noexcept {\n";
    std::cout << "  constexpr struct {\n"
              << "    using tuple_t = " << "::" << fqname << ";\n";
    std::cout << "    using fields_t = ::cordo::values_t<\n";
    bool firstField = true;
    for (const auto &f : decl->fields()) {
      const auto &name = f->getNameAsString();
      if (!firstField) std::cout << ",\n";
      firstField = false;
      std::cout << "      (\"" << name << "\"_key = &::" << fqname
                << "::" << name << ")";
    }
    std::cout << ">;\n";
    std::cout << "    constexpr auto name() const noexcept { return \""
              << fqname << "\"; }\n";
    std::cout << "  } result{};\n"
              << "  return result;\n"
              << "}\n";

#endif

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, CordocCategory);
  if (!ExpectedParser) {
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }

  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<CordocFrontendAction>().get());
}