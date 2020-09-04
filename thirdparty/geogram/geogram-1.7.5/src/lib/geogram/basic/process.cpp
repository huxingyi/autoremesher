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

#include <geogram/basic/process.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/environment.h>
#include <geogram/basic/string.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/stopwatch.h>
#include <thread>
#include <chrono>

#ifdef GEO_OPENMP
#include <omp.h>
#endif

namespace {
    using namespace GEO;

    ThreadManager_var thread_manager_;
    int running_threads_invocations_ = 0;

    bool multithreading_initialized_ = false;
    bool multithreading_enabled_ = true;

    index_t max_threads_initialized_ = false;
    index_t max_threads_ = 0;

    bool fpe_initialized_ = false;
    bool fpe_enabled_ = false;

    bool cancel_initialized_ = false;
    bool cancel_enabled_ = false;

    double start_time_ = 0.0;

    /************************************************************************/

    /**
     * \brief Process Environment
     * \details This environment exposes and controls the configuration of the
     * Process module.
     */
    class ProcessEnvironment : public Environment {
    protected:
        /**
         * \brief Gets a Process property
         * \details Retrieves the value of the property \p name and stores it
         * in \p value. The property must be a valid Process property (see
         * sys:xxx properties in Vorpaline's help).
         * \param[in] name name of the property
         * \param[out] value receives the value of the property
         * \retval true if the property is a valid Process property
         * \retval false otherwise
         * \see Environment::get_value()
         */
        virtual bool get_local_value(
            const std::string& name, std::string& value
        ) const {
            if(name == "sys:nb_cores") {
                value = String::to_string(Process::number_of_cores());
                return true;
            }
            if(name == "sys:multithread") {
                value = String::to_string(multithreading_enabled_);
                return true;
            }
            if(name == "sys:max_threads") {
                value = String::to_string(
                    Process::maximum_concurrent_threads()
                );
                return true;
            }
            if(name == "sys:FPE") {
                value = String::to_string(fpe_enabled_);
                return true;
            }
            if(name == "sys:cancel") {
                value = String::to_string(cancel_enabled_);
                return true;
            }
            if(name == "sys:assert") {
                value = assert_mode() == ASSERT_THROW ? "throw" : "abort";
                return true;
            }
            return false;
        }

        /**
         * \brief Sets a Process property
         * \details Sets the property \p name with value \p value in the
         * Process. The property must be a valid Process property (see sys:xxx
         * properties in Vorpaline's help) and \p value must be a legal value
         * for the property.
         * \param[in] name name of the property
         * \param[in] value value of the property
         * \retval true if the property was successfully set
         * \retval false otherwise
         * \see Environment::set_value()
         */
        virtual bool set_local_value(
            const std::string& name, const std::string& value
        ) {
            if(name == "sys:multithread") {
                Process::enable_multithreading(String::to_bool(value));
                return true;
            }
            if(name == "sys:max_threads") {
                Process::set_max_threads(String::to_uint(value));
                return true;
            }
            if(name == "sys:FPE") {
                Process::enable_FPE(String::to_bool(value));
                return true;
            }
            if(name == "sys:cancel") {
                Process::enable_cancel(String::to_bool(value));
                return true;
            }
            if(name == "sys:assert") {
                if(value == "throw") {
                    set_assert_mode(ASSERT_THROW);
                    return true;
                }
		if(value == "abort") {
                    set_assert_mode(ASSERT_ABORT);
                    return true;
                }
		if(value == "breakpoint") {
                    set_assert_mode(ASSERT_BREAKPOINT);
                    return true;
		}
                Logger::err("Process")
                    << "Invalid value for property sys:abort: "
                    << value
                    << std::endl;
                return false;
            }
            return false;
        }

        /** ProcessEnvironment destructor */
        virtual ~ProcessEnvironment() {
        }
    };

    /************************************************************************/

#ifdef GEO_OPENMP

    /**
     * \brief OpenMP Thread Manager
     * \details
     * OMPThreadManager is an implementation of ThreadManager that uses OpenMP
     * for running concurrent threads and control critical sections.
     */
    class GEOGRAM_API OMPThreadManager : public ThreadManager {
    public:
        /**
         * \brief Creates and initializes the OpenMP ThreadManager
         */
        OMPThreadManager() {
            omp_init_lock(&lock_);
        }

        /** \copydoc GEO::ThreadManager::maximum_concurrent_threads() */
        virtual index_t maximum_concurrent_threads() {
            return Process::number_of_cores();
        }

        /** \copydoc GEO::ThreadManager::enter_critical_section() */
        virtual void enter_critical_section() {
            omp_set_lock(&lock_);
        }

        /** \copydoc GEO::ThreadManager::leave_critical_section() */
        virtual void leave_critical_section() {
            omp_unset_lock(&lock_);
        }

    protected:
        /** \brief OMPThreadManager destructor */
        virtual ~OMPThreadManager() {
            omp_destroy_lock(&lock_);
        }

        /** \copydoc GEO::ThreadManager::run_concurrent_threads() */
        virtual void run_concurrent_threads(
            ThreadGroup& threads, index_t max_threads
        ) {
            // TODO: take max_threads_ into account
            geo_argused(max_threads);

#pragma omp parallel for schedule(dynamic)
            for(index_t i = 0; i < threads.size(); i++) {
                set_thread_id(threads[i],i);
                set_current_thread(threads[i]);
                threads[i]->run();
            }
        }

    private:
        omp_lock_t lock_;
    };

#endif
}


namespace {
    /**
     * \brief The (thread-local) variable that stores a
     *  pointer to the current thread.
     * \details It cannot be a static member of class
     *  Thread, because Visual C++ does not accept
     *  to export thread local storage variables in 
     *  DLLs.
     */
    thread_local Thread* geo_current_thread_ = nullptr;
}

namespace GEO {

    void Thread::set_current(Thread* thread) {
        geo_current_thread_ = thread;
    }

    Thread* Thread::current() {
        return geo_current_thread_;
    }
    
    Thread::~Thread() {
    }

    /************************************************************************/

    ThreadManager::~ThreadManager() {
    }

    void ThreadManager::run_threads(ThreadGroup& threads) {
        index_t max_threads = maximum_concurrent_threads();
        if(Process::multithreading_enabled() && max_threads > 1) {
            run_concurrent_threads(threads, max_threads);
        } else {
            for(index_t i = 0; i < threads.size(); i++) {
                threads[i]->run();
            }
        }
    }

    /************************************************************************/

    MonoThreadingThreadManager::~MonoThreadingThreadManager() {
    }

    void MonoThreadingThreadManager::run_concurrent_threads(
        ThreadGroup& threads, index_t max_threads
    ) {
        geo_argused(threads);
        geo_argused(max_threads);
        geo_assert_not_reached;
    }

    index_t MonoThreadingThreadManager::maximum_concurrent_threads() {
        return 1;
    }

    void MonoThreadingThreadManager::enter_critical_section() {
    }

    void MonoThreadingThreadManager::leave_critical_section() {
    }

    /************************************************************************/

    namespace Process {

        // OS dependent functions implemented in process_unix.cpp and
        // process_win.cpp

        bool os_init_threads();
        void os_brute_force_kill();
        bool os_enable_FPE(bool flag);
        bool os_enable_cancel(bool flag);
        void os_install_signal_handlers();
        index_t os_number_of_cores();
        size_t os_used_memory();
        size_t os_max_used_memory();
        std::string os_executable_filename();
        
        void initialize(int flags) {

            Environment* env = Environment::instance();
            env->add_environment(new ProcessEnvironment);

            if(!os_init_threads()) {
#ifdef GEO_OPENMP
                Logger::out("Process")
                    << "Using OpenMP threads"
                    << std::endl;
                set_thread_manager(new OMPThreadManager);
#else
                Logger::out("Process")
                    << "Multithreading not supported, going monothread"
                    << std::endl;
                set_thread_manager(new MonoThreadingThreadManager);
#endif
            }

	    if(
		(::getenv("GEO_NO_SIGNAL_HANDLER") == nullptr) &&
		(flags & GEOGRAM_INSTALL_HANDLERS) != 0
	    ) {
		os_install_signal_handlers();
	    }
	    
            // Initialize Process default values
            enable_multithreading(multithreading_enabled_);
            set_max_threads(number_of_cores());
            enable_FPE(fpe_enabled_);
            enable_cancel(cancel_enabled_);

            start_time_ = SystemStopwatch::now();
        }

        void show_stats() {

            Logger::out("Process") << "Total elapsed time: " 
                                   << SystemStopwatch::now() - start_time_
                                   << "s" << std::endl;

            const size_t K=size_t(1024);
            const size_t M=K*K;
            const size_t G=K*M;
            
            size_t max_mem = Process::max_used_memory() ;
            size_t r = max_mem;
            
            size_t mem_G = r / G;
            r = r % G;
            size_t mem_M = r / M;
            r = r % M;
            size_t mem_K = r / K;
            r = r % K;
            
            std::string s;
            if(mem_G != 0) {
                s += String::to_string(mem_G)+"G ";
            }
            if(mem_M != 0) {
                s += String::to_string(mem_M)+"M ";
            }
            if(mem_K != 0) {
                s += String::to_string(mem_K)+"K ";
            }
            if(r != 0) {
                s += String::to_string(r);
            }

            Logger::out("Process") << "Maximum used memory: " 
                                   << max_mem << " (" << s << ")"
                                   << std::endl;
        }

        void terminate() {
            thread_manager_.reset();
        }

        void brute_force_kill() {
            os_brute_force_kill();
        }

        index_t number_of_cores() {
            static index_t result = 0;
            if(result == 0) {
#ifdef GEO_NO_THREAD_LOCAL
		// Deactivate multithreading if thread_local is
		// not supported (e.g. with old OS-X).
		result = 1;
#else		
                result = os_number_of_cores();
#endif		
            }
            return result;
        }

        size_t used_memory() {
            return os_used_memory();
        }

        size_t max_used_memory() {
            return os_max_used_memory();
        }

        std::string executable_filename() {
            return os_executable_filename();
        }
        
        void set_thread_manager(ThreadManager* thread_manager) {
            thread_manager_ = thread_manager;
        }

        void run_threads(ThreadGroup& threads) {
            running_threads_invocations_++;
            thread_manager_->run_threads(threads);
            running_threads_invocations_--;
        }

        void enter_critical_section() {
            thread_manager_->enter_critical_section();
        }

        void leave_critical_section() {
            thread_manager_->leave_critical_section();
        }

        bool is_running_threads() {
#ifdef GEO_OPENMP
            return (
		omp_in_parallel() ||
		(running_threads_invocations_ > 0)
	    );	    
#else	    
            return running_threads_invocations_ > 0;
#endif	    
        }

        bool multithreading_enabled() {
            return multithreading_enabled_;
        }

        void enable_multithreading(bool flag) {
            if(
                multithreading_initialized_ &&
                multithreading_enabled_ == flag
            ) {
                return;
            }
            multithreading_initialized_ = true;
            multithreading_enabled_ = flag;
            if(multithreading_enabled_) {
                Logger::out("Process")
                    << "Multithreading enabled" << std::endl
                    << "Available cores = " << number_of_cores()
                    << std::endl;
                // Logger::out("Process")
                //    << "Max. concurrent threads = "
                //    << maximum_concurrent_threads() << std::endl ;
                if(number_of_cores() == 1) {
                    Logger::warn("Process")
                        << "Processor is not a multicore"
			<< "(or multithread is not supported)"
                        << std::endl;
                }
                if(thread_manager_ == nullptr) {
                    Logger::warn("Process")
                        << "Missing multithreading manager"
                        << std::endl;
                }
            } else {
                Logger::out("Process")
                    << "Multithreading disabled" << std::endl;
            }
        }

        index_t max_threads() {
            return max_threads_initialized_
                   ? max_threads_
                   : number_of_cores();
        }

        void set_max_threads(index_t num_threads) {
            if(
                max_threads_initialized_ &&
                max_threads_ == num_threads
            ) {
                return;
            }
            max_threads_initialized_ = true;
            if(num_threads == 0) {
                num_threads = 1;
            } else if(num_threads > number_of_cores()) {
                Logger::warn("Process")
                    << "Cannot allocate " << num_threads 
                    << " for multithreading"
                    << std::endl;
                num_threads = number_of_cores();
            }
            max_threads_ = num_threads;
            Logger::out("Process")
                << "Max used threads = " << max_threads_
                << std::endl;
        }

        index_t maximum_concurrent_threads() {
            if(!multithreading_enabled_ || thread_manager_ == nullptr) {
                return 1;
            }
            return max_threads_;
            /*
               // commented out for now, since under Windows,
               // it seems that maximum_concurrent_threads() does not
               // report the number of hyperthreaded cores.
                        return
                            geo_min(
                                thread_manager_->maximum_concurrent_threads(),
                                max_threads_
                            ) ;
             */
        }

        bool FPE_enabled() {
            return fpe_enabled_;
        }

        void enable_FPE(bool flag) {
            if(fpe_initialized_ && fpe_enabled_ == flag) {
                return;
            }
            fpe_initialized_ = true;
            fpe_enabled_ = flag;
	    os_enable_FPE(flag);
        }

        bool cancel_enabled() {
            return cancel_enabled_;
        }

        void enable_cancel(bool flag) {
            if(cancel_initialized_ && cancel_enabled_ == flag) {
                return;
            }
            cancel_initialized_ = true;
            cancel_enabled_ = flag;

            if(os_enable_cancel(flag)) {
                Logger::out("Process")
                    << (flag ? "Cancel mode enabled" : "Cancel mode disabled")
                    << std::endl;
            } else {
                Logger::warn("Process")
                    << "Cancel mode not implemented" << std::endl;
            }
        }
    }
}


namespace {
    using namespace GEO;

    /**
     * \brief Used by the implementation of GEO::parallel()
     * \see GEO::parallel()
     */
    class ParallelThread : public Thread {
    public:
	/**
	 * \brief ParallelThread constructor.
	 * \param[in] func a void function with no parameter.
	 */
	ParallelThread(
	    std::function<void(void)> func
	) : func_(func) {
	}

	/**
	 * \copydoc Thread::run()
	 */
        void run() override {
	    func_();
        }
    private:
	std::function<void()> func_;
    };


    /**
     * \brief Used by the implementation of GEO::parallel_for()
     * \see GEO::parallel_for()
     */
    class ParallelForThread : public Thread {
    public:

	/**
	 * \param[in] func a void function that takes an index_t
	 * \param[in] from the first iteration index
	 * \param[in] to one position past the last interation index
	 * \param[in] step iteration step
	 */
	ParallelForThread(
	    std::function<void(index_t)> func,
	    index_t from, index_t to, index_t step=1
	) : func_(func), from_(from), to_(to), step_(step) {
	}

	/**
	 * \copydoc Thread::run()
	 */
        void run() override {
            for(index_t i = from_; i < to_; i += step_) {
                func_(i);
            }
        }
    private:
	std::function<void(index_t)> func_;
	index_t from_;
	index_t to_;
	index_t step_;
    };

    /**
     * \brief Used by the implementation of GEO::parallel_for_slice()
     * \see GEO::parallel_for_slice()
     */
    class ParallelForSliceThread : public Thread {
    public:

	/**
	 * \param[in] func a void function that takes two index_t arguments
	 * \param[in] from the first iteration index
	 * \param[in] to one position past the last interation index
	 */
	ParallelForSliceThread(
	    std::function<void(index_t,index_t)> func,
	    index_t from, index_t to
	) : func_(func), from_(from), to_(to) {
	}

	/**
	 * \copydoc Thread::run()
	 */
        void run() override {
	    func_(from_, to_);
        }
    private:
	std::function<void(index_t,index_t)> func_;
	index_t from_;
	index_t to_;
    };
    
}

namespace GEO {

    void parallel_for(
        index_t from, index_t to, std::function<void(index_t)> func,
        index_t threads_per_core, bool interleaved 
    ) {
#ifdef GEO_OS_WINDOWS
        // TODO: This is a limitation of WindowsThreadManager, to be fixed.
        threads_per_core = 1;
#endif

        index_t nb_threads = std::min(
            to - from,
            Process::maximum_concurrent_threads() * threads_per_core
        );

	nb_threads = std::max(index_t(1), nb_threads);
	
        index_t batch_size = (to - from) / nb_threads;
        if(Process::is_running_threads() || nb_threads == 1) {
            for(index_t i = from; i < to; i++) {
                func(i);
            }
        } else {
            ThreadGroup threads;
            if(interleaved) {
                for(index_t i = 0; i < nb_threads; i++) {
                    threads.push_back(
                        new ParallelForThread(
                            func, from + i, to, nb_threads
                        )
                    );
                }
            } else {
                index_t cur = from;
                for(index_t i = 0; i < nb_threads; i++) {
                    if(i == nb_threads - 1) {
                        threads.push_back(
                            new ParallelForThread(
                                func, cur, to
                            )
                        );
                    } else {
                        threads.push_back(
                            new ParallelForThread(
                                func, cur, cur + batch_size
                            )
                        );
                    }
                    cur += batch_size;
                }
            }
            Process::run_threads(threads);
        }
    }


    void parallel_for_slice(
	index_t from, index_t to, std::function<void(index_t, index_t)> func,
        index_t threads_per_core 
    ) {
#ifdef GEO_OS_WINDOWS
        // TODO: This is a limitation of WindowsThreadManager, to be fixed.
        threads_per_core = 1;
#endif

        index_t nb_threads = std::min(
            to - from,
            Process::maximum_concurrent_threads() * threads_per_core
        );

	nb_threads = std::max(index_t(1), nb_threads);
	
        index_t batch_size = (to - from) / nb_threads;
        if(Process::is_running_threads() || nb_threads == 1) {
	    func(from, to);
        } else {
            ThreadGroup threads;
	    index_t cur = from;
	    for(index_t i = 0; i < nb_threads; i++) {
		if(i == nb_threads - 1) {
		    threads.push_back(
			new ParallelForSliceThread(
			    func, cur, to
			  )
			);
		} else {
		    threads.push_back(
			new ParallelForSliceThread(
			    func, cur, cur + batch_size
                           )
                        );
		}
		cur += batch_size;
	    }
            Process::run_threads(threads);
        }
    }

    void parallel(
	std::function<void()> f1,
	std::function<void()> f2
    ) {
        if(Process::is_running_threads()) {
	    f1();
	    f2();
        } else {
            ThreadGroup threads;
	    threads.push_back(new ParallelThread(f1));
	    threads.push_back(new ParallelThread(f2));
            Process::run_threads(threads);
        }
    }
    

    void parallel(
	std::function<void()> f1,
	std::function<void()> f2,
	std::function<void()> f3,
	std::function<void()> f4
    ) {
        if(Process::is_running_threads()) {
	    f1();
	    f2();
	    f3();
	    f4();
        } else {
            ThreadGroup threads;
	    threads.push_back(new ParallelThread(f1));
	    threads.push_back(new ParallelThread(f2));
	    threads.push_back(new ParallelThread(f3));
	    threads.push_back(new ParallelThread(f4));
            Process::run_threads(threads);
        }
    }

    
    void parallel(
	std::function<void()> f1,
	std::function<void()> f2,
	std::function<void()> f3,
	std::function<void()> f4,
	std::function<void()> f5,
	std::function<void()> f6,
	std::function<void()> f7,
	std::function<void()> f8	 
    ) {
        if(Process::is_running_threads()) {
	    f1();
	    f2();
	    f3();
	    f4();
	    f5();
	    f6();
	    f7();
	    f8();
        } else {
            ThreadGroup threads;
	    threads.push_back(new ParallelThread(f1));
	    threads.push_back(new ParallelThread(f2));
	    threads.push_back(new ParallelThread(f3));
	    threads.push_back(new ParallelThread(f4));
	    threads.push_back(new ParallelThread(f5));
	    threads.push_back(new ParallelThread(f6));
	    threads.push_back(new ParallelThread(f7));
	    threads.push_back(new ParallelThread(f8));	    
            Process::run_threads(threads);
        }
    }

    namespace Process {
	void sleep(index_t microseconds) {
	    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));	    
	}
    }
}

