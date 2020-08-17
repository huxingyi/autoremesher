#ifndef FPG_IO_H
#define FPG_IO_H

#include <string>

namespace AST {
    struct TranslationUnit;
}


namespace Flags {
    extern bool prettyprint; // pretty print the (desugared) program
    extern bool dumpAST;
    extern bool xstdin;
}

/* parsing stuff */
extern int lineno;
extern AST::TranslationUnit *translation_unit;

void process_cmdline_args( int argc, char **argv );
bool open_and_parse( std::string filename );
void reset_parser();

#endif
