#include <iostream>
#include <string>
#include <cassert>
#include <cstdlib>

#include <FPG/io.h>
#include <FPG/Symbol.h>
#include <FPG/Simple_semistatic_filter.h>
#include <FPG/Static_filter_with_scaling.h>
#include <FPG/Prettyprint_visitor.h>
//#include <FPG/Static_filter_error.h>
#include <iomanip>

int main(int argc, char**argv) {
    // some small self-tests
    std::string filename;
    bool semistatic = false;
    for( int i = 1; i < argc; ++i ) {
        std::string arg = argv[i];
        if( arg == "-semistatic" )
            semistatic = true;
        else if( arg == "-return_type" ) {
            assert( i + 1 < argc );
            ++i;
            filtered_predicate_return_type->id = argv[i];
        } else if( arg == "-uncertain_value" ) {
            assert( i + 1 < argc );
            ++i;
            AST::uncertain_return_value = new AST::PlainTextExpression( argv[i], type_int );
        } else
            filename = arg;
    }
    if( filename != std::string() ) {
        try {
            if( !open_and_parse( filename ) ) {
                std::cerr << "! could not open file " << filename << std::endl;
	       ::exit(1);
            }
            Prettyprint_visitor pretty( std::cout );
            if( semistatic ) {
                //translation_unit->dump();
                using namespace Simple_semistatic_filter;
                Add_bound_computation add_bound_computation;
                translation_unit->accept( &add_bound_computation );
            } else {
                using namespace Static_filter_with_scaling;
                Add_bound_variables add_var(false);
                add_var.visit( translation_unit );
            }
            Beautify_visitor beautify;
            translation_unit->accept( &beautify );
            translation_unit->accept( &pretty );
        } catch( Error& e ) {
            std::cout << e << std::endl;
        }

    } else {
        std::cout << "you need to give exactly one filename" << std::endl;
    }
    return 0;
}
