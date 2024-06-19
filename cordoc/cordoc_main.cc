// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
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
      std::cout << FS->getASTContext()
                       .getTypeDeclType(const_cast<CXXRecordDecl *>(FS))
                       .getAsString()
                << "\n";
      std::cout << FS->getNameAsString() << "\n";
      for (const auto &f : FS->fields()) {
        std::cout << f->getNameAsString() << "\n";
      }
      // FS->dump();
    }
  }
};

int main(int argc, const char **argv) {
  const std::vector<std::string> args{argv, argv + argc};
  auto fs = llvm::vfs::getRealFileSystem();
  auto files = new FileManager(FileSystemOptions(), fs);

  Cb Printer;
  MatchFinder Finder;
  Finder.addMatcher(StructMatcher, &Printer);
  auto action = newFrontendActionFactory(&Finder);

  ToolInvocation Tool(args, action->create(), files);
  return Tool.run();
}