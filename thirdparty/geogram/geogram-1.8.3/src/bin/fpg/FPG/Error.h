#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <iostream>

#include <FPG/Location.h>

/**
 * \brief Suppresses compiler warnings about unused parameters.
 */
template <class T> inline void argused(const T&) {
}

class Error {
 public:
  Location location;
  const std::string msg;


    Error()  {}
    Error( const std::string& s) : msg( s ) {}
    Error( const std::string& s, const Location& location )
      : location(location), msg( s ) { }
};

inline std::ostream&
operator<<(std::ostream &o, Error& e) {
  o << e.location << std::endl;
  o << e.msg << std::endl;
  return o;
}

struct ParseError : public Error {
  ParseError( const std::string& s, const Location& location ) : Error( "parse error: " + s, location) {}
    ParseError( const std::string& s ) : Error( "parse error: " + s) {}
};

struct InternalError: public Error {
  InternalError( const std::string& s, const Location& location ) : Error( "internal error: " + s, location ) {}
    InternalError( const std::string& s ) : Error( "internal error: " + s ) {}
};

struct RuntimeError: public Error {
   RuntimeError( const std::string& s , const Location& location ) : Error(s, location) {}
    RuntimeError( const std::string& s ) : Error(s) {}
    RuntimeError() : Error( "runtime error: ") {}
};

struct TypeError : public Error {
  TypeError( const std::string& s, const Location& location) : Error(s, location) {}
    TypeError( const std::string& s ) : Error( s ) {}
    TypeError() : Error("type error: ") {}
};


struct InvalidArgumentError : public RuntimeError {
  InvalidArgumentError( const std::string& s , const Location& location ) : RuntimeError( s , location ) {}
    InvalidArgumentError( const std::string& s ) : RuntimeError( s ) {}
    InvalidArgumentError() : RuntimeError("invalid argument error") {}
};

// thrown, if a non-constant expression is statically evaluated
struct StaticEvaluationError : public RuntimeError {};

struct Warning {
    const std::string msg;
    Warning( const std::string &msg ) : msg(msg) {}
};


#endif
