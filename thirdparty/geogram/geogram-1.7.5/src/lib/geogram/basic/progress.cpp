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

#include <geogram/basic/progress.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/stopwatch.h>
#include <stack>

namespace {

    using namespace GEO;

    ProgressClient_var progress_client_;
    std::stack<const ProgressTask*> progress_tasks_;
    bool task_canceled_ = false;

    /**
     * \brief Notifies clients that a task has started.
     * \details This function is called when a new ProgressTask is
     * constructed. It pushes the given \p task to the task stack and notifies
     * the registered LoggerClient to start listening to the progress of the
     * task.
     * \param[in] task the task being started
     * \see ProgressTask::ProgressTask()
     * \see LoggerClient::begin()
     */
    void begin_task(const ProgressTask* task) {
        task_canceled_ = false;
        progress_tasks_.push(task);

        if(progress_client_) {
            progress_client_->begin();
        }
    }

    /**
     * \brief Notifies clients that a task has restarted.
     * \details This function is called when a new ProgressTask is reset.
     * In some contexts, the same ProgressTask instance is reused several
     * times, and the progress is restarted by calling ProgressTask::reset().
     * We must consider this situation as if a new ProgressTask had been
     * created.
     * \note It does \b not push the given \p task to the task stack and does
     * \b not notify the registered LoggerClient to start listening to the
     * progress of the task.
     * \param[in] task the task being restarted
     * \see ProgressTask::reset()
     */
    void reset_task(const ProgressTask* task) {
        geo_argused(task);
        task_canceled_ = false;
    }

    /**
     * \brief Notifies clients that a task progresses.
     * \details This function is called by ProgressTask::progress() to
     * notify registered LoggerClient%s that the execution step has
     * changed.
     * \param[in] step the current progress step
     * \param[in] percent the percentage of completion of the task
     * \see LoggerClient::progress()
     */
    void task_progress(index_t step, index_t percent) {
        if(task_canceled_) {
            throw TaskCanceled();
        }

        if(progress_client_) {
            progress_client_->progress(step, percent);
        }
    }

    /**
     * \brief Notifies clients that a task has ended.
     * \details This function is called by the ProgressTask destructor. It
     * restores the previous task from the task stack and notifies registered
     * LoggerClient%s that the current task has ended.
     * \param[in] task the task being terminated
     * \note The parameter \p task is not strictly necessary here, but we want
     * to make sure that the ProgressTask being destroyed is really the one at
     * the top of the stack.
     * \see ProgressTask::~ProgressTask()
     * \see LoggerClient::end()
     */
    void end_task(const ProgressTask* task) {
        geo_assert(!progress_tasks_.empty());
        geo_assert(progress_tasks_.top() == task);

        if(progress_client_) {
            progress_client_->end(task_canceled_);
        }

        progress_tasks_.pop();
        if(progress_tasks_.empty()) {
            task_canceled_ = false;
        }
    }

    /**
     * \brief Logs task progress to the console
     * \details TerminalProgressClient is an implementation of LoggerClient
     * that logs progress to the console using console progress functions.
     * \see CmdLine
     */
    class TerminalProgressClient : public ProgressClient {
    public:
        /** \copydoc GEO::ProgressClient::begin() */
	void begin() override {
            const ProgressTask* task = Progress::current_task();
            CmdLine::ui_progress(task->task_name(), 0, 0);
        }

        /** \copydoc GEO::ProgressClient::progress(index_t,index_t) */
	void progress(index_t step, index_t percent) override {
            const ProgressTask* task = Progress::current_task();
            CmdLine::ui_progress(task->task_name(), step, percent);
        }

        /** \copydoc GEO::ProgressClient::end(bool) */
	void end(bool canceled) override {
            const ProgressTask* task = Progress::current_task();
            double elapsed = SystemStopwatch::now() - task->start_time();
            if(canceled) {
                CmdLine::ui_progress_canceled(
                    task->task_name(), elapsed, task->percent()
                );
            } else {
                CmdLine::ui_progress_time(task->task_name(), elapsed);
            }
        }

    protected:
        /** \brief TerminalProgressClient destructor */
	~TerminalProgressClient() override {
        }
    };
}

/****************************************************************************/

namespace GEO {

    const char* TaskCanceled::what() const GEO_NOEXCEPT {
        return "Task canceled";
    }

    /************************************************************************/

    namespace Progress {

        void initialize() {
            set_client(new TerminalProgressClient());
        }

        void terminate() {
            set_client(nullptr);
        }

        void set_client(ProgressClient* client) {
            progress_client_ = client;
        }

        const ProgressTask* current_task() {
            return progress_tasks_.empty() ? nullptr : progress_tasks_.top();
        }

        void cancel() {
            if(!progress_tasks_.empty()) {
                task_canceled_ = true;
            }
        }

        bool is_canceled() {
            return task_canceled_;
        }

        void clear_canceled() {
            task_canceled_ = false;
        }
    }

    /************************************************************************/

    ProgressClient::~ProgressClient() {
    }

    /************************************************************************/

    ProgressTask::ProgressTask(
        const std::string& task_name, index_t max_steps, bool quiet
    ) :
        task_name_(task_name),
        start_time_(SystemStopwatch::now()),
        quiet_(quiet),
        max_steps_(std::max(index_t(1), max_steps)),
        step_(0),
        percent_(0)
    {
        if(!quiet_) {
            begin_task(this);
        }
    }

    ProgressTask::ProgressTask(
        const std::string& task_name, index_t max_steps
    ) :
        task_name_(task_name),
        start_time_(SystemStopwatch::now()),
        quiet_(Logger::instance()->is_quiet()),
        max_steps_(std::max(index_t(1), max_steps)),
        step_(0),
        percent_(0)
    {
        if(!quiet_) {
            begin_task(this);
        }
    }

    
    ProgressTask::~ProgressTask() {
        if(!quiet_) {
            end_task(this);
        }
    }

    void ProgressTask::reset() {
        start_time_ = SystemStopwatch::now();
        reset_task(this);
        progress(0);
    }

    void ProgressTask::reset(index_t max_steps) {
        max_steps_ = std::max(index_t(1), max_steps);
        reset();
    }

    void ProgressTask::next() {
        step_++;
	step_ = std::min(step_, max_steps_);
	update();
    }

    void ProgressTask::progress(index_t step) {
        if(step_ != step) {
            step_ = step;
	    step_ = std::min(step_, max_steps_);	    
	    update();
        }
    }

    bool ProgressTask::is_canceled() const {
        return task_canceled_;
    }

    void ProgressTask::update() {
	index_t new_percent =
	    std::min(index_t(100), index_t(step_ * 100 / max_steps_));
	if(new_percent != percent_) {
	    percent_ = new_percent;
	    if(!quiet_) {
		task_progress(step_, percent_);
	    }
	}
    }
}

