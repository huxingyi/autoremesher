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
#include <geogram/basic/logger.h>
#include <geogram/basic/process.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/stopwatch.h>

namespace {

    using namespace GEO;

    /**
     * \brief Locking test
     * \details
     * The test consists in concurrent threads randomly accessing a critical
     * section represented by an array of integer values. The critical section
     * can be protected by either a global lock, or per-element using a
     * SpinLockArray.
     */
    class LockTest {
    public:
        /**
         * \brief Creates a new locking test
         * \param[in] size number of elements in the critical section.
         * \param[in] single_lock if true, the critical section is protected
         * by a single global lock. Otherwise, the critical section is
         * protected per-element using a SpinLockArray.
         * \param[in] nb_times number of access operations performed by each
         * thread.
         */
        LockTest(
            index_t size, bool single_lock, index_t nb_times
        ) :
            single_lock_(single_lock),
            nb_times_(nb_times) {
            global_lock_ = GEOGRAM_SPINLOCK_INIT;
            if(!single_lock_) {
                locks_.resize(size);
            }
            data_.assign(size, -1);
        }

        /**
         * \brief Accesses the critical section without locking
         * \details This function is executed by each thread: it does nb_times
         * random access in the critical section \b without locking it.
         * \param[in] pid The id of the thread
         */
        void test_locks(index_t pid) {
            Process::enter_critical_section();
            std::cerr << "Starting thread " << Thread::current()->id() 
                      << std::endl;
            Process::leave_critical_section();
            index_t j = 0;
            for(index_t i = 0; i < nb_times_; ++i) {
                j = (j + 7) % index_t(data_.size());
                lock(j);
                geo_assert(data_[j] == -1);
                data_[j] = signed_index_t(pid);
                fast_pause();
                geo_assert(data_[j] == signed_index_t(pid));
                data_[j] = -1;
                unlock(j);
            }
            Process::enter_critical_section();
            std::cerr << "End of thread " << Thread::current()->id() 
                      << std::endl;
            Process::leave_critical_section();
        }

    protected:
        /**
         * \brief Locks an critical section element
         * \details Locks the element at index \p i in the critical section.
         * In single_lock mode, this locks the whole critical section is
         * locked, otherwise the element is locked individually using the
         * spinlock at index \p i in a SpinLockArray .
         * \param[in] i index of the element in the critical section
         */
        void lock(index_t i) {
            if(single_lock_) {
                Process::acquire_spinlock(global_lock_);
            } else {
                locks_.acquire_spinlock(i);
            }
        }

        /**
         * \brief Unlocks an critical section element
         * \details Unlocks the element at index \p i in the critical section.
         * In single_lock mode, this unlocks the whole critical section is
         * locked, otherwise the element is unlocked individually using the
         * spinlock at index \p i in a SpinLockArray .
         * \param[in] i index of the element in the critical section
         */
        void unlock(index_t i) {
            if(single_lock_) {
                Process::release_spinlock(global_lock_);
            } else {
                locks_.release_spinlock(i);
            }
        }

        /**
         * \brief Does a (very fast pause)
         * \details This pause is called by test_locks() to increase the time
         * spent in the critical section.
         */
        static void fast_pause() {
            for(index_t i = 0; i < 50000; ++i) {
            }
        }

    private:
        bool single_lock_;
        Process::spinlock global_lock_;
        Process::SpinLockArray locks_;
        std::vector<signed_index_t> data_;
        index_t nb_times_;
    };
}

int main(int argc, char** argv) {
    using namespace GEO;

    GEO::initialize();

    try {
        Stopwatch W("Total time");
        CmdLine::import_arg_group("standard");
        CmdLine::declare_arg("array_size", 2, "number of cells in array");
        CmdLine::declare_arg("global_lock", false, "use a global lock");
        CmdLine::declare_arg("nb_times", 1000000, "number of ops");
        CmdLine::declare_arg("locks", true, "use locks in test");

        if(!CmdLine::parse(argc, argv)) {
            return 1;
        }

        LockTest lock_test(
            CmdLine::get_arg_uint("array_size"),
            CmdLine::get_arg_bool("global_lock"),
            CmdLine::get_arg_uint("nb_times")
        );

        if(CmdLine::get_arg_bool("locks")) {
            parallel_for(
                0, Process::max_threads(),
		std::bind(&LockTest::test_locks, &lock_test, std::placeholders::_1)
            );
        } else {
            parallel_for(
                0, Process::max_threads(),
		std::bind(&LockTest::test_locks, &lock_test, std::placeholders::_1)		
            );
        }
    }
    catch(const std::exception& e) {
        std::cerr << "Received an exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

