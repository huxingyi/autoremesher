///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2012, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <ImfXdr.h>
#include <ImfIO.h>
#include <typeinfo>
#include <string.h>
#include <assert.h>

#if defined (__GNUC__) && __GNUC__ == 2
#include <strstream.h>
#else
#include <sstream>
#endif


using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;


namespace {


struct CharIO
{
    static void
    writeChars (ostream &os, const char c[], int n)
    {
	os.write (c, n);
    }

    static bool
    readChars (istream &is, char c[], int n)
    {
	is.read (c, n);
	return true;
    }
};


void
writeData (ostream &os)
{
    Xdr::write<CharIO>    (os, (bool) true);
    Xdr::write<CharIO>    (os, (bool) false);
    Xdr::write<CharIO>    (os, (char) 'r');
    Xdr::write<CharIO>    (os, (char) 'e');
    Xdr::write<CharIO>    (os, (signed char) 'u');
    Xdr::write<CharIO>    (os, (signed char) 'c');
    Xdr::write<CharIO>    (os, (unsigned char) 'k');
    Xdr::write<CharIO>    (os, (unsigned char) 'y');
    Xdr::write<CharIO>    (os, (signed short)  8765);
    Xdr::write<CharIO>    (os, (signed short) -9876);
    Xdr::write<CharIO>    (os, (unsigned short)  6543);
    Xdr::write<CharIO>    (os, (unsigned short) 17432);
    Xdr::write<CharIO>    (os, (signed int)  2023456789);
    Xdr::write<CharIO>    (os, (signed int) -2012345678);
    Xdr::write<CharIO>    (os, (unsigned int) 1234567890u);
    Xdr::write<CharIO>    (os, (unsigned int) 2345678901u);
    Xdr::write<CharIO>    (os, (signed long)  2034567890);
    Xdr::write<CharIO>    (os, (signed long) -2045678901);
    Xdr::write<CharIO>    (os, (unsigned long) 1345678901u);
    Xdr::write<CharIO>    (os, (unsigned long) 2456789012u);
    Xdr::write<CharIO>    (os, (Int64) 0x1122334455667788ll);
    Xdr::write<CharIO>    (os, (Int64) 0xf1f2f3f4f5f6f7f8ll);
    Xdr::write<CharIO>    (os, (float) 0.0f);
    Xdr::write<CharIO>    (os, (float) 3.141592653589793f);
    Xdr::write<CharIO>    (os, (float) 6.141592653589793f);
    Xdr::write<CharIO>    (os, (double) 0);
    Xdr::write<CharIO>    (os, (double) 1.41421356237309504880);
    Xdr::write<CharIO>    (os, (double) 2.41421356237309504880);
    Xdr::write<CharIO>    (os, (half) 0);
    Xdr::write<CharIO>    (os, (half) 3.4142);
    Xdr::write<CharIO>    (os, (half) 4.4142);
    Xdr::write<CharIO>    (os, "abcdefgh", 4);	// fixed-size char array
    Xdr::write<CharIO>    (os, "rstuvwxy", 5);
    Xdr::write<CharIO>    (os, "qwerty");	// zero-terminated string
    Xdr::write<CharIO>    (os, "asdfghjkl");
    Xdr::write<CharIO>    (os, "");	
    Xdr::pad<CharIO>      (os, 17);
    Xdr::write<CharIO>    (os, (int) 1);
    Xdr::pad<CharIO>      (os, 0);
    Xdr::write<CharIO>    (os, (int) 2);
    Xdr::pad<CharIO>      (os, 17);
    Xdr::write<CharIO>    (os, (int) 3);

    assert (!os == false);
}

template <class T>
void
check (istream &is, T c)
{
    T v;

    Xdr::read<CharIO> (is, v);
    cout << typeid(v).name() << ": expected " << c << ", got " << v << endl;
    assert (c == v);
}


template <>
void
check (istream &is, const char * c)
{
    char v[1024];

    Xdr::read<CharIO> (is, sizeof (v), v);

    cout << "zero-terminated string: "
	    "expected \"" << c << "\", "
	    "got \"" << v << "\"" << endl;

    assert (!strcmp (c, v));
}


void
check (istream &is, const char c[/*n*/], int n)
{
    char v[1024];

    assert (n <= (int) sizeof (v));

    Xdr::read<CharIO> (is, v, n);

    cout << "char[" << n << "]: expected \"";

    for (int i = 0; i < n; i++)
	cout << c[i];

    cout << "\", got \"";

    for (int i = 0; i < n; i++)
	cout << v[i];

    cout << "\"" << endl;

    for (int i = 0; i < n; i++)
	assert (c[i] == v[i]);
}


void
readData (istream &is)
{
    check (is, (bool) true);
    check (is, (bool) false);
    check (is, (char) 'r');
    check (is, (char) 'e');
    check (is, (signed char) 'u');
    check (is, (signed char) 'c');
    check (is, (unsigned char) 'k');
    check (is, (unsigned char) 'y');
    check (is, (signed short)  8765);
    check (is, (signed short) -9876);
    check (is, (unsigned short)  6543);
    check (is, (unsigned short) 17432);
    check (is, (signed int)  2023456789);
    check (is, (signed int) -2012345678);
    check (is, (unsigned int) 1234567890u);
    check (is, (unsigned int) 2345678901u);
    check (is, (signed long)  2034567890);
    check (is, (signed long) -2045678901);
    check (is, (unsigned long) 1345678901u);
    check (is, (unsigned long) 2456789012u);
    check (is, (Int64) 0x1122334455667788ll);
    check (is, (Int64) 0xf1f2f3f4f5f6f7f8ll);
    check (is, (float) 0.0f);
    check (is, (float) 3.141592653589793f);
    check (is, (float) 6.141592653589793f);
    check (is, (double) 0.0);
    check (is, (double) 1.41421356237309504880);
    check (is, (double) 2.41421356237309504880);
    check (is, (half) 0);
    check (is, (half) 3.4142);
    check (is, (half) 4.4142);
    check (is, (const char *) "abcdefgh", 4);
    check (is, (const char *) "rstuvwxy", 5);
    check (is, (const char *) "qwerty");
    check (is, (const char *) "asdfghjkl");
    check (is, (const char *) "");	
    Xdr::skip<CharIO>  (is, 17);
    check (is, (int) 1);
    Xdr::skip<CharIO>  (is, 0);
    check (is, (int) 2);
    Xdr::skip<CharIO>  (is, 17);
    check (is, (int) 3);
    assert (!is == false);
}


} // namespace


void
testXdr (const std::string&)
{
    cout << "Testing Xdr" << endl;

    try
    {
	assert (sizeof (Int64) == 8);

#if defined (__GNUC__) && __GNUC__ == 2
	strstream s;
#else
	stringstream s;
#endif

	writeData (s);
	s.seekg (0);
	readData (s);
    }
    catch (const std::exception &e)
    {
	cout << "unexpected exception: " << e.what() << endl;
	assert (false);
    }
    catch (...)
    {
	cout << "unexpected exception" << endl;
	assert (false);
    }

    cout << "ok\n" << endl;
}
