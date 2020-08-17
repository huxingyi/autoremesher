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

#ifndef GEOGRAM_BASIC_LOGGER
#define GEOGRAM_BASIC_LOGGER

#ifdef __cplusplus

#include <geogram/basic/common.h>
#include <geogram/basic/environment.h>
#include <geogram/basic/process.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <stdlib.h>

/**
 * \file geogram/basic/logger.h
 * \brief Generic logging mechanism
 */

namespace GEO {

    class Logger;
    class LoggerStream;

    /**
     * \brief Stream buffer used by the LoggerStream%s
     * \details This class is used internally to implement the logger
     * mechanism. Since it inherits a STL class, it is declared as 
     * NO_GEOGRAM_API so that it is not exported when Windows DLLs
     * are generated (doing otherwise would generate multiply defined
     * symbols).
     */
    class NO_GEOGRAM_API LoggerStreamBuf : public std::stringbuf {
    public:
        /**
         * \brief Creates a Logger stream buffer
         * \details Creates a LoggerStreamBuf associated to the LoggerStream
         * \p loggerStream
         * \param[in] loggerStream the LoggerStream that owns this buffer
         */
        LoggerStreamBuf(LoggerStream* loggerStream) :
  	loggerStream_(loggerStream) {
        }

    private:
        /**
         * \brief Synchronizes stream buffer
         * \details Reimplementation of function std::stringbuf::sync() that
         * sends the character sequence to the LoggerStream
         * \retval zero, on success.
         * \retval -1 on failure.
         * \see LoggerStream::notify()
         */
        virtual int sync();

    private:
        LoggerStream* loggerStream_;
    };

    /************************************************************************/

    /**
     * \brief Stream used by the Logger
     * \details This class is used used internally to implement logger
     * mechanism. Since it inherits a STL class, it is declared as 
     * NO_GEOGRAM_API so that it is not exported when Windows DLLs
     * are generated (doing otherwise would generate multiply defined
     * symbols).
     */
    class NO_GEOGRAM_API LoggerStream : public std::ostream {
    public:
        /**
         * \brief Creates a Logger stream
         * \details Creates a LoggerStream associated to the Logger \p logger
         * \param[in] logger the Logger that owns this stream
         */
        LoggerStream(Logger* logger);

        /**
         * \brief Logger stream destructor
         */
        virtual ~LoggerStream();

    protected:
        /**
         * \brief Sends a string to the Logger
         * \details This function is called by LoggerStreamBuf::sync() when a
         * sequence of characters \p str is available in the stream. This
         * sequence is sent back to the logger to deliver to the
         * LoggerClient%s
         * \param[in] str the sequence of characters to send
         * \see LoggerStreamBuf::sync()
         */
        void notify(const std::string& str);

    private:
        Logger* logger_;
        friend class LoggerStreamBuf;
    };

    /************************************************************************/

    /**
     * \brief Logger client base class
     * \details Messages sent to the Logger are sent back to registered
     * LoggerClient%s. Logger clients must implement the following functions
     * to handle the messages:
     * - div() - to create a new division
     * - out() - to handle information messages
     * - warn() - to handle warning messages
     * - err() - to handle error messages
     * - status() - to handle status messages
     * It is the responsibility of the derived LoggerClient%s to handle the
     * various kind of messages sent by the Logger appropriately.
     */
    class GEOGRAM_API LoggerClient : public Counted {
    public:
        /**
         * \brief Creates a new division
         * \details This creates a new division entitled with \p title
         * \param[in] title the text of the title
         */
        virtual void div(const std::string& title) = 0;

        /**
         * \brief Handles an information message
         * \param[in] str the text of the message
         */
        virtual void out(const std::string& str) = 0;

        /**
         * \brief Handles a warning message
         * \param[in] str the text of the message
         */
        virtual void warn(const std::string& str) = 0;

        /**
         * \brief Handles an error message
         * \param[in] str the text of the message
         */
        virtual void err(const std::string& str) = 0;

        /**
         * \brief Handles a status message
         * \param[in] str the text of the message
         */
        virtual void status(const std::string& str) = 0;

        /**
         * \brief LoggerClient destructor
         */
        virtual ~LoggerClient();
    };

    /** Smart pointer that contains a LoggerClient object */
    typedef SmartPointer<LoggerClient> LoggerClient_var;

    /************************************************************************/

    /**
     * \brief Logger client that redirects messages to standard output.
     */
    class GEOGRAM_API ConsoleLogger : public LoggerClient {
    public:
        /**
         * \brief Creates a ConsoleLogger
         */
        ConsoleLogger();

        /**
         * \copydoc LoggerClient::div()
         */
        void div(const std::string& title);

        /**
         * \copydoc LoggerClient::out()
         */
        void out(const std::string& str);

        /**
         * \copydoc LoggerClient::warn()
         */
        void warn(const std::string& str);

        /**
         * \copydoc LoggerClient::err()
         */
        void err(const std::string& str);

        /**
         * \copydoc LoggerClient::status()
         * This function does actually nothing
         */
        void status(const std::string& str);

    protected:
        /**
         * \brief ConsoleLogger destructor
         */
        virtual ~ConsoleLogger();
    };

    /************************************************************************/

    /**
     * \brief Logger client that redirects messages to a file.
     */
    class GEOGRAM_API FileLogger : public LoggerClient {
    public:
        /**
         * \brief Creates an empty file logger
         * \details The default constructed file logger does not handle
         * messages until it is set a filename with set_file_name()
         */
        FileLogger();

        /**
         * \brief Creates logger that logs messages to a file
         * \details All sent to the file logger are sent to the file
         * \p file_name.
         * \param[in] file_name name of the log file
         */
        FileLogger(const std::string& file_name);

        /**
         * \copydoc LoggerClient::div()
         */
        void div(const std::string& title);

        /**
         * \copydoc LoggerClient::out()
         */
        void out(const std::string& str);

        /**
         * \copydoc LoggerClient::warn()
         */
        void warn(const std::string& str);

        /**
         * \copydoc LoggerClient::err()
         */
        void err(const std::string& str);

        /**
         * \copydoc LoggerClient::status()
         * This function does actually nothing
         */
        void status(const std::string& str);

    protected:
        /**
         * \brief FileLogger destructor
         */
        virtual ~FileLogger();

        /**
         * \brief Sets the log file name
         * \details If the client already had a file name, the corresponding
         * file stream is closed and reopened with \p file_name.
         * \param[in] file_name the name of the log file
         */
        void set_file_name(const std::string& file_name);

    private:
        std::string log_file_name_;
        std::ostream* log_file_;
    };

    /************************************************************************/

    /**
     * \brief Generic logging framework.
     *
     * The Logger is a framework for logging messages with different
     * severities to various destinations.
     *
     * Logging destinations can be specified by registering LoggerClient%s to
     * the Logger (see register_client()). Predefined clients exist to log
     * messages to a file or to the console with a pretty or standard
     * formatting (see set_pretty()). Any number of clients can be registered
     * to the Logger.
     *
     * The Logger provides 4 level of severities, each of them having its own
     * LoggerStream:
     * - information: out()
     * - warning: warn()
     * - error: err()
     * - status: status()
     *
     * Thus logging a message to the specific stream is equivalent of sending
     * a message of the correspnding severity. For instance, logging a message
     * to the warn() stream means sending a warning message to the Logger.
     *
     * The Logger also provides a pseudo stream div() that creates a division
     * in the log output, that is the log can be structured in kind of
     * chapters introduced by a heading title.
     *
     * Messages are associated to features. A feature can be considered as the
     * source context (eg: messages sent to the Logger by the mesh I/O module
     * specify feature "I/O"). The feature is specified when selecting the
     * stream to. When a message is sent to the LoggerClient%s by the Logger,
     * it contains information about its severity and the associated feature.
     *
     * Features are not only information about of the message source, they
     * also support a message filtering mechanism. Specific features can be
     * enabled by setting the Logger property \e log:features to a
     * colon-separated list of enabled feature names, or disabled by setting
     * the Logger property \e log:features_exclude to a colon-separated list
     * of excluded feature names (see set_value()). Note that setting property
     * \e log:features to the special value "*" globally enables all logging
     * features (this is the default).
     *
     * The Logger can also be turned off temporarily by setting the quiet mode
     * to \c true, which disables all messages, warnings and errors included
     * (set set_quiet()).
     */
    class GEOGRAM_API Logger : public Environment {
    public:
        /**
         * \brief Initializes the logging system
         * \details This function must be called once at program startup to
         * create and initialize the Logger instance. It is called by
         * GEO::initialize().
         * \see instance()
         */
        static void initialize();

        /**
         * \brief Terminates the logging system
         * \details This function must be called once when the program ends to
         * delete the Logger instance. It is called by GEO::terminate()
         * \see instance()
         */
        static void terminate();

        /**
         * \brief Returns the Logger single instance
         * \details This function does \b not create the Logger instance.
         * Calling instance() before initialize() has been called returns a \c
         * null pointer. Similarly, calling instance() after terminate() 
         * has been called returns a \c null pointer.
         * \return A pointer to the Logger if initialized, null otherwise
         * \see initialize()
         * \see terminate()
         */
        static Logger* instance();


        /**
         * \brief Tests whether the Logger is initialized.
         * \details Certain error-reporting functions may be triggered
         *  before the Logger is initialized or after it is terminated.
         *  This function is meant to help them determine whether the
         *  logger can be used.
         * \retval true if the Logger can be used
         * \retval false otherwise
         */
        static bool is_initialized();
        
       
        /**
         * \brief Creates a division in the log output
         * \details This is used to start a new "block" of output log with
         * title \p title. LoggerClients are free to honor div messages or to
         * implement them the way they prefer.
         * Example:
         * \code
         * Logger::div("new section") << "message" << endl ;
         * \endcode
         * \param[in] title title of the division
         */
        static std::ostream& div(const std::string& title);

        /**
         * \brief Gets the stream to send information messages.
         * \details Example:
         * \code
         * Logger::out("feature_name") << "initialized" << endl ;
         * \endcode
         * \param[in] feature name of the feature associated to the incoming
         * information messages
         * \return a reference to the information stream
         */
        static std::ostream& out(const std::string& feature);

        /**
         * \brief Gets the stream to send error messages
         * \details Example:
         * \code
         * Logger::err("feature_name") << "problem with args" << endl ;
         * \endcode
         * \param[in] feature name of the feature associated to the incoming
         * warning messages
         * \return a reference to the warning stream
         */
        static std::ostream& err(const std::string& feature);

        /**
         * \brief Gets the stream to send warning messages
         * \details Example:
         * \code
         * Logger::warn("feature_name") << "strange value" << endl ;
         * \endcode
         * \param[in] feature name of the feature associated to the incoming
         * error messages
         * \return a reference to the error stream
         */
        static std::ostream& warn(const std::string& feature);

        /**
         * \brief Gets the stream to send status messages
         * \details Example:
         * \code
         * Logger::status() << "Hyperdrive activated" << endl ;
         * \endcode
         * \return a reference to the status stream
         */
        static std::ostream& status();

        /**
         * \brief Adds a client to the Logger
         * \details This adds client \p client to the existing clients and
         * starts sending messages to it. The Logger takes ownership on
         * the \p client, so there's no need to delete it, unless the client
         * is unregistered by unregister_client().
         * \param[in] client a logger client
         * \see unregister_client()
         */
        void register_client(LoggerClient* client);

        /**
         * \brief Removes a client from the Logger
         * \details This removes client \p client from the list of registered
         * clients if present. After being removed, the client will no longer
         * receive messages from the Logger. It is also the responsibility of
         * the client code to delete the client appropriately.
         * \param[in] client a logger client
         * \see register_client()
         */
        void unregister_client(LoggerClient* client);

        /**
         * \brief Unregisters all the registered clients.
         */
        void unregister_all_clients();
        
        /**
         * \brief Checks if a client is registered
         * \param[in] client a logger client
         * \retval true if \p client is registered to the Logger
         * \retval false otherwise
         */
        bool is_client(LoggerClient* client) const;

        /**
         * \brief Sets the quiet mode
         * \details When the Logger is in quiet mode, all messages sent to it
         * are ignored and not dispatched to the registered clients. The quiet
         * mode can also be set by setting the value of the property
         * "log:quiet" with set_value().
         * \param[in] flag set to true/false to turn the quiet mode on/off
         * \note The quiet mode is on by default
         * \see set_value()
         */
        void set_quiet(bool flag);

        /**
         * \brief Checks the quiet mode
         * \retval true if the quiet mode is on
         * \retval false otherwise
         */
        bool is_quiet() const {
            return quiet_;
        }


        /**
         * \brief Sets the minimal mode
         * \details When the Logger is in minimal mode, only warning and error
         * messages sent to it are dispatched to the registered clients. 
         * The minimal mode can also be set by setting the value of the property
         * "log:minimal" with set_value().
         * \param[in] flag set to true/false to turn the minimal mode on/off
         * \note The minimal mode is off by default
         * \see set_value()
         */
        void set_minimal(bool flag);

        /**
         * \brief Checks the minimal mode
         * \retval true if the minimal mode is on
         * \retval false otherwise
         */
        bool is_minimal() const {
            return minimal_;
        }
        
        /**
         * \brief Sets the console pretty mode
         * \details When the Logger console is in pretty mode, messages are
         * formatted in a fancy way using nice boxes with titles, and long
         * messages are smartly wrapped and aligned on the message features.
         * Otherwise, the messages are displayed "as is". The console pretty
         * mode can also be set by setting the value of the property
         * "log:pretty" with set_value().
         * \param[in] flag set to true/false to turn the quiet mode on/off
         */
        void set_pretty(bool flag);

        /**
         * \brief Checks the console pretty mode
         * \retval true if the console pretty mode is on
         * \retval false otherwise
         */
        bool is_pretty() const {
            return pretty_;
        }

    protected:
        /**
         * \brief Logger default constructor
         * \details The constructor is never called directly but through a call
         * to initialize().
         */
        Logger();

        /**
         * \brief Logger destructor
         */
        virtual ~Logger();

        /** \copydoc div() */
        std::ostream& div_stream(const std::string& title);

        /** \copydoc out() */
        std::ostream& out_stream(const std::string& feature);

        /** \copydoc err() */
        std::ostream& err_stream(const std::string& feature);

        /** \copydoc warn() */
        std::ostream& warn_stream(const std::string& feature);

        /** \copydoc status() */
        std::ostream& status_stream();

        /**
         * \brief Receives a message from a logger stream
         * \details This function is called by the LoggerStream \p stream when
         * a new sequence of characters \p message is sent to the stream. The
         * function dispatches the message to appropriate handling functions
         * notify_xxx() according to the type of the stream
         * \param[in] sender the LoggerStream that sent the message
         * \param[in] message the text of the message
         * \see LoggerStream()
         * \see notify_out()
         * \see notify_warn()
         * \see notify_err()
         * \see notify_status()
         */
        void notify(LoggerStream* sender, const std::string& message);

        /**
         * \brief Handles an information message
         * \details This formats the information message and sends it to the
         * registered clients by calling their function out(). Information
         * messages are sent to the clients only if the current Logger feature
         * matches the current filter or does not matches any feature
         * exclusion rule.
         * sent to the clients.
         * \param[in] message text of the message
         * \see LoggerClient::out()
         */
        void notify_out(const std::string& message);

        /**
         * \brief Handles a warning message
         * \details This formats the warning message and sends it to the
         * registered clients by calling their function warn(). Warning
         * messages ignore feature filters or exclusion rules and are always
         * sent to the clients.
         * \param[in] message text of the message
         * \see LoggerClient::warn()
         */
        void notify_warn(const std::string& message);

        /**
         * \brief Handles an error message
         * \details This formats the error message and sends it to the
         * registered clients by calling their function err(). Error
         * messages ignore feature filters or exclusion rules and are always
         * sent to the clients.
         * \param[in] message text of the message
         * \see LoggerClient::err()
         */
        void notify_err(const std::string& message);

        /**
         * \brief Handles a status message
         * \details This formats the status message and sends it to the
         * registered clients by calling their function status(). Status
         * messages ignore feature filters or exclusion rules and are always
         * sent to the clients.
         * \param[in] message text of the message
         * \see LoggerClient::status()
         */
        void notify_status(const std::string& message);

        /**
         * \brief Sets a Logger property
         * \details Sets the property \p name with value \p value in the
         * Logger. The property must be a valid Logger property (see log:xxx
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
        );

        /**
         * \brief Gets a Logger property
         * \details Retrieves the value of the property \p name and stores it
         * in \p value. The property must be a valid Logger property (see
         * log:xxx properties in Vorpaline's help).
         * \param[in] name name of the property
         * \param[out] value receives the value of the property
         * \retval true if the property is a valid Logger property
         * \retval false otherwise
         * \see Environment::get_value()
         */
        virtual bool get_local_value(
            const std::string& name, std::string& value
        ) const;

    private:
        static SmartPointer<Logger> instance_;

        LoggerStream out_;
        LoggerStream warn_;
        LoggerStream err_;
        LoggerStream status_;

        // features we want or don't want to log (only applies to 'out').

        /** Set of allowed or excluded features */
        typedef std::set<std::string> FeatureSet;
        FeatureSet log_features_;
        FeatureSet log_features_exclude_;
        bool log_everything_;
        std::string log_file_name_;

        std::string current_feature_;
        bool current_feature_changed_;

        /** Set of registered LoggerClient%s */
        typedef std::set<LoggerClient_var> LoggerClients;
        LoggerClients clients_; // list of registered clients

        bool quiet_;
        bool pretty_;
        bool minimal_;
	bool notifying_error_;
        
        friend class LoggerStream;
        friend class LoggerStreamBuf;
    };

    /************************************************************************/

}

extern "C" {
    /**
     * \brief Printf-like wrapper to the Logger
     * \details
     * By #%defining printf to geogram_printf, legacy code can send printf
     * formatted messages directly to Logger::out().
     * \param[in] format printf-like format string
     * \see printf
     */
    int GEOGRAM_API geogram_printf(const char* format, ...);

    /**
     * \brief Fprintf-like wrapper to the Logger
     * \details
     * By #%defining fprintf to geogram_fprintf, legacy code can send fprintf
     * formatted messages directly to the Logger:
     * - formatted text printed to stdout is sent to Logger::out()
     * - formatted text printed to stderr is sent to Logger::err()
     * - otherwise the formatted text is printed to \p out using 
     *   the system fprintf.
     * \param[in] out output file
     * \param[in] format printf-like format string
     * \see fprintf
     */
    int GEOGRAM_API geogram_fprintf(FILE* out, const char* format, ...);
}

#else

#include <stdlib.h>

#ifndef GEOGRAM_API
#define GEOGRAM_API
#endif

/**
 * \brief Printf-like wrapper to the Logger
 * \details
 * By #%defining printf to geogram_printf, legacy code can send printf
 * formatted messages directly to Logger::out().
 * \param[in] format printf-like format string
 * \see printf
 */
extern int GEOGRAM_API geogram_printf(const char* format, ...);

/**
 * \brief Fprintf-like wrapper to the Logger
 * \details
 * By #%defining fprintf to geogram_fprintf, legacy code can send fprintf
 * formatted messages directly to the Logger:
 * - formatted text printed to stdout is sent to Logger::out()
 * - formatted text printed to stderr is sent to Logger::err()
 * - otherwise the formatted text is printed to \p out using the system fprintf.
 * \param[in] out output file
 * \param[in] format printf-like format string
 * \see fprintf
 */
extern int GEOGRAM_API geogram_fprintf(FILE* out, const char* format, ...);

#endif

#endif

