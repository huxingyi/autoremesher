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

#include <geogram/basic/logger.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/argused.h>
#include <geogram/basic/file_system.h>

#include <stdlib.h>
#include <stdarg.h>

/*
   Disables the warning caused by passing 'this' as an argument while
   construction is not finished (in LoggerStream ctor).
   As LoggerStreamBuf only stores the pointer for later use, so we can
   ignore the fact that 'this' is not completely formed yet.
 */
#ifdef GEO_OS_WINDOWS
#pragma warning(disable:4355)
#endif

namespace GEO {

    /************************************************************************/

    int LoggerStreamBuf::sync() {
        std::string str(this->str());
        loggerStream_->notify(str);
        this->str("");
        return 0;
    }

    /************************************************************************/

    LoggerStream::LoggerStream(Logger* logger) :
        std::ostream(new LoggerStreamBuf(this)),
        logger_(logger) {
    }

    LoggerStream::~LoggerStream() {
        std::streambuf* buf = rdbuf();
        delete buf;
    }

    void LoggerStream::notify(const std::string& str) {
	logger_->notify(this, str);
    }

    /************************************************************************/

    LoggerClient::~LoggerClient() {
    }

    /************************************************************************/

    ConsoleLogger::ConsoleLogger() {
    }

    ConsoleLogger::~ConsoleLogger() {
    }

    void ConsoleLogger::div(const std::string& title) {
        CmdLine::ui_separator(title);
    }

    void ConsoleLogger::out(const std::string& str) {
        CmdLine::ui_message(str);
    }

    void ConsoleLogger::warn(const std::string& str) {
        CmdLine::ui_message(str);
    }

    void ConsoleLogger::err(const std::string& str) {
        CmdLine::ui_message(str);
    }

    void ConsoleLogger::status(const std::string& str) {
        geo_argused(str);
    }

    /************************************************************************/

    FileLogger::FileLogger() :
        log_file_(nullptr) {
    }

    FileLogger::FileLogger(const std::string& file_name) :
        log_file_(nullptr)
    {
        set_file_name(file_name);
    }

    FileLogger::~FileLogger() {
        delete log_file_;
        log_file_ = nullptr;
    }

    void FileLogger::set_file_name(const std::string& file_name) {
        log_file_name_ = file_name;
        if(log_file_ != nullptr) {
            delete log_file_;
            log_file_ = nullptr;
        }
        if(log_file_name_.length() != 0) {
            log_file_ = new std::ofstream(log_file_name_.c_str());
        }
    }

    void FileLogger::div(const std::string& title) {
        if(log_file_ != nullptr) {
            *log_file_
                << "\n====[" << title << "]====\n"
                << std::flush;
        }
    }

    void FileLogger::out(const std::string& str) {
        if(log_file_ != nullptr) {
            *log_file_ << str << std::flush;
        }
    }

    void FileLogger::warn(const std::string& str) {
        if(log_file_ != nullptr) {
            *log_file_ << str << std::flush;
        }
    }

    void FileLogger::err(const std::string& str) {
        if(log_file_ != nullptr) {
            *log_file_ << str << std::flush;
        }
    }

    void FileLogger::status(const std::string& str) {
        geo_argused(str);
    }

    /************************************************************************/

    SmartPointer<Logger> Logger::instance_;

    void Logger::initialize() {
        instance_ = new Logger();
        Environment::instance()->add_environment(instance_);
    }

    void Logger::terminate() {
        instance_.reset();
    }

    bool Logger::is_initialized() {
        return (instance_ != nullptr);
    }
    
    bool Logger::set_local_value(
        const std::string& name, const std::string& value
    ) {

        if(name == "log:quiet") {
            set_quiet(String::to_bool(value));
            return true;
        }

        if(name == "log:minimal") {
            set_minimal(String::to_bool(value));
            return true;
        }
        
        if(name == "log:pretty") {
            set_pretty(String::to_bool(value));
            return true;
        }

        if(name == "log:file_name") {
            log_file_name_ = value;
            if(!log_file_name_.empty()) {
                register_client(new FileLogger(log_file_name_));
            }
            return true;
        }

        if(name == "log:features") {
            std::vector<std::string> features;
            String::split_string(value, ';', features);
            log_features_.clear();
            if(features.size() == 1 && features[0] == "*") {
                log_everything_ = true;
            } else {
                log_everything_ = false;
                for(size_t i = 0; i < features.size(); i++) {
                    log_features_.insert(features[i]);
                }
            }
            notify_observers(name);
            return true;
        }

        if(name == "log:features_exclude") {
            std::vector<std::string> features;
            String::split_string(value, ';', features);
            log_features_exclude_.clear();
            for(size_t i = 0; i < features.size(); i++) {
                log_features_exclude_.insert(features[i]);
            }
            notify_observers(name);
            return true;
        }

        return false;
    }

    bool Logger::get_local_value(
        const std::string& name, std::string& value
    ) const {

        if(name == "log:quiet") {
            value = String::to_string(is_quiet());
            return true;
        }

        if(name == "log:minimal") {
            value = String::to_string(is_minimal());
            return true;
        }
        
        if(name == "log:pretty") {
            value = String::to_string(is_pretty());
            return true;
        }

        if(name == "log:file_name") {
            value = log_file_name_;
            return true;
        }

        if(name == "log:features") {
            if(log_everything_) {
                value = "*";
            } else {
                value = "";
                for(auto& it : log_features_) {
                    if(value.length() != 0) {
                        value += ';';
                    }
                    value += it;
                }
            }
            return true;
        }

        if(name == "log:features_exclude") {
            value = "";
            for(auto& it : log_features_exclude_) {
                if(value.length() != 0) {
                    value += ';';
                }
                value += it;
            }
            return true;
        }

        return false;
    }

    void Logger::register_client(LoggerClient* c) {
        clients_.insert(c);
    }

    void Logger::unregister_client(LoggerClient* c) {
        geo_debug_assert(clients_.find(c) != clients_.end());
	clients_.erase(c);
    }

    void Logger::unregister_all_clients() {
        clients_.clear();
    }

    bool Logger::is_client(LoggerClient* c) const {
        return clients_.find(c) != clients_.end();
    }

    void Logger::set_quiet(bool flag) {
        quiet_ = flag;
    }

    void Logger::set_minimal(bool flag) {
        minimal_ = flag;
    }
    
    void Logger::set_pretty(bool flag) {
        pretty_ = flag;
    }


    Logger::Logger() :
        out_(this),
        warn_(this),
        err_(this),
        status_(this),
        log_everything_(true),
        current_feature_changed_(false),
        quiet_(true),
        pretty_(true),
        minimal_(false),
	notifying_error_(false)
    {
        // Add a default client printing stuff to std::cout
        register_client(new ConsoleLogger());
#ifdef GEO_DEBUG
        quiet_ = false;
#endif        
    }

    Logger::~Logger() {
    }

    Logger* Logger::instance() {
        // Do not use geo_assert here:
	//  if the instance is nullptr, geo_assert will
        // call the Logger to print the assertion failure, thus ending in a
        // infinite loop.
        if(instance_ == nullptr) {
            std::cerr
                << "CRITICAL: Accessing uninitialized Logger instance"
                << std::endl;
            geo_abort();
        }
        return instance_;
    }

    std::ostream& Logger::div(const std::string& title) {
	std::ostream& result = 
   	    (is_initialized() && !Process::is_running_threads()) ?
            instance()->div_stream(title) :
            (std::cerr << "=====" << title << std::endl);
	return result;
    }

    std::ostream& Logger::out(const std::string& feature) {
	std::ostream& result =
	    (is_initialized() && !Process::is_running_threads()) ?
            instance()->out_stream(feature) :
            (std::cerr << "    [" << feature << "] ");
	return result;
    }

    std::ostream& Logger::err(const std::string& feature) {
	std::ostream& result = 
	    (is_initialized() && !Process::is_running_threads()) ?	    
            instance()->err_stream(feature) :
            (std::cerr << "(E)-[" << feature << "] ");
	return result;
    }

    std::ostream& Logger::warn(const std::string& feature) {
	std::ostream& result = 
	    (is_initialized() && !Process::is_running_threads()) ?	    	    
            instance()->warn_stream(feature) :
            (std::cerr << "(W)-[" << feature << "] ");
	return result;
    }

    std::ostream& Logger::status() {
	std::ostream& result =	
	    (is_initialized() && !Process::is_running_threads()) ?	    	    	
            instance()->status_stream() :
            (std::cerr << "[status] ");
	return result;
    }

    std::ostream& Logger::div_stream(const std::string& title) {
        if(!quiet_) {
            current_feature_changed_ = true;
            current_feature_.clear();
            for(auto it : clients_) {
                it->div(title);
            }
        }
        return out_;
    }

    std::ostream& Logger::out_stream(const std::string& feature) {
        if(!quiet_ && !minimal_ && current_feature_ != feature) {
            current_feature_changed_ = true;
            current_feature_ = feature;
        }
        return out_;
    }

    std::ostream& Logger::err_stream(const std::string& feature) {
        if(!quiet_ && current_feature_ != feature) {
            current_feature_changed_ = true;
            current_feature_ = feature;
        }
        return err_;
    }

    std::ostream& Logger::warn_stream(const std::string& feature) {
        if(!quiet_ && current_feature_ != feature) {
            current_feature_changed_ = true;
            current_feature_ = feature;
        }
        return warn_;
    }

    std::ostream& Logger::status_stream() {
        return status_;
    }

    void Logger::notify_out(const std::string& message) {
        if(
            (log_everything_ &&
                log_features_exclude_.find(current_feature_) ==
                log_features_exclude_.end())
            || (log_features_.find(current_feature_) != log_features_.end())
        ) {
            std::string feat_msg =
                CmdLine::ui_feature(current_feature_, current_feature_changed_)
                + message;

	    for(auto it : clients_) {
                it->out(feat_msg);
            }

            current_feature_changed_ = false;
        }
    }

    void Logger::notify_warn(const std::string& message) {
        std::string msg = "Warning: " + message;
        std::string feat_msg =
            CmdLine::ui_feature(current_feature_, current_feature_changed_)
            + msg;

        for(auto it : clients_) {
            it->warn(feat_msg);
            it->status(msg);
        }

        current_feature_changed_ = false;
    }

    void Logger::notify_err(const std::string& message) {
        std::string msg = "Error: " + message;
        std::string feat_msg =
            CmdLine::ui_feature(current_feature_, current_feature_changed_)
            + msg;

	if(notifying_error_) {
	    std::cerr << "Error while displaying error (!):"
		      << feat_msg << std::endl;
	} else {
	    notifying_error_ = true;
	    for(auto it : clients_) {
		it->err(feat_msg);
		it->status(msg);
	    }
	    notifying_error_ = false;
	}

        current_feature_changed_ = false;
    }

    void Logger::notify_status(const std::string& message) {
        for(auto it : clients_) {
            it->status(message);
        }

        current_feature_changed_ = false;
    }

    void Logger::notify(LoggerStream* s, const std::string& message) {

        if(quiet_ || (minimal_ && s == &out_) || clients_.empty()) {
            return;
        }

        if(s == &out_) {
            notify_out(message);
        } else if(s == &warn_) {
            notify_warn(message);
        } else if(s == &err_) {
            notify_err(message);
        } else if(s == &status_) {
            notify_status(message);
        } else {
            geo_assert_not_reached;
        }
    }

    /************************************************************************/
    
}

extern "C" {

    int geogram_printf(const char* format, ...) {

        static std::string last_string;

        va_list args;

        // Get the number of characters to be printed.        
        va_start(args, format);
        int nb = vsnprintf(nullptr, 0, format, args)+1; // +1, I don't know why...
        va_end(args);

        // Generate the output string
        GEO::vector<char> buffer(GEO::index_t(nb+1));
        va_start(args, format);
        vsnprintf(buffer.data(),buffer.size()-1, format, args);
        va_end(args);

        // Find the lines in the generated string
        GEO::vector<char*> lines;
        lines.push_back(buffer.data());
        char last_char = '\n';
        for(char* ptr = buffer.data(); *ptr; ptr++) {
            if(*ptr != '\0') {
                last_char = *ptr;
            }
            if(*ptr == '\n') {
                *ptr = '\0';
                ptr++;
                if(*ptr != '\0') {
                    lines.push_back(ptr);
                }
            }
        }

        // If last character is not a carriage return,
        // memorize the last line for later.
        if(last_char != '\n') {
            last_string += *lines.rbegin();
            lines.pop_back();
        }

        // Output all the lines.
        // Prepend the optionally memorized previous strings to the
        // first one.
        for(GEO::index_t i=0; i<lines.size(); ++i) {
            if(i == 0) {
                GEO::Logger::out("") << last_string << lines[i] << std::endl;
                last_string.clear();
            } else {
                GEO::Logger::out("") << lines[i] << std::endl;                
            }
        }

	return nb;
    }

    int geogram_fprintf(FILE* out, const char* format, ...) {


        static std::string last_string;

        va_list args;

        // Get the number of characters to be printed.        
        va_start(args, format);
        int nb = vsnprintf(nullptr, 0, format, args)+1; // +1, I don't know why...
        va_end(args);

        // Generate the output string
        GEO::vector<char> buffer(GEO::index_t(nb+1));
        va_start(args, format);
        vsnprintf(buffer.data(),buffer.size()-1, format, args);
        va_end(args);

        // Find the lines in the generated string
        GEO::vector<char*> lines;
        lines.push_back(buffer.data());
        char last_char = '\n';
        for(char* ptr = buffer.data(); *ptr; ptr++) {
            if(*ptr != '\0') {
                last_char = *ptr;
            }
            if(*ptr == '\n') {
                *ptr = '\0';
                ptr++;
                if(*ptr != '\0') {
                    lines.push_back(ptr);
                }
            }
        }

        // If last character is not a carriage return,
        // memorize the last line for later.
        if(last_char != '\n') {
            last_string += *lines.rbegin();
            lines.pop_back();
        }

        // Output all the lines.
        // Prepend the optionally memorized previous strings to the
        // first one.
        for(GEO::index_t i=0; i<lines.size(); ++i) {
            if(i == 0) {
                if(out == stdout) {
                    GEO::Logger::out("") << last_string << lines[i] << std::endl;
                } else if(out == stderr) {
                    GEO::Logger::err("") << last_string << lines[i] << std::endl;                    
                } else {
                    fprintf(out, "%s%s", last_string.c_str(), lines[i]);                    
                }
                last_string.clear();
            } else {
                if(out == stdout) {
                    GEO::Logger::out("") << lines[i] << std::endl;
                } else if(out == stderr) {
                    GEO::Logger::err("") << lines[i] << std::endl;                    
                } else {
                    fprintf(out, "%s", lines[i]);                    
                }
            }
        }
	
	return nb;
    }
}

