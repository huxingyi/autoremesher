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

#ifndef GEOGRAM_BASIC_PROCESS
#define GEOGRAM_BASIC_PROCESS

#include <geogram/basic/common.h>
#include <geogram/basic/thread_sync.h>
#include <geogram/basic/counted.h>
#include <geogram/basic/smart_pointer.h>
#include <functional>

/**
 * \file geogram/basic/process.h
 * \brief Function and classes for process manipulation
 */

namespace GEO {

// thread_local is supposed to be supported by c++0x,
// but some old MSVC compilers do not have it.    
#if defined(GEO_COMPILER_MSVC) && !defined(thread_local)
#  define thread_local __declspec(thread)
#endif

// Older MAC OS X do not have thread_local
#ifdef GEO_OS_APPLE
# if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_9
#  define thread_local
#  define GEO_NO_THREAD_LOCAL    
# endif
#endif
   
    /**
     * \brief Platform-independent base class for running threads.
     * \details
     * A Thread object manages one thread of control within the program.
     * Thread%s begin executing with run(). Operational threads can be created
     * by creating a derived class and reimplement function run().
     *
     * Thread%s are reference-counted objects. Their allocation and
     * destruction can be automatically managed with Thread_var.
     */
    class GEOGRAM_API Thread : public Counted {
    public:

        /**
         * \brief Thread constructor.
         */
        Thread() : id_(0) {
        }

        /**
         * \brief Starts the thread execution.
         */
        virtual void run() = 0;

        /**
         * \brief Gets the identifier of this thread.
         * \return the identifier of the thread, i.e. 
         *  an unsigned integer in the range [0, N-1] 
         *  where N denotes the number of currently 
         *  running threads.
         */
        index_t id() const {
            return id_;
        }

        /**
         * \brief Gets the current thread.
         * \return A pointer to the instance of the
         *  currently running thread.
         */
        static Thread* current();


    protected:
        /** Thread destructor */
        virtual ~Thread();


    private:
        /**
         * \brief Sets the identifier of this thread.
         * \details This function is meant to be called
         *  by the thread manager for each created thread.
         * \param[in] id_in the identifier of this thread.
         */
        void set_id(index_t id_in) {
            id_ = id_in;
        }

        /**
         * \brief Specifies the current instance, used by current().
         * \details Stores the specified thread in the thread-local-storage
         *   static variable so that current() can retrieve it. 
         *   Should be called by the ThreadManager right before launching 
         *   the threads.
         * \param[in] thread a pointer to the thread currently executed
         */   
        static void set_current(Thread* thread);

        index_t id_;

        // ThreadManager needs to access set_current() and 
        // set_id().
        friend class ThreadManager;
    };

    /** Smart pointer that contains a Thread object */
    typedef SmartPointer<Thread> Thread_var;

    /**
     * \brief Collection of Thread%s
     * \details ThreadGroup is a std::vector of Thread_var it provides the
     * same operations for adding, removing or accessing thread elements.
     * ThreadGroup takes ownership of Thread elements when they are added to
     * the group, so there's is no need to delete Threads when the group is
     * deleted.
     */
    typedef std::vector<Thread_var> ThreadGroup;

    /**
     * \brief Typed collection of Thread%s.
     * \details
     * TypedThreadGroup is a ThreadGroup that provides a typed accessor with
     * operator[]().
     * \tparam THREAD the type of Thread%s in the collection
     */
    template <class THREAD>
    class TypedThreadGroup : public ThreadGroup {
    public:
        /**
         * \brief Creates an empty group of Thread%s
         * \details Thread elements can be added with the std::vector
         * operation push_back()
         */
        TypedThreadGroup() {
        }

        /**
         * \brief Gets a thread element by index
         * \param[in] i index of the element
         * \return a pointer to the \p THREAD at position \p i in the
         * thread group
         */
        THREAD* operator[] (index_t i) {
            geo_debug_assert(i < size());
            Thread* result = ThreadGroup::operator[] (i);
            return static_cast<THREAD*>(result);
        }
    };

    /**
     * \brief Platform-independent base class for running concurrent threads.
     * \details
     * The ThreadManager manager provides a platform-independent abstract
     * interface for running concurrent Threads and managing critical
     * sections.
     *
     * The ThreadManager is derived in multiple platform-specific or
     * technology-specific implementations.
     *
     * Platform-specific implementations:
     * - POSIX Thread manager (Unix)
     * - Windows Threads manager (Windows)
     * - Windows ThreadPool manager (Windows)
     *
     * Technology-specific implementations:
     * - OpenMP-based manager
     *
     * Which ThreadManager to use is determined at runtime by
     * Process::initialize() according to the current platform or the current
     * available technology.
     *
     * \note For internal use only.
     * \see Process::set_thread_manager()
     */
    class GEOGRAM_API ThreadManager : public Counted {
    public:
        /**
         * \brief Runs a group of Thread%s.
         * \details
         * This start the execution of the threads
         * contained in vector \p threads.
         *
         * If the threads cannot be executed in a concurrent environment
         * (multi-threading is disabled or the number of maximum threads is 1),
         * then the threads are executed sequentially. Otherwise the function
         * run_concurrent_threads() is called to execute the threads
         * concurrently. The execution terminates when the last thread
         * terminates.
         *
         * \param[in] threads the vector of threads to be executed.
         * \see maximum_concurrent_threads()
         * \see run_concurrent_threads()
         * \see Process::max_threads()
         */
        virtual void run_threads(ThreadGroup& threads);

        /**
         * \brief Gets the maximum number of possible concurrent threads
         * \return The maximum number of possible concurrent threads allowed
         * by this manager. It depends on the physical number of cores
         * (including hyper-threading or not) and the technology implemented
         * by this manager.
         * \see Process::number_of_cores()
         */
        virtual index_t maximum_concurrent_threads() = 0;

        /**
         * \brief Enters a critical section
         * \details
         * One thread at a time can enter the critical section, all the other
         * threads that call this function are blocked until the blocking
         * thread leaves the critical section.
         * \see leave_critical_section()
         */
        virtual void enter_critical_section() = 0;

        /**
         * \brief Leaves a critical section
         * \details When a blocking thread leaves a critical section, this
         * makes the critical section available for a waiting thread.
         * \see enter_critical_section()
         */
        virtual void leave_critical_section() = 0;

    protected:
        /**
         * \brief Runs a group of Thread%s concurrently.
         * \details This start the concurrent execution of the threads
         * contained in vector \p threads, using the given number of threads
         * \p max_threads. The execution terminates when the last thread
         * terminates.
         * \param[in] threads the vector of threads to be executed.
         * \param[in] max_threads maximum number of threads allowed for this
         * execution. It is always greater than one
         */
        virtual void run_concurrent_threads(
            ThreadGroup& threads, index_t max_threads
        ) = 0;


        /**
         * \brief Sets the id of a thread.
         * \details This function is called right before starting
         *  the threads. Each thread will have an id in [0, N-1]
         *  where N denotes the number of running threads.
         * \param[in] thread the thread
         * \param[in] id the id
         */
        static void set_thread_id(Thread* thread, index_t id) {
            thread->set_id(id);
        }

        /**
         * \brief Specifies the current instance, used by current().
         * \details Stores the specified thread in the thread-local-storage
         *   static variable so that current() can retrieve it. 
         *   Should be called by the ThreadManager right before launching 
         *   the threads.
         * \param[in] thread a pointer to the thread currently executed
         */   
        static void set_current_thread(Thread* thread) {
            Thread::set_current(thread);
        }

        /** ThreadManager destructor */
        virtual ~ThreadManager();
    };

    /** Smart pointer that contains a ThreadManager object */
    typedef SmartPointer<ThreadManager> ThreadManager_var;

    /**
     * \brief Single thread ThreadManager
     * \details MonoThreadingThreadManager implements a ThreadManager for
     * single thread environments.
     */
    class GEOGRAM_API MonoThreadingThreadManager : public ThreadManager {
    public:
        /**
         * \copydoc ThreadManager::maximum_concurrent_threads()
         * \note This implementation always returns 1.
         */
        virtual index_t maximum_concurrent_threads();

        /**
         * \copydoc ThreadManager::enter_critical_section()
         * \note This implementation does actually nothing
         */
        virtual void enter_critical_section();

        /**
         * \copydoc ThreadManager::leave_critical_section()
         * \note This implementation does actually nothing
         */
        virtual void leave_critical_section();

    protected:
        /** MonoThreadingThreadManager destructor */
        virtual ~MonoThreadingThreadManager();

        /**
         * \copydoc ThreadManager::run_concurrent_threads()
         * \note This implementation always executes threads sequentially.
         */
        virtual void run_concurrent_threads(
            ThreadGroup& threads, index_t max_threads
        );
    };

    /**
     * \brief Abstraction layer for process management and multi-threading.
     */
    namespace Process {

        /**
         * \brief Initializes GeogramLib
	 * \param[in] flags the flags passed to GEO::initialize()
         * \details This function must be called once before using
         * any functionality of GeogramLib.
         */
        void GEOGRAM_API initialize(int flags);

        /**
         * \brief Terminates GeogramLib
         * \details This function is called automatically when the program
         * exits, so it should never be called directly.
         */
        void GEOGRAM_API terminate();


	/**
	 * \brief Sleeps for a period of time.
	 * \param[in] microseconds the time to sleep,
	 *  in microseconds.
	 */
	void GEOGRAM_API sleep(index_t microseconds);
	
        /**
         * \brief Displays statistics about the current process
         * \details Displays the maximum used amount of memory.
         */
        void GEOGRAM_API show_stats();
        
        /**
         * \brief Terminates the current process.
         */
        void GEOGRAM_API brute_force_kill();

        /**
         * \brief Returns the maximum number of threads that can be running
         * simultaneously.
         * \retval The number of cores if multi-threading is supported
         * \retval 1 otherwise.
         */
        index_t GEOGRAM_API maximum_concurrent_threads();

        /**
         * \brief Runs a set of threads simultaneously
         * \details Launches the execution of the threads contained in the
         * vector \p threads and waits for the completion of all of them.
         */
        void GEOGRAM_API run_threads(ThreadGroup& threads);

        /**
         * \brief Enters a critical section
         * \details One thread at a time can enter the critical section,
         * all the other threads that call this function are blocked until the
         * blocking thread leaves the critical section
         * \see ThreadManager::enter_critical_section()
         * \see leave_critical_section()
         */
        void GEOGRAM_API enter_critical_section();

        /**
         * \brief Leaves a critical section
         * \details When a blocking thread leaves a critical section, this
         * makes the critical section available for a waiting thread.
         * \see ThreadManager::leave_critical_section()
         * \see enter_critical_section()
         */
        void GEOGRAM_API leave_critical_section();

        /**
         * \brief Gets the number of available cores
         * \return The number of available cores including the "virtual ones" if
         * hyper-threading is activated.
         */
        index_t GEOGRAM_API number_of_cores();

        /**
         * \brief Sets the thread manager (internal use).
         * \details This sets the ThreadManager to use for concurrent thread
         * execution. This function is called internally by
         * Process::initialize() and should not be called explicitly.
         * \note For internal use only
         */
        void GEOGRAM_API set_thread_manager(ThreadManager* thread_manager);

        /**
         * \brief Checks whether threads are running.
         * \retval true if concurrent threads are currently running as an
         * effect to Process::run_threads().
         * \retval false otherwise.
         * \see Process::run_threads()
         */
        bool GEOGRAM_API is_running_threads();

        /**
         * \brief Enables/disables floating point exceptions
         * \details If FPEs are enabled, then floating point exceptions
         * raise a SIGFPE signal, otherwise they generate NaNs. FPEs can also
         * be configured by setting the value of the property "sys:FPE" with
         * Environment::set_value().
         * \param[in] flag set to \c true to enable FPEs, \c false to disable.
         * \see FPE_enabled()
         */
        void GEOGRAM_API enable_FPE(bool flag);

        /**
         * \brief Gets the status of floating point exceptions
         * \retval true if FPE are enabled
         * \retval false otherwise
         * \see enable_FPE()
         */
        bool GEOGRAM_API FPE_enabled();

        /**
         * \brief Enables/disables multi-threaded computations
         * Multi-threading can also be configured by setting the value of the
         * property "sys:multithread" with Environment::set_value().
         * \param[in] flag set to \c true to enable multi-threading, \c false
         * to disable.
         * \see multithreading_enabled()
         */
        void GEOGRAM_API enable_multithreading(bool flag);

        /**
         * \brief Gets the status of multi-threading
         * \retval true if multi-threading is enabled
         * \retval false otherwise
         * \see enable_multithreading()
         */
        bool GEOGRAM_API multithreading_enabled();

        /**
         * \brief Limits the number of concurrent threads to use
         * \details The number of threads can also be configured by setting
         * the value of the property "sys:max_threads" with
         * Environment::set_value().
         * \param[in] num_threads maximum number of threads to use.
         * \see max_threads()
         */
        void GEOGRAM_API set_max_threads(index_t num_threads);

        /**
         * \brief Gets the number of allowed concurrent threads
         * \see set_max_threads()
         */
        index_t GEOGRAM_API max_threads();

        /**
         * \brief Enables interruption of cancelable tasks
         * \details This allows to interrupt cancelable tasks by typing
         * CTRL-C in the terminal. This sets a specific handler on the
         * interrupt signal that calls Progress::cancel() is there is a
         * running cancelable task. If no task is running, the program is
         * interrupted. The cancel mode can also be configured by setting the
         * value of the property "sys:cancel" with
         * Environment::set_value().
         * \param[in] flag set to \c true to enable cancel mode, \c false
         * to disable.
         * \see cancel_enabled()
         */
        void GEOGRAM_API enable_cancel(bool flag);

        /**
         * \brief Gets the status of the cancel mode
         * \retval true if the cancel mode is enabled
         * \retval false otherwise
         * \see enable_cancel()
         */
        bool GEOGRAM_API cancel_enabled();

        /**
         * \brief Gets the currently used memory.
         * \return the used memory in bytes
         */
        size_t GEOGRAM_API used_memory();

        /**
         * \brief Gets the maximum used memory.
         * \return the maximum used memory in bytes
         */
        size_t GEOGRAM_API max_used_memory();


        /**
         * \brief Gets the full path to the currently
         *  running program.
         */
        std::string GEOGRAM_API executable_filename();
    }

    /**
     * \brief Executes a loop with concurrent threads.
     * \details
     *   Executes a parallel for loop from index \p to index \p to, calling
     *   functional object \p func at each iteration.
     *
     * Calling parallel_for(from, to, func) is equivalent
     * to the following loop, computed in parallel:
     * \code
     * for(index_t i = from; i < to; i++) {
     *    func(i)
     * }
     * \endcode
     *
     * When applicable, iterations are executed by concurrent threads: 
     * the range of the loop is split in to several contiguous
     * sub-ranges, each of them being executed by a separate thread.
     *
     * If parameter \p interleaved is set to true, the loop range is
     * decomposed in interleaved index sets. Interleaved execution may 
     * improve cache coherency.
     *
     * \param[in] func function that takes an index_t.
     * \param[in] from the first iteration index
     * \param[in] to one position past the last iteration index
     * \param[in] threads_per_core number of threads to allocate per physical
     *  core (default is 1).
     * \param[in] interleaved if set to \c true, indices are allocated to
     * threads with an interleaved pattern.
     */
     void GEOGRAM_API parallel_for(
        index_t from, index_t to, std::function<void(index_t)> func,
        index_t threads_per_core = 1,
        bool interleaved = false
    );

    /**
     * \brief Executes a loop with concurrent threads.
     *
     * \details
     * When applicable, iterations are executed by concurrent 
     * threads: the range of the loop is split in to several contiguous
     * sub-ranges, each of them being executed by a separate thread.
     *
     * Calling parallel_for(func, from, to) is equivalent
     * to the following loop, computed in parallel:
     * \code
     *   func(from, i1);
     *   func(i1, i2);
     *   ...
     *   func(in, to);
     * \endcode
     * where i1,i2,...in are automatically generated. Typically one interval
     * per physical core is generated.
     *
     * \param[in] func functional object that accepts two arguments of
     *  type index_t.
     * \param[in] from first iteration index of the loop
     * \param[in] to one position past the last iteration index
     * \param[in] threads_per_core number of threads to allocate per physical
     *  core (default is 1).
     */
     void GEOGRAM_API parallel_for_slice(
	 index_t from, index_t to, std::function<void(index_t, index_t)> func,
	 index_t threads_per_core = 1
     );

     /**
      * \brief Calls functions in parallel.
      * \details Can be typically used with lambdas that capture this. See
      *  mesh/mesh_reorder.cpp and points/kd_tree.cpp for examples.
      * \param[in] f1 , f2 functions to be called in parallel.
      */
     void GEOGRAM_API parallel(
	 std::function<void()> f1,
	 std::function<void()> f2	 
     );

     /**
      * \brief Calls functions in parallel.
      * \details Can be typically used with lambdas that capture this. See
      *  mesh/mesh_reorder.cpp and points/kd_tree.cpp for examples.
      * \param[in] f1 , f2 , f3 , f4 functions to be called in parallel.
      */
     void GEOGRAM_API parallel(
	 std::function<void()> f1,
	 std::function<void()> f2,
	 std::function<void()> f3,
	 std::function<void()> f4	 
     );

     /**
      * \brief Calls functions in parallel.
      * \details Can be typically used with lambdas that capture this. See
      *  mesh/mesh_reorder.cpp and points/kd_tree.cpp for examples.
      * \param[in] f1 , f2 , f3 , f4 , f5 , f6 , f7 , f8 functions 
      *  to be called in parallel.
      */
     void GEOGRAM_API parallel(
	 std::function<void()> f1,
	 std::function<void()> f2,
	 std::function<void()> f3,
	 std::function<void()> f4,
	 std::function<void()> f5,
	 std::function<void()> f6,
	 std::function<void()> f7,
	 std::function<void()> f8	 
     );
     
}

#endif

