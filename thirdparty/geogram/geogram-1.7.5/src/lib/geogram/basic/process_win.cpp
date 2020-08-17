/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#include <geogram/basic/common.h>

#ifdef GEO_OS_WINDOWS

#include <geogram/basic/process.h>
#include <geogram/basic/process_private.h>
#include <geogram/basic/atomics.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/progress.h>

#include <sstream>
#include <windows.h>
#include <signal.h>
#include <new.h>
#include <rtcapi.h>
#include <psapi.h>

// MSVC++ 11.0 _MSC_VER = 1700  (2012)
// MSVC++ 10.0 _MSC_VER = 1600  (2010)
// MSVC++ 9.0  _MSC_VER = 1500  (2008)
// MSVC++ 8.0  _MSC_VER = 1400
// MSVC++ 7.1  _MSC_VER = 1310
// MSVC++ 7.0  _MSC_VER = 1300
// MSVC++ 6.0  _MSC_VER = 1200
// MSVC++ 5.0  _MSC_VER = 1100
//
// Thread pools are supported since Windows Vista (_WIN32_WINNT >= 0x600)
// See http://msdn.microsoft.com/fr-fr/library/aa383745.aspx
// for details on _WIN32_WINNT values.

#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x600)
// In addition, I deactivate support of thread pool
// for Visual C++ <= 2008 (not mandatory, but else
// we get runtime errors on Windows XP)
#if (_MSC_VER > 1500)
#define GEO_OS_WINDOWS_HAS_THREADPOOL
#endif
#endif

namespace {

    using namespace GEO;

    /**
     * \brief Windows Thread ThreadManager
     * \details
     * WindowsThreadManager is an implementation of ThreadManager that uses
     * Windows threads for running concurrent threads and control critical
     * sections.
     */
    class WindowsThreadManager : public ThreadManager {
    public:
        /**
         * \brief Creates and initializes the Windows ThreadManager
         */
        WindowsThreadManager() {
            InitializeCriticalSection(&lock_);
        }

        /** \copydoc GEO::ThreadManager::maximum_concurrent_threads() */
	index_t maximum_concurrent_threads() override {
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            return sysinfo.dwNumberOfProcessors;
        }

        /** \copydoc GEO::ThreadManager::enter_critical_section() */
	void enter_critical_section() override {
            EnterCriticalSection(&lock_);
        }

        /** \copydoc GEO::ThreadManager::leave_critical_section() */
	void leave_critical_section() override {
            LeaveCriticalSection(&lock_);
        }

    protected:
        /** \brief WindowsThreadManager destructor */
	~WindowsThreadManager() override {
            DeleteCriticalSection(&lock_);
        }

        /** \copydoc GEO::ThreadManager::run_concurrent_threads() */
	void run_concurrent_threads(
            ThreadGroup& threads, index_t max_threads
        ) override {
            // TODO: take max_threads into account
            geo_argused(max_threads);

            HANDLE* threadsHandle = new HANDLE[threads.size()];
            for(index_t i = 0; i < threads.size(); i++) {
                set_thread_id(threads[i],i);
                threadsHandle[i] = CreateThread(
                    nullptr, 0, run_thread, (void*) &threads[i], 0, nullptr
                );
            }
            WaitForMultipleObjects(
                DWORD(threads.size()), threadsHandle, TRUE, INFINITE
            );
            for(index_t i = 0; i < threads.size(); i++) {
                CloseHandle(threadsHandle[i]);
            }
            delete[] threadsHandle;
        }

        /**
         * \brief CreateThread callback for running a thread
         * \details This function is passed a void pointer \p p to a
         * Thread_var and invokes the Thread function run().
         * \param[in] p pointer to the Thread_var to be executed.
         * \return always 0.
         * \see Thread::run()
         */
        static DWORD WINAPI run_thread(LPVOID p) {
            Thread* thread = (*reinterpret_cast<Thread_var*>(p));
            // Sets the thread-local-storage instance pointer, so
            // that Thread::current() can retrieve it.
            set_current_thread(thread);
            thread->run();
            return 0;
        }

    private:
        CRITICAL_SECTION lock_;
    };

#ifdef GEO_OS_WINDOWS_HAS_THREADPOOL

    /**
     * \brief Windows ThreadPool ThreadManager
     * \details
     * WindowsThreadPoolManager is an implementation of ThreadManager that
     * uses Windows Threads Pools for running concurrent threads and control
     * critical sections.
     */
    class WindowsThreadPoolManager : public WindowsThreadManager {
    public:
        /**
         * \brief Creates and initializes the Windows Threads Pool manager
         */
        WindowsThreadPoolManager() {
            pool_ = CreateThreadpool(nullptr);
            InitializeThreadpoolEnvironment(&cbe_);
            cleanupGroup_ = CreateThreadpoolCleanupGroup();
            SetThreadpoolCallbackPool(&cbe_, pool_);
            SetThreadpoolCallbackCleanupGroup(&cbe_, cleanupGroup_, nullptr);
            // Rem: cannot do what follows, since
            // maximum_concurrent_threads is not initialized yet...
            // SetThreadpoolThreadMaximum(
            //   pool_, Process::maximum_concurrent_threads()
            // );
            // SetThreadpoolThreadMinimum(
            //   pool_, Process::maximum_concurrent_threads()
            // );
            threadCounter_ = 0;
        }

    protected:
        /** \brief WindowsThreadPoolManager destructor */
	~WindowsThreadPoolManager() override {
// It makes it crash on exit when calling these functions
// with dynamic libs, I do not know why...            
// TODO: investigate...
#ifndef GEO_DYNAMIC_LIBS            
            CloseThreadpool(pool_);
            CloseThreadpoolCleanupGroup(cleanupGroup_);
#endif            
        }

        /** \copydoc GEO::ThreadManager::run_concurrent_threads() */
	void run_concurrent_threads(
            ThreadGroup& threads, index_t max_threads
        ) override {
            // TODO: take max_threads into account
            geo_argused(max_threads);

            // geo_assert(!Process::is_running_threads());
            // --> no, this doesn't work
            TP_WORK* worker = CreateThreadpoolWork(
                run_thread, (void*) &threads, &cbe_
            );
            threadCounter_ = 0;
            for(index_t i = 0; i < threads.size(); i++) {
                SubmitThreadpoolWork(worker);
            }
            WaitForThreadpoolWorkCallbacks(worker, FALSE);
            CloseThreadpoolWork(worker);
        }

        /**
         * \brief CreateThreadpoolWork callback for running a thread
         * \details This function is passed a void pointer \p to the vector of
         * Thread_var%s to run. The index of the thread to be executed is
         * contained in the static counter threadCounter_ This index is
         * incremented at each call of the function.
         * \param[in] Instance pointer to the current ThreadPool (unused)
         * \param[in] Context pointer to the vector of threads to run
         * \param[in] Work pointer to the current ThreadpoolWork (unused)
         */
        static VOID CALLBACK run_thread(
            PTP_CALLBACK_INSTANCE Instance,
            PVOID Context,
            PTP_WORK Work
        ) {
            geo_argused(Work);
            geo_argused(Instance);
            ThreadGroup& threads = *reinterpret_cast<ThreadGroup*>(Context);
            LONG id = InterlockedIncrement(&threadCounter_);
            DWORD tid = (id - 1) % threads.size();
            set_thread_id(threads[tid], tid);
            // Sets the thread-local-storage instance pointer, so
            // that Thread::current() can retreive it.
            set_current_thread(threads[tid]);
            threads[tid]->run();
        }

    private:
        PTP_POOL pool_;
        TP_CALLBACK_ENVIRON cbe_;
        PTP_CLEANUP_GROUP cleanupGroup_;
        static volatile LONG threadCounter_;
    };

    volatile long WindowsThreadPoolManager::threadCounter_ = 0;

#endif

#ifdef GEO_COMPILER_MSVC    
    /**
     * \brief Abnormal termination handler
     * \details Exits the program. If \p message is
     * non nullptr, the following message is printed before the stacktrace:
     * <em>Abnormal program termination: message</em>
     * \param[in] message optional message to print
     */
    void abnormal_program_termination(const char* message = nullptr) {
        if(message != nullptr) {
            // Do not use Logger here!
            std::cout
                << "Abnormal program termination: "
                << message << std::endl;
        }
        ExitProcess(1);
    }

    /**
     * \brief Signal handler
     * \details The handler exits the application
     * \param[in] signal signal number
     */
    void signal_handler(int signal) {
        const char* sigstr;
        switch(signal) {
            case SIGINT:
                sigstr = "SIGINT";
                break;
            case SIGILL:
                sigstr = "SIGILL";
                break;
            case SIGFPE:
                sigstr = "SIGFPE";
                break;
            case SIGSEGV:
                sigstr = "SIGSEGV";
                break;
            case SIGTERM:
                sigstr = "SIGTERM";
                break;
            case SIGBREAK:
                sigstr = "SIGBREAK";
                break;
            case SIGABRT:
                sigstr = "SIGABRT";
                break;
            default:
                sigstr = "UNKNOWN";
                break;
        }

        std::ostringstream os;
        os << "received signal " << signal << " (" << sigstr << ")";
        abnormal_program_termination(os.str().c_str());
    }

    /**
     * \brief Floating point error handler
     * \details The handler exits the application
     * \param[in] signal signal number
     * \param[in] code type of the FPE error
     */
    void fpe_signal_handler(int /*signal*/, int code) {
        const char* error;
        switch(code) {
            case _FPE_INVALID:
                error = "invalid number";
                break;
            case _FPE_OVERFLOW:
                error = "overflow";
                break;
            case _FPE_UNDERFLOW:
                error = "underflow";
                break;
            case _FPE_ZERODIVIDE:
                error = "divide by zero";
                break;
            default:
                error = "unknown";
                break;
        }

        std::ostringstream os;
        os << "floating point exception detected: " << error;
        abnormal_program_termination(os.str().c_str());
    }

    /**
     * \brief Interrupt signal handler
     * \details The handler cancels the current task if any or exits the
     * program.
     */
    void sigint_handler(int) {
        if(Progress::current_task() != nullptr) {
            Progress::cancel();
        } else {
            exit(1);
        }
    }

    /**
     * Catches uncaught C++ exceptions
     */
    void uncaught_exception_handler() {
        abnormal_program_termination("function terminate() was called");
    }

// Disable the "unreachable code" warning issued by
// Microsoft Visual C++
// (abnormal_program_termination() does not return,
//  but memory_exhausted_handler() needs to return
//  something...)    
#ifdef GEO_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
    
    /**
     * Catches allocation errors
     */
    int memory_exhausted_handler(size_t) {
        abnormal_program_termination("memory exhausted");
        return 0; 
    }

#ifdef GEO_COMPILER_MSVC    
#pragma warning(pop)
#endif
    
    /**
     * Catches invalid calls to pure virtual functions
     */
    void pure_call_handler() {
        abnormal_program_termination("pure virtual function called");
    }

    /**
     * Catches debug runtime errors due to invalid parameters
     *
     * Example: the following code throws a runtime assertion error
     * \code
     * printf(nullptr);
     * \endcode
     */
    void invalid_parameter_handler(
        const wchar_t* expression,
        const wchar_t* function,
        const wchar_t* file,
        unsigned int line,
        uintptr_t /*pReserved*/
    ) {
        wprintf(
            L"Abnormal program termination: Invalid parameter detected.\n"
            L"Function: %s\nFile: %s\nLine: %d\n",
            function, file, int(line)
        );
        wprintf(L"Expression: %s\n", expression);
        abnormal_program_termination();
    }

    /**
     * Catches runtime check exceptions
     *
     * Example: the following code throws a "uninitialized variable"
     * exception.
     * \code
     * float a;
     * float b = a * 1;
     * \endcode
     */
    int runtime_error_handler(
        int /*errorType*/,
        const wchar_t* filename,
        int linenumber,
        const wchar_t* moduleName,
        const wchar_t* format,
        ...
    ) {
        va_list vl;
        va_start(vl, format);

        wprintf(L"Abnormal program termination: ");
        vwprintf(format, vl);
        wprintf(
            L"\nModule: %s\nFile: %s\nLine: %d\n", 
            moduleName, filename, linenumber

        );
        va_end(vl);
        
        // Must return 1 to force the program to stop with an exception which
        // will be captured by the unhandled exception handler
        return 1;
    }

    /**
     * Debug report hook
     *
     * This function is called by _CrtDbgReport to handle custom
     * reporting. We use this function to print the message to the
     * standard error and exit the application (except for warning
     * messages).
     */
    int debug_report_hook(int reportType, char* message, int* returnValue) {
        if(reportType != _CRT_WARN) {
            // Critical error: exit the application
            abnormal_program_termination(message);
        }

        // Runtime warning messages        
        if(Logger::is_initialized()) {
            Logger::err("SignalHandler") << message << std::endl;
        } else {
            fprintf(stderr, "SignalHandler: %s\n", message);
        }

        // Tell _CrtDbgReport to continue processing
        if(returnValue != 0) {
            *returnValue = 0;
        }

        // Tell _CrtDbgReport that no further reporting is required.
        return TRUE;
    }
    
#endif
    
}

/****************************************************************************/

namespace GEO {

    namespace Process {

        bool os_init_threads() {
#ifdef GEO_COMPILER_MSVC
#  ifdef GEO_OS_WINDOWS_HAS_THREADPOOL
            // Env. variable to deactivate thread pool, e.g.
            // used under Wine (that does not implement thread pools yet).
            if(::getenv("GEO_NO_THREAD_POOL")) {
                Logger::out("Process")
                    << "Windows thread pool disabled by GEO_NO_THREAD_POOL"
                    << ", using Windows threads"
                    << std::endl;
                set_thread_manager(new WindowsThreadManager);
            } else {
                Logger::out("Process")
                    << "Using Windows thread pool"
                    << std::endl;
                set_thread_manager(new WindowsThreadPoolManager);
            }
            return true;
#  else
            Logger::out("Process")
                << "Windows thread pool not supported, using Windows threads"
                << std::endl;
            set_thread_manager(new WindowsThreadManager);
            return true;
#  endif
#else
	   // If compiling for Windows with a compiler different from MSVC, 
	   // return false, and use OpenMP fallback from process.cpp
	   return false;
#endif	   
        }

        void os_brute_force_kill() {
            // Get the pid of this process
            DWORD processId = GetCurrentProcessId();

            // then modify its privileges to allow full access
            HANDLE hHandle;

            hHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION, 0, processId);
            HANDLE tokHandle;
            OpenProcessToken(hHandle, TOKEN_ALL_ACCESS, &tokHandle);

            TOKEN_PRIVILEGES tp;
            LUID luid;
            LookupPrivilegeValue(
                nullptr,            // lookup privilege on local system
                SE_DEBUG_NAME,   // privilege to lookup
                &luid);

            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            // Enable the privilege.

            AdjustTokenPrivileges(
                tokHandle,
                FALSE,
                &tp,
                sizeof(TOKEN_PRIVILEGES),
                (PTOKEN_PRIVILEGES) nullptr,
                (PDWORD) nullptr
            );

            if(hHandle == nullptr) {
                DWORD err = GetLastError();
                geo_argused(err);
            }

            // kill the process in a quite brutal way...
            HANDLE hHandle2 = ::OpenProcess(PROCESS_ALL_ACCESS, 0, processId);
            DWORD dwExitCode = 0;
            // we don't need to know the current state of the process :
            //   it is STILL_ACTIVE (259)
            // and we want this termination to look normal (exit with code 0)
            // ::GetExitCodeProcess(hHandle2,&dwExitCode);
            ::TerminateProcess(hHandle2, dwExitCode);
        }

        index_t os_number_of_cores() {
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            return si.dwNumberOfProcessors;
        }

        size_t os_used_memory() {
#ifdef GEO_COMPILER_MSVC
            PROCESS_MEMORY_COUNTERS info;
#if (PSAPI_VERSION >= 2)
            K32GetProcessMemoryInfo(GetCurrentProcess( ), &info, sizeof(info));            
#else            
            GetProcessMemoryInfo(GetCurrentProcess( ), &info, sizeof(info));
#endif            
            return size_t(info.WorkingSetSize);
#else
	   return size_t(0);
#endif	   
        }

        size_t os_max_used_memory() {
#ifdef GEO_COMPILER_MSVC	   
            PROCESS_MEMORY_COUNTERS info;
#if (PSAPI_VERSION >= 2)
            K32GetProcessMemoryInfo(GetCurrentProcess( ), &info, sizeof(info));            
#else            
            GetProcessMemoryInfo(GetCurrentProcess( ), &info, sizeof(info));
#endif            
            return size_t(info.PeakWorkingSetSize);
#else
	   return size_t(0);
#endif	   
        }

        bool os_enable_FPE(bool flag) {
#ifdef GEO_COMPILER_MSVC	    
            if(flag) {
                unsigned int excepts = 0
                    // | _EM_INEXACT // inexact result
                    | _EM_ZERODIVIDE // division by zero
                    | _EM_UNDERFLOW // result not representable due to underflow
                    | _EM_OVERFLOW  // result not representable due to overflow
                    | _EM_INVALID   // invalid operation
                ;
                _clearfp();
                _controlfp(~excepts, _MCW_EM);
            } else {
                _controlfp(_MCW_EM, _MCW_EM);
            }
            return true;
#else
	    geo_argused(flag);
	    return false;
#endif	    
        }

        bool os_enable_cancel(bool flag) {
#ifdef GEO_COMPILER_MSVC	    
            if(flag) {
                signal(SIGINT, sigint_handler);
            } else {
                signal(SIGINT, SIG_DFL);
            }
            return true;
#else
	    geo_argused(flag);
	    return false;
#endif	    
        }

        /**
         * \brief Installs signal handlers
         * \details
         * On Windows, this installs standard signal handlers and all kind of
         * exception handling routines that prevent the application from being
         * blocked by a bad assertion, a runtime check or runtime error dialog.
         */
#ifdef GEO_COMPILER_MSVC	
        void os_install_signal_handlers() {

            // Install signal handlers
            signal(SIGSEGV, signal_handler);
            signal(SIGILL, signal_handler);
            signal(SIGBREAK, signal_handler);
            signal(SIGTERM, signal_handler);

            // SIGFPE has a dedicated handler 
            // that provides more details about the error.
            typedef void (__cdecl * sighandler_t)(int);
            signal(SIGFPE, (sighandler_t) fpe_signal_handler);

            // Install uncaught c++ exception handlers	    
            std::set_terminate(uncaught_exception_handler);

            // Install memory allocation handler
            _set_new_handler(memory_exhausted_handler);
            // Also catch malloc errors
            _set_new_mode(1);

            // Install Windows runtime error handlers.
            // This code and the above is inspired from a very good article
            // "Effective Exception Handling in Visual C++" available here:
            // http://www.codeproject.com/Articles/207464/Exception-Handling-in-Visual-Cplusplus

            // Catch calls to pure virtual functions
            _set_purecall_handler(pure_call_handler);

            // Catch abort and assertion failures
            // By default abort() error messages are sent to a dialog box
            // which blocks the application. This is a very annoying behavior
            // especially during test sessions.
            // -> Redirect abort() messages to standard error.
            signal(SIGABRT, signal_handler);
            _set_abort_behavior(0, _WRITE_ABORT_MSG);

            // Catch "invalid parameter" runtime assertions
            _set_invalid_parameter_handler(invalid_parameter_handler);

            // Catch runtime check errors
            _RTC_SetErrorFuncW(runtime_error_handler);

            // Some debug runtime errors are not caught by the error handlers
            // installed above. We must install a custom report hook called by
            // _CrtDbgReport that prints the error message, print the stack
            // trace and exit the application.
            // NOTE: when this hook is installed, it takes precedence over the
            // invalid_parameter_handler(), but not over the 
            // runtime_error_handler().
            // Windows error handling is a nightmare!
            _CrtSetReportHook(debug_report_hook);

            // By default runtime error messages are sent to a dialog box
            // which blocks the application. This is a very annoying behavior
            // especially during test sessions.
            // -> Redirect runtime messages to standard error by security
            _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
 
            // Do not open dialog box on error 
	    SetErrorMode(SEM_NOGPFAULTERRORBOX);
        }
#else
        void os_install_signal_handlers() {
	}
#endif	
	
        /**
         * \brief Gets the full path to the current executable.
         */
        std::string os_executable_filename() {
            TCHAR result[MAX_PATH];
            GetModuleFileName( nullptr, result, MAX_PATH);
            return std::string(result);
        }
    }
}

#endif

