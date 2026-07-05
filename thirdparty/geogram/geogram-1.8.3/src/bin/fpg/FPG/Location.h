#ifndef LOCATION_H
#define LOCATION_H

#include <string>
#include <sstream>
#include <iostream>

/* describes where "something " is, i.e.,
   in which file and at which point in a file */

struct Location  {
    Location() : filename("stdin")
    { reset(); }

    std::string toString() const;
    void reset() { line = 1; column = 1; }

    std::string filename;
    int line;
    int column;
};

inline std::ostream&
operator<<(std::ostream &o, Location& l ) {
    o << "in file " << l.filename << " at line " << l.line << ":" << std::endl;
    return o;
}

#endif
