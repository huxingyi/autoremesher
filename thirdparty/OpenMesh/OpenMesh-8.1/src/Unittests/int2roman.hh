#ifndef INT2ROMAN_HH
#define INT2ROMAN_HH

#include <string>

std::string int2roman( size_t decimal, size_t length = 30 )
{
  assert( decimal > 0 && decimal < 1000 );

  const size_t nrows = 4;
  const size_t ncols = 4;

  static size_t table_arabs[ nrows ][ ncols ] = { { 1000, 1000, 1000, 1000 },
                                                  {  900,  500,  400,  100 },
                                                  {   90,   50,   40,   10 },
                                                  {    9,    5,    4,    1 } };

  static const char *table_romans[ nrows ][ ncols ] = { {  "M",  "M",  "M", "M" },
                                                  { "CM",  "D", "CD", "C" },
                                                  { "XC",  "L", "XL", "X" },
                                                  { "IX",  "V", "IV", "I" } };

  size_t power;	// power of ten
  size_t index;  // Indexes thru values to subtract

  std::string roman = "";
  roman.reserve(length);

  for ( power = 0; power < nrows; power++ )
    for ( index = 0; index < ncols; index++ )
	while ( decimal >= table_arabs[ power ][ index ] )
        {
	   roman   += table_romans[ power ][ index ];
	   decimal -= table_arabs[ power ][ index ];
	}

  return roman;
}

#endif
