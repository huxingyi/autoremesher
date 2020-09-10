///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
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

#include <testBaseExc.h>
#include <Iex.h>
#include <iostream>
#include <stdexcept>
#include <assert.h>

namespace {

void
throwArgExc ()
{
    throw IEX_INTERNAL_NAMESPACE::ArgExc ("ArgExc");
}

void
throwLogicError ()
{
    throw std::logic_error("logic_error");
}

void
throwInt ()
{
    throw 3;
}

void
throwNested()
{
    try
    {
	throwArgExc();
    }
    catch (const IEX_INTERNAL_NAMESPACE::ArgExc &)
    {
        bool caught = false;
	try
	{
	    throwInt();
	}
	catch (...)
	{
            caught = true;
	}

        assert (caught);

	throw;
    }
}

void
test1 ()
{
    std::cout << "1" << std::endl;

    try
    {
	throwArgExc();
    }
    catch (const IEX_INTERNAL_NAMESPACE::ArgExc &)
    {
	return;
    }
    catch (std::exception &)
    {
	assert (false);
    }
    catch (...)
    {
	assert (false);
    }

    assert (false);
}

void
test2 ()
{
    std::cout << "2" << std::endl;

    try
    {
	throwLogicError();
    }
    catch (const IEX_INTERNAL_NAMESPACE::ArgExc &)
    {
	assert (false);
    }
    catch (std::exception &)
    {
	return;
    }
    catch (...)
    {
	assert (false);
    }

    assert (false);
}

void
test3 ()
{
    std::cout << "3" << std::endl;

    try
    {
	throwArgExc();
    }
    catch (std::exception &)
    {
	return;
    }
    catch (...)
    {
	assert (false);
    }

    assert (false);
}

void
test4 ()
{
    std::cout << "4" << std::endl;

    try
    {
	throwInt();
    }
    catch (const IEX_INTERNAL_NAMESPACE::ArgExc &)
    {
	assert (false);
    }
    catch (std::exception &)
    {
	assert (false);
    }
    catch (...)
    {
	return;
    }

    assert (false);
}

void
test5()
{
    std::cout << "5" << std::endl;

    try
    {
	throwNested();
    }
    catch (const IEX_INTERNAL_NAMESPACE::ArgExc &e)
    {
	assert (std::string(e.what()) == "ArgExc");
    }
}

} // namespace


void
testBaseExc()
{
    std::cout << "See if throw and catch work:" << std::endl;

    test1();
    test2();
    test3();
    test4();
    test5();

    std::cout << "ok\n" << std::endl;
}
