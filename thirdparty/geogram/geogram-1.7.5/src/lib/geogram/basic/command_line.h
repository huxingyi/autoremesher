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

#ifndef GEOGRAM_BASIC_COMMAND_LINE
#define GEOGRAM_BASIC_COMMAND_LINE

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/string.h>
#include <geogram/basic/assert.h>

/**
 * \file geogram/basic/command_line.h
 * \brief Functions to parse and store command line arguments
 */

namespace GEO {

    /**
     * \brief Utilities to process command line arguments.
     * \details CmdLine defines utility functions to declare program options
     * and parse command line arguments.
     */
    namespace CmdLine {

        /**
         * \brief Initializes the command line framework
         * \details This function must be called once at program start.
         * It is called by the general initialization function
         * GEO::initialize().
         * \see GEO::initialize().
         */
        void GEOGRAM_API initialize();

        /**
         * \brief Cleans up the command line framework
         * \details This function must be called at program termination
         * program. It is called by the general cleanup function
         * GEO::terminate().
         * \see GEO::terminate().
         */
        void GEOGRAM_API terminate();


	/**
	 * \brief Defines the name of the configuration file.
	 * \param[in] filename the name of the configuration file. Before
	 *  parsing command line arguments, arguments are set according
	 *  to this file, loaded from the home directory (or 'My Documents'
	 *  under Windows). Default is 'geogram.ini'.
	 * \param[in] auto_create_args if set, all the args present in the
	 *  configuration file are created if they do not already exist, else
	 *  a warning message is displayed for args that do not exist.
	 */
	void GEOGRAM_API set_config_file_name(
	    const std::string& filename,
	    bool auto_create_args = false
	);

	/**
	 * \brief Tests whether the configuration file was loaded.
	 * \details The default configuration file, or the one specified
	 *  by set_config_file_name() may not exist, in this case this
	 *  function returns false.
	 * \retval true if the configuration file was loaded.
	 * \retval false otherwise.
	 */
	bool GEOGRAM_API config_file_loaded();
	
	/**
	 * \brief Gets the name of the configuration file.
	 * \return the name of the configuration file, as
	 *  specified by set_config_file_name(). User's home directory
	 *  needs to be prepended to have the complete file path.
	 */
	std::string GEOGRAM_API get_config_file_name();

	/**
	 * \brief Loads command line argument values from a file.
	 * \details only args in the section with \p program_name
	 *  are loaded.
	 * \param[in] filename the complete path to the file.
	 * \param[in] program_name the name of the program.
	 */
	void GEOGRAM_API load_config(
	    const std::string& filename, const std::string& program_name
	);
	
        /**
         * \brief Command line argument types
         */
        enum ArgType {
            /** Argument type is undefined */
            ARG_UNDEFINED = 0,
            /** Argument is a signed integer */
            ARG_INT = 1,
            /** Argument is a floating point value */
            ARG_DOUBLE = 2,
            /** Argument is a string */
            ARG_STRING = 4,
            /** Argument is a boolean */
            ARG_BOOL = 8,
            /** Argument is a percentage */
            ARG_PERCENT = 16
        };

        /**
         * \brief Command line group or argument flags
         */
        enum ArgFlags {
            /** Default argument flags (nothing) */
            ARG_FLAGS_DEFAULT = 0,
            /** Argument is advanced */
            ARG_ADVANCED = 1
        };

        /**
         * \brief Declares an argument group
         * \details This creates a new argument group with the given name \p
         * name. If a group with the same name already exists, the new group
         * is not created.
         *
         * Argument groups are used to organize arguments in a set of
         * logically related arguments. For instance:
         * - group "sys" contains arguments to configure the Process module,
         * - group "log" contains arguments to configure the Logger,
         * - ...
         *
         *  To declare an argument in a given group, the argument name must be
         *  prefixed by the group name followed by a colon as in "group:arg".
         *
         * \param[in] name the group name
         * \param[in] description the group description
         * \param[in] flags the group flags
         * \see declare_arg()
         */
        void GEOGRAM_API declare_arg_group(
            const std::string& name,
            const std::string& description,
            ArgFlags flags = ARG_FLAGS_DEFAULT
        );

        /**
         * \brief Declares an argument
         * \details This is the general function for declaring an argument of
         * name \p name. The default value \p default_value is given as a
         * string and must be convertible to the given type \p type. The
         * command line framework provides type-safe variants of declare_arg()
         * for declaring arguments, it is highly recommended to use them.
         *
         * Argument names can have 2 forms:
         * - "group:arg" - the argument "arg" is added to the group "group".
         * - "arg" - without a group name, the argument "arg" is added to the
         *   default group "global"
         *
         * \param[in] name the argument name
         * \param[in] type the argument type
         * \param[in] default_value the default value as a string
         * \param[in] description the argument description
         * \param[in] flags the argument flags
         */
        void GEOGRAM_API declare_arg(
            const std::string& name,
            ArgType type,
            const std::string& default_value,
            const std::string& description,
            ArgFlags flags = ARG_FLAGS_DEFAULT
        );

        /**
         * \brief Gets the type of an argument
         * \param[in] name the argument name
         * \retval the type of the argument if it exists
         * \retval #ARG_UNDEFINED otherwise
         */
        ArgType GEOGRAM_API get_arg_type(const std::string& name);

        /**
         * \brief Checks if an argument exists
         * \param[in] name the argument name
         * \retval true if the argument exists
         * \retval false otherwise
         */
        bool GEOGRAM_API arg_is_declared(const std::string& name);

        /**
         * \brief Declares an argument of type string
         * \param[in] name the argument name
         * \param[in] default_value argument's default string value
         * \param[in] description argument description
         * \param[in] flags the argument flags
         */
        inline void declare_arg(
            const std::string& name,
            const std::string& default_value,
            const std::string& description,
            ArgFlags flags = ARG_FLAGS_DEFAULT
        ) {
            declare_arg(
                name, ARG_STRING, default_value,
                description, flags
            );
        }

        /**
         * \brief Declares an argument of type string
         * \param[in] name the argument name
         * \param[in] default_value argument's default C-string value
         * \param[in] description argument description
         * \param[in] flags the argument flags
         */
        inline void declare_arg(
            const std::string& name,
            const char* default_value,
            const std::string& description,
            ArgFlags flags = ARG_FLAGS_DEFAULT
        ) {
            declare_arg(
                name, ARG_STRING, default_value,
                description, flags
            );
        }

        /**
         * \brief Declares an argument of type integer
         * \param[in] name the argument name
         * \param[in] default_value argument's default integer value
         * \param[in] description argument description
         * \param[in] flags the argument flags
         */
        inline void declare_arg(
            const std::string& name,
            int default_value,
            const std::string& description,
            ArgFlags flags = ARG_FLAGS_DEFAULT
        ) {
            declare_arg(
                name, ARG_INT, String::to_string(default_value),
                description, flags
            );
        }

        /**
         * \brief Declares an argument of type floating point
         * \param[in] name the argument name
         * \param[in] default_value argument's default floating point value
         * \param[in] description argument description
         * \param[in] flags the argument flags
         */
        inline void declare_arg(
            const std::string& name,
            double default_value,
            const std::string& description,
            ArgFlags flags = ARG_FLAGS_DEFAULT
        ) {
            declare_arg(
                name, ARG_DOUBLE, String::to_string(default_value),
                description, flags
            );
        }

        /**
         * \brief Declares an argument of type boolean
         * \param[in] name the argument name
         * \param[in] default_value argument's default boolean value
         * \param[in] description argument description
         * \param[in] flags the argument flags
         */
        inline void declare_arg(
            const std::string& name,
            bool default_value,
            const std::string& description,
            ArgFlags flags = ARG_FLAGS_DEFAULT
        ) {
            declare_arg(
                name, ARG_BOOL, default_value ? "true" : "false",
                description, flags
            );
        }

        /**
         * \brief Declares an argument of type percentage
         * \details Percentage values are normal floating point values, except
         * that their string representation is followed by a percent "%" sign.
         * \param[in] name the argument name
         * \param[in] default_value argument's default percentage value
         * \param[in] description argument description
         * \param[in] flags the argument flags
         */
        inline void declare_arg_percent(
            const std::string& name,
            double default_value,
            const std::string& description = "...",
            ArgFlags flags = ARG_FLAGS_DEFAULT
        ) {
            declare_arg(
                name, ARG_PERCENT, String::to_string(default_value) + "%",
                description, flags
            );
        }

        /**
         * \brief Parses the command line arguments
         * \details This analyzes command line arguments passed to the main()
         * program in \p argc and \p argv. Arguments not matching program
         * options declared with declare_arg() are stored in output vector \p
         * unparsed_args.
         *
         * Command line parsing allows users to specify partial option names
         * on the command line. If a partial name matches more than 1 declared
         * argument, it is rejected and the functions fails, otherwise, it is
         * accepted and replaced by the full argument name.
         *
         * Parameter \p additional_arg_specs specifies how to
         * handle unparsed arguments. It consists in a white space separated
         * list of additional argument names expected by the program, with the
         * following meaning:
         * - "<arg_name>" - means that the argument is optional
         * - "arg_name" - means that the argument is mandatory
         *
         * For instance the following additional argument specification:
         * \code
         * "input_file <option_file> <output_file>
         * \endcode
         * means that the program expects at least one additional argument
         * \e input_file, with two additional optional arguments \e
         * option_file and \e output_file.
         *
         * If argument "-h" is found in the list of arguments, then the
         * program displays a summary of program options and exits (see
         * show_usage()).
         *
         * \param[in] argc number of arguments passed to main()
         * \param[in] argv array of command line arguments passed to main()
         * \param[out] unparsed_args output vector of unparsed arguments
         * \param[in] additional_arg_specs unparsed argument specification
         * \retval true if the command line arguments are successfully parsed
         * \retval false otherwise
         */
        bool GEOGRAM_API parse(
            int argc, char** argv, std::vector<std::string>& unparsed_args,
            const std::string& additional_arg_specs = ""
        );

        /**
         * \brief Parses the command line arguments
         * \details This is a simplified version of parse() which does not
         * accept additional unparsed arguments.
         * \param[in] argc number of arguments passed to main()
         * \param[in] argv array of command line arguments passed to main()
         * \retval true if the command line arguments are successfully parsed
         * \retval false otherwise
         */
        bool GEOGRAM_API parse(
            int argc, char** argv
        );

	/**
	 * \brief Gets the number of arguments of the command line.
	 * \return the number of arguments plus one.
	 * \details parse() should be called before.
	 */
	int GEOGRAM_API argc();

	
	typedef char** charptrptr; // Need to do that else the compiler thinks
	                           // that GEOGRAM_API qualifies the ptr instead
	                           // of the function.
	
	/**
	 * \brief Gets the command line arguments.
	 * \return a pointer to an array of null-terminated strings with
	 *  the command line arguments. The first one is the program name.
	 * \details parse() should be called before.
	 */
	charptrptr GEOGRAM_API argv();
	
        /**
         * \brief Displays program help
         * \details Displays a list of all declared arguments (sorted by
         * argument group) with their current value. By default, show_usage()
         * only displays standard groups and arguments. If parameter \p
         * advanced is set to \c true, show_usage() also displays advanced
         * groups and arguments (declared with flag ARG_ADVANCED).
         * \param[in] additional_args additional argument specification (see
         * parse()).
         * \param[in] advanced boolean flag that controls the display of
         * advanced groups and arguments.
         */
        void GEOGRAM_API show_usage(
            const std::string& additional_args = "",
            bool advanced = false
        );

        /**
         * \brief Gets an argument value
         * \details Retrieves the string value of argument \p name. If the
         * argument does not exist, then the function calls abort().
         * \param[in] name the argument name
         * \return the value of the argument as a string if it exists
         */
        std::string GEOGRAM_API get_arg(const std::string& name);

        /**
         * \brief Gets an argument value as an integer
         * \details Retrieves the value of argument \p name and converts it to
         * an integer. If the argument does not exists or its value is not
         * convertible to an integer, then the function aborts.
         * \param[in] name the argument name
         * \return the argument value converted to an integer if the argument
         * exists
         * \see String::to_int()
         */
        int GEOGRAM_API get_arg_int(const std::string& name);

        /**
         * \brief Gets an argument value as an unsigned integer
         * \details Retrieves the value of argument \p name and converts it to
         * an unsigned integer. If the argument does not exists or its value is not
         * convertible to an unsigned integer, then the function aborts.
         * \param[in] name the argument name
         * \return the argument value converted to an unsigned integer if the argument
         * exists
         * \see String::to_uint()
         */
        unsigned int GEOGRAM_API get_arg_uint(const std::string& name);

        /**
         * \brief Gets an argument value as a floating point
         * \details Retrieves the value of argument \p name and converts it to
         * a floating point. If the argument does not exists or its value is
         * not convertible to a floating point, then the function aborts.
         * \param[in] name the argument name
         * \return the argument value converted to a floating point if the
         * argument exists
         * \see String::to_double()
         */
        double GEOGRAM_API get_arg_double(const std::string& name);

        /**
         * \brief Gets an argument value as a percentage
         * \details Retrieves the value of argument \p name:
         * - if the value has the form of a percentage \e "dd%", it is
         *   considered as a relative error of the parameter \p reference. The
         *   returned value is the floating point value \e 0.01*dd multiplied
         *   by the parameter \p reference.
         * - if the value is directly convertible to a floating point number,
         *   it is considered as an "absolute error" and returned as is. In this
         *   case parameter \p reference is ignored.
         * - in all other cases, the function aborts
         * \param[in] name the argument name
         * \param[in] reference the value to apply the percentage to
         * \return either:
         * - the argument value converted to a percentage of the \p reference
         * - or the floating point value of argument \p name
         * \see String::to_double()
         */
        double GEOGRAM_API get_arg_percent(
            const std::string& name, double reference
        );

        /**
         * \brief Gets an argument value as a boolean
         * \details Retrieves the value of argument \p name and converts it to
         * an integer. If the argument does not exists or its value is not
         * convertible to a boolean, then the function aborts.
         * \param[in] name the argument name
         * \return the argument value converted to a boolean if the
         * argument exists
         * \see String::to_bool()
         */
        bool GEOGRAM_API get_arg_bool(const std::string& name);

        /**
         * \brief Sets an argument value from a string
         * \details This replaces the value of argument \p name by the given
         * string \p value. If the string \p value is not strictly convertible
         * to the declared type of the argument then the function aborts. If
         * the argument does not exist, it is added as a new argument of
         * undefined type.
         * \param[in] name the argument name
         * \param[in] value the new value as a string
         * \retval true if the argument was successfully set
         * \retval false otherwise
         */
        bool GEOGRAM_API set_arg(
            const std::string& name, const std::string& value
        );

        /**
         * \brief Sets an argument value from a C-string
         * \details This replaces the value of argument \p name by the given
         * C-string \p value. If the string \p value is not strictly
         * convertible to the declared type of the argument then the function
         * aborts. If the argument does not exist, it is added as a new
         * argument of undefined type.
         * \param[in] name the argument name
         * \param[in] value the new value as a C-string
         * \retval true if the argument was successfully set
         * \retval false otherwise
         */
        inline bool set_arg(const std::string& name, const char* value) {
            return set_arg(name, std::string(value));
        }

        /**
         * \brief Sets an argument value from an integer
         * \details This replaces the value of argument \p name by the given
         * integer \p value. If the declared type of the argument is not
         * compatible with an integer then the function aborts (compatible
         * argument types are: int, double or string). If the argument does
         * not exist, it is added as a new argument of undefined type.
         * \param[in] name the argument name
         * \param[in] value the new value as an integer
         */
        void GEOGRAM_API set_arg(const std::string& name, int value);

        /**
         * \brief Sets an argument value from an unsigned integer
         * \details This replaces the value of argument \p name by the given
         * unsigned integer \p value. If the declared type of the argument is
         * not compatible with an unsigned integer then the function aborts
         * (compatible argument types are: int, double or string). If the
         * argument does not exist, it is added as a new argument of undefined
         * type.
         * \param[in] name the argument name
         * \param[in] value the new value as an unsigned integer
         */
        void GEOGRAM_API set_arg(const std::string& name, unsigned int value);

        /**
         * \brief Sets an argument value from a floating point
         * \details This replaces the value of argument \p name by the given
         * floating point \p value. If the declared type of the argument is
         * not compatible with a floating point then the function aborts
         * (compatible argument types are: double or string). If the argument
         * does not exist, it is added as a new argument of undefined type.
         * \param[in] name the argument name
         * \param[in] value the new value as a floating point
         */
        void GEOGRAM_API set_arg(const std::string& name, double value);

        /**
         * \brief Sets an argument value from a boolean
         * \details This replaces the value of argument \p name by the given
         * boolean \p value. If the declared type of the argument is not
         * compatible with a boolean then the function aborts (compatible
         * argument types are: boolean or string). If the argument does not
         * exist, it is added as a new argument of undefined type.
         * \param[in] name the argument name
         * \param[in] value the new value as a floating point
         */
        void GEOGRAM_API set_arg(const std::string& name, bool value);

        /**
         * \brief Sets an argument value from a percentage
         * \details This replaces the value of argument \p name by the given
         * floating point \p value. If the declared type of the argument is
         * not compatible with a floating point then the function aborts
         * (compatible argument types are: double or string), otherwise the
         * argument receives the \p value converted to a floating with a
         * trailing '%' sign. If the argument does not exist, it is added as a
         * new argument of undefined type.
         * \param[in] name the argument name
         * \param[in] value the new value as a floating point
         */
        void GEOGRAM_API set_arg_percent(const std::string& name, double value);

        /********************************************************************/

        /**
         * \brief Gets the value of all arguments
         * \details Stores in vector \p args the value of all declared
         * arguments in the form "name=value".
         * \param[out] args output vector
         */
        void GEOGRAM_API get_args(std::vector<std::string>& args);

        /**
         * \brief Gets the width of the console.
         * \return the console width in number of characters
         */
        index_t GEOGRAM_API ui_terminal_width();

        /**
         * \brief Outputs a separator with a title on the console
         * \details Prints a separator string on the console that contains the
         * given \p title and optional \p short_title. If it is specified, the
         * \p short_title appears on the left of \p title in the separator as
         * illustrated below. This function is used by Logger::div() to create
         * a new division in the log output.
         *
         * When the Logger is in pretty mode, the separator has the following
         * form:
         * \code
         * _/ ==[short_title]====[title]== \\______
         * |                                        |
         * \endcode
         *
         * Otherwise the separator has the following form
         * In non-pretty mode, or if the program output is redirected to a
         * file, the separator has the following form:
         * \code
         * ==[short_title]====[title]==
         * \endcode
         *
         * \param[in] title the main title string
         * \param[in] short_title optional title that appears on the left of
         * \p title
         * \see Logger::div()
         */
        void GEOGRAM_API ui_separator(
            const std::string& title,
            const std::string& short_title = ""
        );

        /**
         * \brief Outputs a separator without a title on the console.
         * \details Prints a simple separator string on the console.
         * When the Logger is in pretty mode, the separator has the following
         * form:
         * \code
         * ________________________________________
         * |                                        |
         * \endcode
         *
         * In non-pretty mode, or if the program output is redirected to a
         * file, nothing is printed.
         */
        void GEOGRAM_API ui_separator();

        /**
         * \brief Closes an opened separator
         * \details This closes the last opened box.
         * When the Logger is in pretty mode, the separator has the following
         * form:
         * \code
         * \\________________________________________/
         * \endcode
         *
         * In non-pretty mode, or if the program output is redirected to a
         * file, nothing is printed.
         */
        void GEOGRAM_API ui_close_separator();

        /**
         * \brief Outputs a message on the console.
         * \details Prints the message \p message to the console. In pretty
         * mode, the message is enclosed in vertical bars, thus forming a box
         * with the previous separator (see ui_separator()). If the message is
         * too long to fit in the console width, it is wrapped on the next
         * line(s) with an additional left margin given by \p wrap_margin.
         * \code
         * | This is an example of a very very long |
         * | <wrap_margin> message that wraps on th |
         * | <wrap_margin> e next lines             |
         * \endcode
         *
         * In non-pretty mode, the message is printed "as is".
         * \param[in] message the message to print
         * \param[in] wrap_margin extra left margin used to print continuation
         * lines of long messages
         */
        void GEOGRAM_API ui_message(
            const std::string& message,
            index_t wrap_margin
        );

        /**
         * \brief Outputs a message on the console.
         * \details This is a variant of ui_message() with a default
         * wrap_margin that keeps wrapped lines aligned with the end of the
         * feature names.
         * \param[in] message the message to print
         * \see ui_feature()
         */
        void GEOGRAM_API ui_message(
            const std::string& message
        );

        /**
         * \brief Clears the last line.
         * \details This function is only used in pretty mode and mainly to
         * display progress bars.
         */
        void GEOGRAM_API ui_clear_line();

        /**
         * \brief Displays a progress bar
         * \details This displays a progress bar for the given task \p task
         * name. The progress of the task is specified by the current progress
         * value \p val and the percentage of completion \p percent. The
         * progress bar is formatted as illustrated below:
         * \code
         * o-[task_name  ] (wheel)-[val]-[percent]--[waves]
         * \endcode
         * where:
         * - wheel is an animated rotating progress wheel
         * - waves is an animated progress bar
         * Once the task has completed, one should call ui_progress_time() to
         * display the elapsed time of for the task.
         * \param[in] task_name the name of the task in progress
         * \param[in] val the current progress value
         * \param[in] percent the percentage of completion
         * \param[in] clear whether to clear the line before displaying the
         * progress bar (default is \c true)
         * \see ui_progress_time()
         */
        void GEOGRAM_API ui_progress(
            const std::string& task_name, index_t val,
            index_t percent, bool clear = true
        );

        /**
         * \brief Displays the time elapsed for a completed task
         * \details Call this function after successive calls to ui_progress()
         * to display the elapsed time once the task task \p task_name has
         * completed.
         * \param[in] task_name the name of the task being completed.
         * \param[in] elapsed the time elapsed for completing the task
         * \param[in] clear whether to clear the line before displaying the
         * progress bar (default is \c true)
         * \see ui_progress()
         */
        void GEOGRAM_API ui_progress_time(
            const std::string& task_name,
            double elapsed, bool clear = true
        );

        /**
         * \brief Displays the time elapsed for a canceled task
         * \details Call this function when a task has been canceled to
         * display the elapsed time \p elapsed and the percentage of
         * completion \p percent.
         * \param[in] task_name the name of the task being completed.
         * \param[in] elapsed the time elapsed since the beginning of the task
         * \param[in] percent the percentage of completion of the task
         * \param[in] clear whether to clear the line before displaying the
         * progress bar (default is \c true)
         * \see ui_progress()
         */
        void GEOGRAM_API ui_progress_canceled(
            const std::string& task_name,
            double elapsed, index_t percent, bool clear = true
        );

        /**
         * \brief Formats a Logger feature name
         * \details Logger features are displayed with a special formatting in
         * the output log. They precede the messages sent to the Logger as
         * illustrated below:
         * \code
         * o-[feature    ] message...
         * \endcode
         *
         * Multiple messages sent to the Logger with the same feature display
         * the feature only once:
         *
         * \code
         * o-[feature    ] first message ...
         *                 second message with the same feature...
         * \endcode
         *
         * If parameter \p show is set to \c true, the function returns the
         * feature formatted as for the first message in the example above. If
         * \p show is set to \c false, the functions returns an empty
         * placeholder used as a left margin
         * length.
         * \param[in] feature the text of the feature
         * \param[in] show \c true to actually display the feature name or \c
         * false to return an empty placeholder of the same width
         * \return the formatted feature
         */
        std::string GEOGRAM_API ui_feature(
            const std::string& feature, bool show = true
        );
    }
}


#ifdef GEO_OS_ANDROID
struct android_app;

namespace GEO {
    namespace CmdLine {
	/**
	 * \brief Declares the current android app.
	 * \param[in] app a pointer to the android app.
	 */
	void GEOGRAM_API set_android_app(android_app* app);

	/**
	 * \brief Gets the android app.
	 * \return a pointer to the android app.
	 */
	android_app* GEOGRAM_API get_android_app();
    }
}

#endif


#endif

