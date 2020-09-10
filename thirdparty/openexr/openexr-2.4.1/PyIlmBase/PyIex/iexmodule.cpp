///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2011, Industrial Light & Magic, a division of Lucas
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

#include <Python.h>
#include <boost/python.hpp>
#include <boost/format.hpp>
#include <Iex.h>
#include <PyIex.h>
#include <PyIexExport.h>
#include <IexErrnoExc.h>
#include <iostream>

using namespace boost::python;
using namespace IEX_NAMESPACE;

namespace PyIex {

namespace {

static void translateBaseExc(const IEX_NAMESPACE::BaseExc &exc)
{
    PyErr_SetObject(baseExcTranslator().typeObject(&exc),ExcTranslator<IEX_NAMESPACE::BaseExc>::convert(exc));
}

void
registerBaseExc()
{
    using namespace boost::python;

    std::string name = "BaseExc";
    std::string module = "iex";
    std::string baseName = "RuntimeError";
#if PY_MAJOR_VERSION > 2
    std::string baseModule = "builtins";
#else
    std::string baseModule = "__builtin__";
#endif
    // if module != baseModule, the type object isn't used
    object exc_class = createExceptionProxy(name, module, baseName, baseModule, 0);
    scope().attr(name.c_str()) = exc_class;
    setBaseExcTranslator(new TypeTranslator<IEX_NAMESPACE::BaseExc>(name, module, exc_class.ptr()));

    // to python
    to_python_converter<IEX_NAMESPACE::BaseExc,ExcTranslator<IEX_NAMESPACE::BaseExc> >();

    // from python
    converter::registry::push_back(&ExcTranslator<IEX_NAMESPACE::BaseExc>::convertible,
                                   &ExcTranslator<IEX_NAMESPACE::BaseExc>::construct,type_id<IEX_NAMESPACE::BaseExc>());

    // exception translation for BaseExc and subtypes
    register_exception_translator<IEX_NAMESPACE::BaseExc>(&translateBaseExc);
}

void
testCxxExceptions (int i)
{
    //
    // This function is only for testing.
    // It exercises the PY_TRY / PY_CATCH macros
    // and the C++ to Python exception translation.
    //


    switch (i)
    {
      case 1:
	throw int (1);

      case 2:
	throw std::invalid_argument ("2");

      case 3:
	throw IEX_NAMESPACE::BaseExc ("3");

      case 4:
	throw IEX_NAMESPACE::ArgExc ("4");

      default:
	;
    }
}

std::string
testBaseExcString(const BaseExc &exc)
{
    return exc.what();
}

std::string
testArgExcString(const ArgExc &exc)
{
    return exc.what();
}

BaseExc
testMakeBaseExc(const std::string &s)
{
    return BaseExc(s);
}

ArgExc
testMakeArgExc(const std::string &s)
{
    return ArgExc(s);
}

} // namespace

} // namespace PyIex

using namespace PyIex;

BOOST_PYTHON_MODULE(iex)
{
    using namespace IEX_NAMESPACE;

    def("testCxxExceptions", &testCxxExceptions);
    def("testBaseExcString", &testBaseExcString);
    def("testArgExcString", &testArgExcString);
    def("testMakeBaseExc", &testMakeBaseExc);
    def("testMakeArgExc", &testMakeArgExc);

    registerBaseExc();
    registerExc<ArgExc,BaseExc>("ArgExc","iex");
    registerExc<LogicExc,BaseExc>("LogicExc","iex");
    registerExc<InputExc,BaseExc>("InputExc","iex");
    registerExc<IoExc,BaseExc>("IoExc","iex");
    registerExc<MathExc,BaseExc>("MathExc","iex");
    registerExc<NoImplExc,BaseExc>("NoImplExc","iex");
    registerExc<NullExc,BaseExc>("NullExc","iex");
    registerExc<TypeExc,BaseExc>("TypeExc","iex");
    registerExc<ErrnoExc,BaseExc>("ErrnoExc","iex");
    registerExc<EpermExc,ErrnoExc>("EpermExc","iex");
    registerExc<EnoentExc,ErrnoExc>("EnoentExc","iex");
    registerExc<EsrchExc,ErrnoExc>("EsrchExc","iex");
    registerExc<EintrExc,ErrnoExc>("EintrExc","iex");
    registerExc<EioExc,ErrnoExc>("EioExc","iex");
    registerExc<EnxioExc,ErrnoExc>("EnxioExc","iex");
    registerExc<E2bigExc,ErrnoExc>("E2bigExc","iex");
    registerExc<EnoexecExc,ErrnoExc>("EnoexecExc","iex");
    registerExc<EbadfExc,ErrnoExc>("EbadfExc","iex");
    registerExc<EchildExc,ErrnoExc>("EchildExc","iex");
    registerExc<EagainExc,ErrnoExc>("EagainExc","iex");
    registerExc<EnomemExc,ErrnoExc>("EnomemExc","iex");
    registerExc<EaccesExc,ErrnoExc>("EaccesExc","iex");
    registerExc<EfaultExc,ErrnoExc>("EfaultExc","iex");
    registerExc<EnotblkExc,ErrnoExc>("EnotblkExc","iex");
    registerExc<EbusyExc,ErrnoExc>("EbusyExc","iex");
    registerExc<EexistExc,ErrnoExc>("EexistExc","iex");
    registerExc<ExdevExc,ErrnoExc>("ExdevExc","iex");
    registerExc<EnodevExc,ErrnoExc>("EnodevExc","iex");
    registerExc<EnotdirExc,ErrnoExc>("EnotdirExc","iex");
    registerExc<EisdirExc,ErrnoExc>("EisdirExc","iex");
    registerExc<EinvalExc,ErrnoExc>("EinvalExc","iex");
    registerExc<EnfileExc,ErrnoExc>("EnfileExc","iex");
    registerExc<EmfileExc,ErrnoExc>("EmfileExc","iex");
    registerExc<EnottyExc,ErrnoExc>("EnottyExc","iex");
    registerExc<EtxtbsyExc,ErrnoExc>("EtxtbsyExc","iex");
    registerExc<EfbigExc,ErrnoExc>("EfbigExc","iex");
    registerExc<EnospcExc,ErrnoExc>("EnospcExc","iex");
    registerExc<EspipeExc,ErrnoExc>("EspipeExc","iex");
    registerExc<ErofsExc,ErrnoExc>("ErofsExc","iex");
    registerExc<EmlinkExc,ErrnoExc>("EmlinkExc","iex");
    registerExc<EpipeExc,ErrnoExc>("EpipeExc","iex");
    registerExc<EdomExc,ErrnoExc>("EdomExc","iex");
    registerExc<ErangeExc,ErrnoExc>("ErangeExc","iex");
    registerExc<EnomsgExc,ErrnoExc>("EnomsgExc","iex");
    registerExc<EidrmExc,ErrnoExc>("EidrmExc","iex");
    registerExc<EchrngExc,ErrnoExc>("EchrngExc","iex");
    registerExc<El2nsyncExc,ErrnoExc>("El2nsyncExc","iex");
    registerExc<El3hltExc,ErrnoExc>("El3hltExc","iex");
    registerExc<El3rstExc,ErrnoExc>("El3rstExc","iex");
    registerExc<ElnrngExc,ErrnoExc>("ElnrngExc","iex");
    registerExc<EunatchExc,ErrnoExc>("EunatchExc","iex");
    registerExc<EnocsiExc,ErrnoExc>("EnocsiExc","iex");
    registerExc<El2hltExc,ErrnoExc>("El2hltExc","iex");
    registerExc<EdeadlkExc,ErrnoExc>("EdeadlkExc","iex");
    registerExc<EnolckExc,ErrnoExc>("EnolckExc","iex");
    registerExc<EbadeExc,ErrnoExc>("EbadeExc","iex");
    registerExc<EbadrExc,ErrnoExc>("EbadrExc","iex");
    registerExc<ExfullExc,ErrnoExc>("ExfullExc","iex");
    registerExc<EnoanoExc,ErrnoExc>("EnoanoExc","iex");
    registerExc<EbadrqcExc,ErrnoExc>("EbadrqcExc","iex");
    registerExc<EbadsltExc,ErrnoExc>("EbadsltExc","iex");
    registerExc<EdeadlockExc,ErrnoExc>("EdeadlockExc","iex");
    registerExc<EbfontExc,ErrnoExc>("EbfontExc","iex");
    registerExc<EnostrExc,ErrnoExc>("EnostrExc","iex");
    registerExc<EnodataExc,ErrnoExc>("EnodataExc","iex");
    registerExc<EtimeExc,ErrnoExc>("EtimeExc","iex");
    registerExc<EnosrExc,ErrnoExc>("EnosrExc","iex");
    registerExc<EnonetExc,ErrnoExc>("EnonetExc","iex");
    registerExc<EnopkgExc,ErrnoExc>("EnopkgExc","iex");
    registerExc<EremoteExc,ErrnoExc>("EremoteExc","iex");
    registerExc<EnolinkExc,ErrnoExc>("EnolinkExc","iex");
    registerExc<EadvExc,ErrnoExc>("EadvExc","iex");
    registerExc<EsrmntExc,ErrnoExc>("EsrmntExc","iex");
    registerExc<EcommExc,ErrnoExc>("EcommExc","iex");
    registerExc<EprotoExc,ErrnoExc>("EprotoExc","iex");
    registerExc<EmultihopExc,ErrnoExc>("EmultihopExc","iex");
    registerExc<EbadmsgExc,ErrnoExc>("EbadmsgExc","iex");
    registerExc<EnametoolongExc,ErrnoExc>("EnametoolongExc","iex");
    registerExc<EoverflowExc,ErrnoExc>("EoverflowExc","iex");
    registerExc<EnotuniqExc,ErrnoExc>("EnotuniqExc","iex");
    registerExc<EbadfdExc,ErrnoExc>("EbadfdExc","iex");
    registerExc<EremchgExc,ErrnoExc>("EremchgExc","iex");
    registerExc<ElibaccExc,ErrnoExc>("ElibaccExc","iex");
    registerExc<ElibbadExc,ErrnoExc>("ElibbadExc","iex");
    registerExc<ElibscnExc,ErrnoExc>("ElibscnExc","iex");
    registerExc<ElibmaxExc,ErrnoExc>("ElibmaxExc","iex");
    registerExc<ElibexecExc,ErrnoExc>("ElibexecExc","iex");
    registerExc<EilseqExc,ErrnoExc>("EilseqExc","iex");
    registerExc<EnosysExc,ErrnoExc>("EnosysExc","iex");
    registerExc<EloopExc,ErrnoExc>("EloopExc","iex");
    registerExc<ErestartExc,ErrnoExc>("ErestartExc","iex");
    registerExc<EstrpipeExc,ErrnoExc>("EstrpipeExc","iex");
    registerExc<EnotemptyExc,ErrnoExc>("EnotemptyExc","iex");
    registerExc<EusersExc,ErrnoExc>("EusersExc","iex");
    registerExc<EnotsockExc,ErrnoExc>("EnotsockExc","iex");
    registerExc<EdestaddrreqExc,ErrnoExc>("EdestaddrreqExc","iex");
    registerExc<EmsgsizeExc,ErrnoExc>("EmsgsizeExc","iex");
    registerExc<EprototypeExc,ErrnoExc>("EprototypeExc","iex");
    registerExc<EnoprotooptExc,ErrnoExc>("EnoprotooptExc","iex");
    registerExc<EprotonosupportExc,ErrnoExc>("EprotonosupportExc","iex");
    registerExc<EsocktnosupportExc,ErrnoExc>("EsocktnosupportExc","iex");
    registerExc<EopnotsuppExc,ErrnoExc>("EopnotsuppExc","iex");
    registerExc<EpfnosupportExc,ErrnoExc>("EpfnosupportExc","iex");
    registerExc<EafnosupportExc,ErrnoExc>("EafnosupportExc","iex");
    registerExc<EaddrinuseExc,ErrnoExc>("EaddrinuseExc","iex");
    registerExc<EaddrnotavailExc,ErrnoExc>("EaddrnotavailExc","iex");
    registerExc<EnetdownExc,ErrnoExc>("EnetdownExc","iex");
    registerExc<EnetunreachExc,ErrnoExc>("EnetunreachExc","iex");
    registerExc<EnetresetExc,ErrnoExc>("EnetresetExc","iex");
    registerExc<EconnabortedExc,ErrnoExc>("EconnabortedExc","iex");
    registerExc<EconnresetExc,ErrnoExc>("EconnresetExc","iex");
    registerExc<EnobufsExc,ErrnoExc>("EnobufsExc","iex");
    registerExc<EisconnExc,ErrnoExc>("EisconnExc","iex");
    registerExc<EnotconnExc,ErrnoExc>("EnotconnExc","iex");
    registerExc<EshutdownExc,ErrnoExc>("EshutdownExc","iex");
    registerExc<EtoomanyrefsExc,ErrnoExc>("EtoomanyrefsExc","iex");
    registerExc<EtimedoutExc,ErrnoExc>("EtimedoutExc","iex");
    registerExc<EconnrefusedExc,ErrnoExc>("EconnrefusedExc","iex");
    registerExc<EhostdownExc,ErrnoExc>("EhostdownExc","iex");
    registerExc<EhostunreachExc,ErrnoExc>("EhostunreachExc","iex");
    registerExc<EalreadyExc,ErrnoExc>("EalreadyExc","iex");
    registerExc<EinprogressExc,ErrnoExc>("EinprogressExc","iex");
    registerExc<EstaleExc,ErrnoExc>("EstaleExc","iex");
    registerExc<EioresidExc,ErrnoExc>("EioresidExc","iex");
    registerExc<EucleanExc,ErrnoExc>("EucleanExc","iex");
    registerExc<EnotnamExc,ErrnoExc>("EnotnamExc","iex");
    registerExc<EnavailExc,ErrnoExc>("EnavailExc","iex");
    registerExc<EisnamExc,ErrnoExc>("EisnamExc","iex");
    registerExc<EremoteioExc,ErrnoExc>("EremoteioExc","iex");
    registerExc<EinitExc,ErrnoExc>("EinitExc","iex");
    registerExc<EremdevExc,ErrnoExc>("EremdevExc","iex");
    registerExc<EcanceledExc,ErrnoExc>("EcanceledExc","iex");
    registerExc<EnolimfileExc,ErrnoExc>("EnolimfileExc","iex");
    registerExc<EproclimExc,ErrnoExc>("EproclimExc","iex");
    registerExc<EdisjointExc,ErrnoExc>("EdisjointExc","iex");
    registerExc<EnologinExc,ErrnoExc>("EnologinExc","iex");
    registerExc<EloginlimExc,ErrnoExc>("EloginlimExc","iex");
    registerExc<EgrouploopExc,ErrnoExc>("EgrouploopExc","iex");
    registerExc<EnoattachExc,ErrnoExc>("EnoattachExc","iex");
    registerExc<EnotsupExc,ErrnoExc>("EnotsupExc","iex");
    registerExc<EnoattrExc,ErrnoExc>("EnoattrExc","iex");
    registerExc<EdircorruptedExc,ErrnoExc>("EdircorruptedExc","iex");
    registerExc<EdquotExc,ErrnoExc>("EdquotExc","iex");
    registerExc<EnfsremoteExc,ErrnoExc>("EnfsremoteExc","iex");
    registerExc<EcontrollerExc,ErrnoExc>("EcontrollerExc","iex");
    registerExc<EnotcontrollerExc,ErrnoExc>("EnotcontrollerExc","iex");
    registerExc<EenqueuedExc,ErrnoExc>("EenqueuedExc","iex");
    registerExc<EnotenqueuedExc,ErrnoExc>("EnotenqueuedExc","iex");
    registerExc<EjoinedExc,ErrnoExc>("EjoinedExc","iex");
    registerExc<EnotjoinedExc,ErrnoExc>("EnotjoinedExc","iex");
    registerExc<EnoprocExc,ErrnoExc>("EnoprocExc","iex");
    registerExc<EmustrunExc,ErrnoExc>("EmustrunExc","iex");
    registerExc<EnotstoppedExc,ErrnoExc>("EnotstoppedExc","iex");
    registerExc<EclockcpuExc,ErrnoExc>("EclockcpuExc","iex");
    registerExc<EinvalstateExc,ErrnoExc>("EinvalstateExc","iex");
    registerExc<EnoexistExc,ErrnoExc>("EnoexistExc","iex");
    registerExc<EendofminorExc,ErrnoExc>("EendofminorExc","iex");
    registerExc<EbufsizeExc,ErrnoExc>("EbufsizeExc","iex");
    registerExc<EemptyExc,ErrnoExc>("EemptyExc","iex");
    registerExc<EnointrgroupExc,ErrnoExc>("EnointrgroupExc","iex");
    registerExc<EinvalmodeExc,ErrnoExc>("EinvalmodeExc","iex");
    registerExc<EcantextentExc,ErrnoExc>("EcantextentExc","iex");
    registerExc<EinvaltimeExc,ErrnoExc>("EinvaltimeExc","iex");
    registerExc<EdestroyedExc,ErrnoExc>("EdestroyedExc","iex");
}
