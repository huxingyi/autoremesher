#include <FPG/io.h>
#include <FPG/Symbol.h> // for type_sign_function
#include <FPG/SymbolEnvironment.h>
#include <FPG/AST.h>
#include <stdio.h>

static bool have_input = false;

namespace Flags {
    bool prettyprint = false; // pretty print the (desugared) program
    bool dumpAST = false;     // print the AST
    bool xstdin = false;       // read from stdin
}


inline
void setFlag( char current_flag ) {
    switch( current_flag ) {
    case 'p': Flags::prettyprint = true; break;
    case 'd': Flags::dumpAST = true; break;
    case 's': Flags::xstdin = true; break;
    default : std::cerr << "unknown parameter " << current_flag << std::endl; break;
    }
}

/* parsing stuff */
int lineno;
AST::TranslationUnit *translation_unit;

// provided by yacc
extern void yyparse();

// - overwrites current pointer to translation_unit
// - ...
void process_cmdline_args( int argc, char **argv ) {
    for (int i=1; i<argc; ++i) {
        if( argv[i][0] == '-' ) {
            for(int j=1; argv[i][j]!='\0';++j)
                setFlag( argv[i][j] );
            continue;
        }
        if( Flags::xstdin )
            continue;

        if ( have_input)
            std::cerr << "only one file per run" << std::endl;
        else if( open_and_parse( argv[i] ) )
            have_input = true;
        else {
            std::cerr<<"file \""  << argv[i]
            <<"\" not found."<<std::endl;
        }
    }

    if(Flags::xstdin) {
        reset_parser();
        yyparse();
        symbol_env.resolve_function_calls( translation_unit );
        translation_unit->computeType();
        have_input = true;
    }

    if(!have_input)
        throw Error("no input files");

}

bool
open_and_parse( std::string filename ) {
    AST::current_location.filename = filename;
    if( freopen(filename.c_str(), "r", stdin) ) {
        reset_parser();
        yyparse();
        symbol_env.resolve_function_calls( translation_unit );
        translation_unit->computeType();
        return true;
    } 
    return false;
}

void
reset_parser() {
    AST::current_location.reset();
    // TODO: big memory leak. might lead to problems in the future.
    translation_unit = new AST::TranslationUnit();
    symbol_env.clear();
}
