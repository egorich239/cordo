#include <iostream>

// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

DeclarationMatcher StructMatcher =
    cxxRecordDecl(hasDefinition(), hasAttr(clang::attr::Annotate))
        .bind("record");

class Cb final : public MatchFinder::MatchCallback {
 public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const CXXRecordDecl *FS =
            Result.Nodes.getNodeAs<clang::CXXRecordDecl>("record");
        FS && FS->isStruct() && FS->isAggregate() && !FS->getNumBases() &&
        FS->hasAttr<FinalAttr>()) {
      const auto fqname = FS->getQualifiedNameAsString();
      std::cout << "constexpr auto cordo_cpo(\n"
                << "    ::cordo::mirror_cpo,\n"
                << "    ::cordo::tag_t<::" << fqname << ">) noexcept {\n";
      std::cout << "  constexpr struct {\n"
                << "    using tuple_t = " << "::" << fqname << ";\n";
      std::cout << "    using fields_t = ::cordo::values_t<\n";
      bool firstField = true;
      for (const auto &f : FS->fields()) {
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
    }
  }
};

CommandLineArguments makeCommandLineArgs(int argc, const char **argv) {
  CommandLineArguments args{argv, argv + argc};
  args = getClangStripOutputAdjuster()(args, "");
  args = getClangStripDependencyFileAdjuster()(args, "");
  args = getClangSyntaxOnlyAdjuster()(args, "");
  return args;
}

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }

  Cb Printer;
  MatchFinder Finder;
  Finder.addMatcher(StructMatcher, &Printer);

  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory(&Finder).get());
}