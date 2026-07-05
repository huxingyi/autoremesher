#include <FPG/Location.h>

std::string Location::toString() const
{
  std::stringstream loc;
  if(line)
    if(filename!="")
      loc<<filename<<":"<<line<<":"<<column<<":";
    else
      return "";
  else
    loc<<filename<<":";
  return loc.str();
}
