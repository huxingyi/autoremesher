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

#ifndef GEOGRAM_BASIC_PROGRESS
#define GEOGRAM_BASIC_PROGRESS

#include <geogram/basic/common.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/counted.h>
#include <geogram/basic/smart_pointer.h>

/**
 * \file geogram/basic/progress.h
 * \brief Functions and classes for displaying progress bars
 */

namespace GEO {

    /**
     * \brief Task progress listener
     * \details ProgressClient is an abstract interface for listening to the
     * progress of tasks tracked by the ProgressTask. The main purpose of a
     * ProgressClient is to make the progress visible to the end user (to the
     * console, to a progress bar).
     *
     * To implement a specific progress client, you must create a derived
     * class of ProgressClient and implement the 3 functions:
     * - begin()
     * - progress()
     * - end()
     *
     * Then the client must be registered to the Progress system with
     * Progress::set_client(). The client can access the status of the current
     * task at any with Progress::current_task().
     * time
     *
     * \see Progress
     * \see ProgressTask
     */
    class GEOGRAM_API ProgressClient : public Counted {
    public:
        /**
         * \brief Starts listening progress
         * \details This function is called by the ProgressTask to start
         * tracking the execution of a new task. Clients are free to do
         * whatever is appropriate (show a progress bar, a progress dialog,
         * ...)
         */
        virtual void begin() = 0;

        /**
         * \brief Tracks progress
         * \details This function is called by the ProgressTask repeatedly
         * during the execution of the task. Clients are free to do whatever
         * is appropriate (update a progress bar, log the current values,
         * ...).
         * \param[in] step the current progress step
         * \param[in] percent the percentage of completion of the task
         */
        virtual void progress(index_t step, index_t percent) = 0;

        /**
         * \brief Stops listening progress
         * \details This function is called by the ProgressTask to stop
         * tracking the execution of the task. Clients are free to do whatever
         * is appropriate (hide a progress bar, a progress dialog, log the
         * elapsed time, ...). If the task was not terminated normally (i.e.,
         * canceled() then parameter \p canceled is set to \c true.
         * \param[in] canceled set to \c true if the task was canceled, \c
         * false otherwise.
         */
        virtual void end(bool canceled) = 0;

    protected:
        /** ProgressClient destructor */
        virtual ~ProgressClient();
    };

    /** Smart pointer that contains a ProgressClient object */
    typedef SmartPointer<ProgressClient> ProgressClient_var;

    /************************************************************************/

    /**
     * \brief Exception thrown when a task is canceled
     * \see Progress::cancel()
     */
    struct GEOGRAM_API TaskCanceled : std::exception {
        /**
         * \brief Gets the string identifying the exception
         */
        virtual const char* what() const GEO_NOEXCEPT;
    };

    /************************************************************************/

    class ProgressTask;

    /**
     * \brief Framework for tracking the progression of a task
     */
    namespace Progress {
        /**
         * \brief Initializes the Progress framework
         * \details This function must be called once at program startup to
         * create the unique instance of the Progress class and
         * set a default LoggerClient that logs progress to the console.
         * \note This function is called by the Vorpaline initialization
         * function.
         * \see GEO::initialize()
         * \see CmdLine
         */
        void GEOGRAM_API initialize();

        /**
         * \brief Cleans up the Progress framework
         * \details This function must be called when the program exits to
         * cleanup the framework. It is called by the Vorpaline cleanup
         * function \c GEO::terminate().
         */
        void GEOGRAM_API terminate();

        /**
         * \brief Sets the Progress client
         * \details Sets the Progress client to \p client. The Progress
         * instance takes ownership of the client so there's no need to delete
         * it when the Progress terminates.
         * \param[in] client a pointer to a ProgressClient
         */
        void GEOGRAM_API set_client(ProgressClient* client);

        /**
         * \brief Gets the current task
         * \details The current task is the last ProgressTask being created,
         * which corresponds to the top-most ProgressTask in the execution
         * call stack.
         * \return a pointer the current ProgressTask if any or a null pointer
         * if there's no current task.
         */
        GEOGRAM_API const ProgressTask* current_task();

        /**
         * \brief Cancels the current task
         * \details This sets a cancellation flag to \c true.
         * This makes the next call to
         * ProgressTask::progress() throw an exception TaskCanceled. It is the
         * responsibility of the client code to catch this exception and do
         * appropriate cleanup. The cancellation flag can be tested with
         * is_canceled() before ProgressTask::progress() is being called by
         * the current task.
         * \see is_canceled()
         * \see TaskCanceled
         */
        void GEOGRAM_API cancel();

        /**
         * \brief Checks if the current task is canceled
         * \details This returns \c true if a request was made to cancel the
         * task (e.g., from the user interface).
         * \retval true if the task was canceled with cancel()
         * \retval false otherwise
         */
        bool GEOGRAM_API is_canceled();

        /**
         * \brief Clears the cancellation flag
         */
        void GEOGRAM_API clear_canceled();
    }

    /************************************************************************/

    /**
     * \brief Tracks the progress of a task.
     * \details
     * The progress of a task can be represented by a number of steps to
     * execute. As the task progresses, the client code informs the
     * ProgressTask of the execution step by calling function next() or
     * progress(), which notify the LoggerClient%s of the progress. Finally,
     * when the current logger goes out of scope (is destroyed), the logger
     * notifies the LoggerClient%s that the task is terminated.
     *
     * When a task is canceled by Progress::cancel() the next call to
     * progress() throws an exception TaskCanceled. It is the
     * responsibility of the client code to catch this exception and do
     * appropriate cleanup. The recommended usage of ProgressTask is
     * illustrated below:
     * \code
     * try {
     *     ProgressTask task("something to do", 100);
     *     for(size_t i = 0; i < 100; ++i) {
     *          do_something();
     *          task.progress(i);
     *     }
     * }
     * catch(const TaskCanceled&) {
     *     // Do early cleanup
     * }
     * \endcode
     *
     * ProgressTask can be sub-classed by client code, typically to route
     * progress display to a progress bar.
     */
    class GEOGRAM_API ProgressTask {
    public:
        /**
         * \brief Creates a logger for a task
         * \details This creates a ProgressTask object for task \p
         * task_name with a number of steps given by \p max_steps. The
         * registered LoggerClient is notified to start listening to the
         * progress of the task.
         * \param[in] task_name the name of the task
         * \param[in] max_steps the number of steps of the task
         * \param[in] quiet set to \c true to make the progress silent
         * \see LoggerClient::begin()
         */
        ProgressTask(
            const std::string& task_name, index_t max_steps,
            bool quiet 
        );

        /**
         * \brief Creates a logger for a task
         * \details This creates a ProgressTask object for task \p
         * task_name with a number of steps given by \p max_steps. The
         * registered LoggerClient is notified to start listening to the
         * progress of the task.
         * \param[in] task_name the name of the task
         * \param[in] max_steps the number of steps of the task
         */
        ProgressTask(
            const std::string& task_name = "", index_t max_steps = 100
        );

        /**
         * \brief Destroys a ProgressTask
         * \details This notifies the registered LoggerClient%s that the
         * task is terminated.
         * \see LoggerClient::end()
         */
        virtual ~ProgressTask();

        /**
         * \brief Sets the current execution step
         * \details This sets the current step value to \p step. The new
         * value must not be greater than the configured number of steps in
         * the ProgressTask constructor. This updates the percentage of
         * completion of the task and notifies the registered
         * LoggerClient%s that the execution step has changed.
         * \param[in] step the new step value
         * \see update()
         * \throw TaskCanceled
         */
        virtual void progress(index_t step);

        /**
         * \brief Goes to the next step
         * \details This increments the current step value by 1. This updates
         * the percentage of completion of the task and notifies
         * the registered LoggerClient%s that the execution step has changed.
         * \see update()
         */
        virtual void next();

        /**
         * \brief Checks if the task is canceled
         * \details This function must be called as often as possible during
         * the execution of the current task to stop the current task
         * in case a request was made to cancel it (e.g., from the user
         * interface).
         * \retval true if the task was canceled
         * \retval false otherwise
         */
        bool is_canceled() const;

        /**
         * \brief Resets the execution step
         * \details Resets progress at the beginning. This updates the
         * percentage of completion of the task and notifies the
         * registered LoggerClient%s that the execution step has changed. This
         * is equivalent to call \c progress(0).
         * \see progress()
         */
        void reset();

        /**
         * \brief Resets the execution step
         * \details This changes the maximum number of steps to \p max_steps and
         * resets progress at the beginning. This updates the percentage of
         * completion of the task and notifies the registered
         * LoggerClient%s that the execution step has changed.
         * \param[in] max_steps the new number of steps of the task.
         * \see LoggerClient::progress()
         */
        void reset(index_t max_steps);

        /**
         * \brief Gets the name of the task
         */
        const std::string& task_name() const {
            return task_name_;
        }

        /**
         * \brief Gets the start time of the task
         */
        double start_time() const {
            return start_time_;
        }

        /**
         * \brief Gets the number of steps of the task
         */
        index_t max_steps() const {
            return max_steps_;
        }

        /**
         * \brief Gets the current step of the task
         */
        index_t step() const {
            return step_;
        }

        /**
         * \brief Gets the percentage of completion of the task
         */
        index_t percent() const {
            return percent_;
        }

    protected:
        /**
         * \brief Updates progress values
         * \details Updates the percentage of completion of the task and
         * notifies the registered LoggerClient%s that the execution step has
         * changed.
         * \see LoggerClient::progress()
         */
        virtual void update();

    private:
        std::string task_name_;
        double start_time_;
        bool quiet_;
        index_t max_steps_;
        index_t step_;
        index_t percent_;
    };
}

#endif

