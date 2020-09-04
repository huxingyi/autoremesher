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

#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/environment.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/process.h>
#include <geogram/bibliography/bibliography.h>
#include <geogram/NL/nl.h>
#include <iostream>
#include <iomanip>

#if defined(GEO_OS_LINUX) || defined(GEO_OS_APPLE)
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#endif

#ifdef GEO_OS_WINDOWS
#include <io.h> // for _isatty()
#endif

/**
 * \brief Checks argument type
 * \details Checks if argument type \p type is in allowed types \p
 * allowed_types or is undefined. Multiple types can be specified in \p
 * allowed_types by bit-OR'ing the values defined in enum
 * GEO::CmdLine::ArgType.
 * \param[in] type the type to be tested
 * \param[in] allowed_types argument types allowed for \p type.
 * \retval true if type is in \p allowed_types
 * \retval false otherwise
 * \see GEO::CmdLine::ArgType
 */
#define geo_assert_arg_type(type, allowed_types) \
    geo_assert(((type) & ~(allowed_types)) == 0)

namespace {

    using namespace GEO;
    using namespace CmdLine;

    std::string config_file_name = "geogram.ini";
    bool auto_create_args = false;
    bool loaded_config_file = false;
    
    int geo_argc = 0;
    char** geo_argv = nullptr;
    
    // True if displaying help in a way that
    // it will be easily processed by help2man
    bool man_mode = false;
    
    /**
     * \brief Command line argument
     * \details Arg stores information about command line arguments:
     */
    struct Arg {
        /** \brief The argument name */
        std::string name;
        /** \brief The argument description */
        std::string desc;
        /** \brief The argument type */
        ArgType type;
        /** \brief The argument flags */
        ArgFlags flags;
    };

    /** \brief Stores command line arguments by name */
    typedef std::map<std::string, Arg> Args;

    /**
     * \brief Stores the list of all arguments in a group
     * \see ArgGroup
     */
    typedef std::vector<std::string> GroupArgs;

    /**
     * \brief Command line argument group
     * \details ArgGroup stores information about command line argument groups
     */
    struct Group {
        /** \brief The group name */
        std::string name;
        /** \brief The group description */
        std::string desc;
        /** \brief The group flags */
        ArgFlags flags;
        /** \brief The list of arguments in the group */
        GroupArgs args;
    };

    /** \brief Stores command line argument groups by name */
    typedef std::map<std::string, Group> Groups;

    /** \brief List of group names ordered in declaration order */
    typedef std::vector<std::string> GroupNames;

    /**
     * \brief Command line private data
     */
    struct CommandLineDesc {
        /** \brief Program name */
        std::string argv0;
        /** \brief Table of all declared arguments indexed by name */
        Args args;
        /** \brief Table of all declared groups indexed by name */
        Groups groups;
        /** \brief Oredered list of declared group names */
        GroupNames group_names;
    };

    /** \brief Maximum length of a feature name */
    const unsigned int feature_max_length = 12;

    /** \brief Pointer to command line private data */
    CommandLineDesc* desc_ = nullptr;

    /**
     * \brief Checks if an argument name matches a sub-strung
     * \param[in] arg_in the sub-string to match
     * \param[in] arg_name the argument name
     * \retval true if \p arg_name contains \p arg_in
     * \retval false otherwise
     */
    bool arg_matches(
        const std::string& arg_in, const std::string& arg_name
    ) {
        return arg_name.find(arg_in) != std::string::npos;
    }

    /**
     * \brief Prints bad argument value error message
     * \param[in] name the argument name
     * \param[in] s the bad value
     * \param[in] type the argument type
     * \return \c false
     */
    bool arg_value_error(
        const std::string& name,
        const std::string& s, const char* type
    ) {
        Logger::instance()->set_quiet(false);
        Logger::err("CmdLine")
            << "Argument " << name << " received a bad value: '"
            << s << "' is not a " << type << " value"
            << std::endl;
        return false;
    }

    /**
     * \brief Checks the value of an argument
     * \details This function is used by parse_internal() to check the values
     * passed to arguments on the command line. It verifies that the string \p
     * s is convertible to the declared type of argument \p name (see
     * declare_arg()).
     * \param[in] name the argument name
     * \param[in] s the string value of the argument
     * \retval true if string \p s is convertible to the type of the argument
     * \retval false otherwise
     */
    bool check_arg_value(
        const std::string& name, const std::string& s
    ) {
        ArgType type = get_arg_type(name);
        if(type == ARG_UNDEFINED || type == ARG_STRING) {
            return true;
        }

        if(type == ARG_INT) {
            int value;
            if(!String::from_string(s, value)) {
                return arg_value_error(name, s, "integer");
            } else {
                return true;
            }
        }

        if(type == ARG_DOUBLE) {
            double value;
            if(!String::from_string(s, value)) {
                return arg_value_error(name, s, "floating point");
            } else {
                return true;
            }
        }

        if(type == ARG_BOOL) {
            bool value;
            if(!String::from_string(s, value)) {
                return arg_value_error(name, s, "boolean");
            } else {
                return true;
            }
        }

        if(type == ARG_PERCENT) {
            std::string s2 = s;
            if(s2.length() > 0 && s2[s2.length() - 1] == '%') {
                s2.resize(s2.length() - 1);
            }
            double value;
            if(!String::from_string(s2, value)) {
                return arg_value_error(name, s, "percentage");
            } else {
                return true;
            }
        }

        return false;
    }


    /**
     * \brief Parses the configuration file in the home directory.
     * \details The configuration file "geogram.ini" in the home directory
     *  has name=value pairs for pre-initializing command line arguments.
     *  In addition it has sections indicated by square-breacketed names.
     *  Only the arguments in the section with the same name as the program
     *  are taken into account. Section [*] refers to all possible programs.
     * \param[in] config_filename the name of the configuration file
     * \param[in] program_name the name of the program
     */
    void parse_config_file(
	const std::string& config_filename, const std::string& program_name
    ) {
	std::string section = "*";
	if(FileSystem::is_file(config_filename)) {
	    std::ifstream in(config_filename.c_str());
	    std::string line;
	    while(std::getline(in,line)) {
		if(line.length() >= 3 && line[0] == '[' && line[line.length()-1] == ']') {
		    section = String::to_uppercase(line.substr(1,line.length()-2));
		} else if(section == program_name || section == "*") {
		    size_t pos = line.find("=");
		    if(pos != std::string::npos) {
			std::string argname = line.substr(0,pos);
			std::string argval  = line.substr(pos+1,line.length()-pos-1);
			if(CmdLine::arg_is_declared(argname)) {
			    CmdLine::set_arg(argname, argval);
			} else {
			    if(auto_create_args) {
				CmdLine::declare_arg(argname, argval, "...");
			    } else {
				Logger::warn("config") << argname << "=" << argval << " ignored" << std::endl;
			    }
			}
		    }
		}
	    }
	    loaded_config_file= true;
	}
    }
    
    /**
     * \brief Parses the configuration file in the home directory.
     * \details The configuration file "geogram.ini" in the home directory
     *  has name=value pairs for pre-initializing command line arguments.
     *  In addition it has sections indicated by square-breacketed names.
     *  Only the arguments in the section with the same name as the program
     *  are taken into account. Section [*] refers to all possible programs.
     * \param[in] argc number of arguments passed to main()
     * \param[in] argv array of command line arguments passed to main()
     */
    void parse_config_file(int argc, char** argv) {
	geo_assert(argc >= 1);
	std::string program_name = String::to_uppercase(FileSystem::base_name(argv[0]));
	static bool init = false;
	if(init) {
	    return;
	}
	init = true;
	Logger::out("config") << "Configuration file name:" << config_file_name
			      << std::endl;
	Logger::out("config") << "Home directory:" << FileSystem::home_directory()
			      << std::endl;
	std::string config_filename = FileSystem::home_directory() + "/" + config_file_name;
	parse_config_file(config_filename, program_name);
    }
    
    /**
     * \brief Parses the command line arguments
     * \details This analyzes command line arguments passed to the main()
     * program in \p argc and \p argv. Arguments not matching program
     * options declared with declare_arg() are stored in output vector \p
     * unparsed_args.
     * \param[in] argc number of arguments passed to main()
     * \param[in] argv array of command line arguments passed to main()
     * \param[out] unparsed_args output vector of unparsed arguments
     * \retval true if the command line arguments are successfully parsed
     * \retval false otherwise
     */
    bool parse_internal(
        int argc, char** argv, std::vector<std::string>& unparsed_args
    ) {
	geo_argc = argc;
	geo_argv = argv;
	
	parse_config_file(argc, argv);
	
        bool ok = true;
        desc_->argv0 = argv[0];
        unparsed_args.clear();

        for(int i = 1; i < argc; i++) {
            std::vector<std::string> parsed_arg;
            String::split_string(argv[i], '=', parsed_arg);
            if(parsed_arg.size() != 2) {
                unparsed_args.push_back(argv[i]);
            } else {
                if(
                    String::string_starts_with(parsed_arg[0], "dbg:") ||
                    desc_->args.find(parsed_arg[0]) != desc_->args.end()
                ) {
                    if(!set_arg(parsed_arg[0], parsed_arg[1])) {
                        ok = false;
                    }
                } else {

                    std::vector<std::string> matches;
                    for( auto& it : desc_->args) {
                        if(arg_matches(parsed_arg[0], it.first)) {
                            matches.push_back(it.first);
                        }
                    }

                    if(matches.size() == 1) {
                        if(!set_arg(matches[0], parsed_arg[1])) {
                            ok = false;
                        }
                    } else if(matches.size() >= 2) {
                        ok = false;
                        Logger::instance()->set_quiet(false);
                        Logger::err("CmdLine")
                            << "Argument is ambiguous: " << argv[i]
                            << std::endl
                            << "Possible matches: "
                            << String::join_strings(matches, ' ')
                            << std::endl;
                    } else {
                        ok = false;
                        Logger::instance()->set_quiet(false);
                        Logger::err("CmdLine")
                            << "Invalid argument: " << parsed_arg[0]
                            << std::endl;
                    }
                }
            }
        }
        return ok;
    }

    /**
     * \brief Extracts the group name from an argument
     * \details If the command line argument \p name has the form
     * "group:arg", then "group" is returned, otherwise the argument is
     * considered to be global and the "global" group is returned.
     * \param[in] name the name of the argument
     * \return the group name or "global"
     */
    std::string arg_group(const std::string& name) {
        size_t pos = name.find(':');
        return pos == std::string::npos
               ? std::string("global")
               : name.substr(0, pos);
    }


    /**
     * \brief Gets an argument as a string for display.
     * \param[in] arg_name the name of the argument.
     * \return a string with the value of the argument for
     *  display.
     * \details It ignores least significant digits for floating
     *  point arguments.
     */
    std::string get_display_arg(const std::string& arg_name) {
	CmdLine::ArgType arg_type = get_arg_type(arg_name);
	std::string result;
	if(arg_type == CmdLine::ARG_DOUBLE) {
	    double x = CmdLine::get_arg_double(arg_name);
	    result = String::to_display_string(x);
	} else if(arg_type == CmdLine::ARG_PERCENT) {
	    double x = CmdLine::get_arg_percent(arg_name, 100.0);
	    result = String::to_display_string(x) + "%";
	} else {
	    result = CmdLine::get_arg(arg_name);
	    if(result.length() > ui_terminal_width()/2) {
                // TODO: fix display long lines in terminal
		// (that trigger infinite loop for now)
		result = "...";
	    }
	}
	return result;
    }
    
    /**
     * \brief Private data used for printing ArgGroup details
     */
    struct Line {
        std::string name;
        std::string value;
        std::string desc;
    };

    /**
     * \brief Prints arguments of a group
     * \details This function is called by show_usage() to display help on a
     * group of arguments. It displays a fancy box that summarizes all
     * arguments of the group \p group. By default, only standard arguments
     * are displayed. If parameter \p advanced is set to \c true, the function
     * also displays advanced arguments.
     * \param[in] group the group name
     * \param[in] advanced boolean flag that controls the display of
     * advanced arguments in the group.
     * \see show_usage()
     */
    void show_group(const std::string& group, bool advanced) {

        auto it = desc_->groups.find(group);
        if(it == desc_->groups.end()) {
            return;
        }

        const Group& g = it->second;
        bool advanced_group = g.flags & ARG_ADVANCED;
        if(!advanced && advanced_group) {
            return;
        }

        if(advanced_group) {
            ui_separator(g.desc, "*" + g.name);
        } else {
            ui_separator(g.desc, g.name);
        }

        // Step 1:
        // Build a vector of lines that contain the various elements to
        // print for the argument, and compute the maximum width of the
        // argument names followed by their default value. This will allow
        // us to align the closing paren of the default value.

        std::vector<Line> lines;
        index_t max_left_width = 0;

        for(size_t i = 0; i < g.args.size(); i++) {
            auto ita = desc_->args.find(g.args[i]);
            if(ita == desc_->args.end()) {
                continue;
            }

            const Arg& arg = ita->second;
            bool advanced_arg = arg.flags & ARG_ADVANCED;
            if(!advanced && advanced_arg) {
                continue;
            }

            const char* name_marker = (advanced_arg && !man_mode) ? "*" : " ";

            Line line;
            line.name = name_marker + arg.name;
            line.value = " (=" + get_display_arg(arg.name) + ") : ";
            line.desc = arg.desc;
            lines.push_back(line);

            max_left_width = std::max(
		index_t(line.name.length() + line.value.length()),
                max_left_width
            );
        }

        // Step 2:
        // Print the lines constructed in step 1 with default values
        // right aligned.

        for(size_t i = 0; i < lines.size(); i++) {
            const Line& line = lines[i];
            int value_width = int(max_left_width - line.name.length());
            std::ostringstream os;
            os << line.name
                << std::setw(value_width) << line.value
                << line.desc
                << std::endl;
            ui_message(os.str(), max_left_width);
            if(man_mode) {
                ui_message("\n");
            }
        }
    }

}

/****************************************************************************/

namespace GEO {

    namespace CmdLine {

        void initialize() {
            desc_ = new CommandLineDesc;
            declare_arg_group("global", "");
        }

        void terminate() {
            ui_close_separator();
            delete desc_;
            desc_ = nullptr;
        }

	int argc() {
	    return geo_argc;
	}

	char** argv() {
	    return geo_argv;
	}

	void set_config_file_name(const std::string& filename, bool auto_create) {
	    config_file_name = filename;
	    auto_create_args = auto_create;
	}

	std::string get_config_file_name() {
	    return config_file_name;
	}

	void load_config(
	    const std::string& filename, const std::string& program_name
	) {
	    parse_config_file(filename, program_name);
	}
	

	bool config_file_loaded() {
	    return loaded_config_file;
	}
	
        bool parse(
            int argc, char** argv, std::vector<std::string>& unparsed_args,
            const std::string& additional_arg_specs
        ) {
            if(!parse_internal(argc, argv, unparsed_args)) {
                return false;
            }

            if(arg_is_declared("profile")) {
                std::string profile = get_arg("profile");
                if(profile != "default") {
                    if(!set_profile(profile)) {
                        return false;
                    }
                    // Re-parse args to override values set by profiles
                    unparsed_args.clear();
                    parse_internal(argc, argv, unparsed_args);
                }
            }

            for(index_t i = 0; i < unparsed_args.size(); ++i) {
                const std::string& arg = unparsed_args[i];
                if(
                    arg == "-h" ||
                    arg == "-?" ||
                    arg == "/h" ||
                    arg == "/?"
                ) {
                    show_usage(additional_arg_specs, true);
                    exit(0);
                }
                if(arg == "--help") {
                    CmdLine::set_arg("log:pretty",false);
                    man_mode = true;
                    show_usage(additional_arg_specs, true);
                    exit(0);
                }
                if(arg == "--version" || arg == "--v") {
                    std::cout << FileSystem::base_name(argv[0])
                     << " "
                     << Environment::instance()->get_value("version")
                     << " (built "
                     << Environment::instance()->get_value(
                         "release_date")
                     << ")"
                     << std::endl
                     << "Copyright (C) 2006-2017"
                     << std::endl
                     << "The Geogram library used by this program is licensed"
                     << std::endl
                     << "under the 3-clauses BSD license."
                     << std::endl
                     << "Inria, the ALICE project"
                     << std::endl
                     << "   <http://alice.loria.fr/software/geogram>"
                     << std::endl
                     << "Report Geogram bugs to the geogram mailing list, see: "
                     << std::endl
                     << "   <https://gforge.inria.fr/mail/?group_id=5833>"
                     << std::endl;
                    exit(0);
                }
            }

            index_t min_unparsed = 0;
            index_t max_unparsed = 0;
            std::vector<std::string> additional_args;
            String::split_string(additional_arg_specs, ' ', additional_args);
            for(index_t i = 0; i < additional_args.size(); ++i) {
                const std::string& arg = additional_args[i];
                if(arg[0] == '<' && arg[arg.length() - 1] == '>') {
                    ++max_unparsed;
                } else if(
                    arg[0] == '<' &&
                    arg[arg.length() - 2] == '>' &&
                    arg[arg.length() - 1] == '*'
                ) {
                    min_unparsed=0;
                    max_unparsed=100000;
                } else {
                    ++max_unparsed;
                    ++min_unparsed;
                }
            }

            if(
                unparsed_args.size() > max_unparsed ||
                unparsed_args.size() < min_unparsed
            ) {
                show_usage(additional_arg_specs);
                return false;
            }

#ifndef GEOGRAM_PSM
	    nlPrintfFuncs(geogram_printf, geogram_fprintf);	    
	    nlInitialize(argc, argv);
#endif
	    if(
		CmdLine::arg_is_declared("nl:CUDA") &&
		CmdLine::get_arg_bool("nl:CUDA")
	    ) {
		geo_cite("DBLP:journals/paapp/BuatoisCL09");
	    }
	    
            return true;
        }

        bool parse(int argc, char** argv) {
            std::vector<std::string> unparsed_args;
            return parse(argc, argv, unparsed_args, "");
        }

        void declare_arg_group(
            const std::string& name,
            const std::string& description,
            ArgFlags flags
        ) {
            if(desc_->groups.find(name) != desc_->groups.end()) {
                Logger::err("CmdLine")
                    << "Group is multiply defined: " << name
                    << std::endl;
                return;
            }

            Group group;
            group.name = name;
            group.desc = description;
            group.flags = flags;
            desc_->groups[name] = group;
            desc_->group_names.push_back(name);
        }

        void declare_arg(
            const std::string& name,
            ArgType type,
            const std::string& default_value,
            const std::string& description,
            ArgFlags flags
        ) {
            if(desc_->args.find(name) != desc_->args.end()) {
                Logger::err("CmdLine")
                    << "Argument is multiply defined: " << name
                    << std::endl;
                return;
            }

            Arg arg;
            arg.name = name;
            arg.type = type;
            arg.desc = description;
            arg.flags = flags;
            desc_->args[name] = arg;

            Environment::instance()->set_value(name, default_value);

            std::string group = arg_group(name);
            auto it = desc_->groups.find(group);
            if(it == desc_->groups.end()) {
                Logger::err("CmdLine")
                    << "Argument group does not exist: " << name
                    << std::endl;
                return;
            }

            it->second.args.push_back(name);
        }

        ArgType get_arg_type(const std::string& name) {
            auto it = desc_->args.find(name);
            return it == desc_->args.end()
                   ? ARG_UNDEFINED
                   : it->second.type;
        }

        std::string get_arg(const std::string& name) {
            return Environment::instance()->get_value(name);
        }

        bool arg_is_declared(const std::string& name) {
            return get_arg_type(name) != ARG_UNDEFINED;
        }

        int get_arg_int(const std::string& name) {
            ArgType type = get_arg_type(name);
            geo_assert_arg_type(type, ARG_INT);
            return String::to_int(get_arg(name));
        }

        unsigned int get_arg_uint(const std::string& name) {
            ArgType type = get_arg_type(name);
            geo_assert_arg_type(type, ARG_INT);
            return String::to_uint(get_arg(name));
        }

        double get_arg_double(const std::string& name) {
            ArgType type = get_arg_type(name);
            geo_assert_arg_type(type, ARG_DOUBLE);
            return String::to_double(get_arg(name));
        }

        double get_arg_percent(
            const std::string& name, double reference
        ) {
            ArgType type = get_arg_type(name);
            geo_assert_arg_type(type, ARG_PERCENT);
            double result;
            std::string s = get_arg(name);
            if(s.length() > 0 && s[s.length() - 1] == '%') {
                s.resize(s.length() - 1);
                result = String::to_double(s) * reference * 0.01;
                Logger::out("CmdLine")
                    << "using " << name << "=" << result
                    << "(" << get_arg(name) << ")"
                    << std::endl;
            } else {
                result = String::to_double(s);
                Logger::out("CmdLine")
                    << "using " << name << "=" << result
                    << std::endl;
            }
            return result;
        }

        bool get_arg_bool(const std::string& name) {
            ArgType type = get_arg_type(name);
            geo_assert_arg_type(type, ARG_BOOL);
            return Environment::instance()->has_value(name) &&
                   String::to_bool(get_arg(name));
        }

        bool set_arg(
            const std::string& name, const std::string& value
        ) {
            if(!check_arg_value(name, value)) {
                return false;
            }
            Environment::instance()->set_value(name, value);
            return true;
        }

        void set_arg(const std::string& name, int value) {
            ArgType type = get_arg_type(name);
            geo_assert_arg_type(
                type, ARG_INT | ARG_DOUBLE | ARG_PERCENT | ARG_STRING
            );
            Environment::instance()->set_value(name, String::to_string(value));
        }

        void set_arg(const std::string& name, unsigned int value) {
            ArgType type = get_arg_type(name);
            geo_assert_arg_type(
                type, ARG_INT | ARG_DOUBLE | ARG_PERCENT | ARG_STRING
            );
            Environment::instance()->set_value(name, String::to_string(value));
        }

        void set_arg(const std::string& name, double value) {
            ArgType type = get_arg_type(name);
            geo_assert_arg_type(type, ARG_DOUBLE | ARG_PERCENT | ARG_STRING);
            Environment::instance()->set_value(name, String::to_string(value));
        }

        void set_arg(const std::string& name, bool value) {
            ArgType type = get_arg_type(name);
            geo_assert_arg_type(type, ARG_BOOL | ARG_STRING);
            Environment::instance()->set_value(name, String::to_string(value));
        }

        void set_arg_percent(const std::string& name, double value) {
            ArgType type = get_arg_type(name);
            geo_assert_arg_type(type, ARG_PERCENT | ARG_STRING);
            Environment::instance()->set_value(
                name, String::to_string(value) + "%"
            );
        }

        void show_usage(const std::string& additional_args, bool advanced) {
            std::string program_name = FileSystem::base_name(desc_->argv0);
            Logger::instance()->set_quiet(false);
            Logger::out("")
                << "Usage: " << program_name << " "
                << additional_args
                << " <parameter=value>*" << std::endl;
            if(!advanced) {
                Logger::out("")
                    << "Showing basic parameters (use " << program_name
                    << " -h to see advanced parameters)"
                    << std::endl;
            }

            for(auto& it : desc_->group_names) {
                show_group(it, advanced);
            }
        }

        void get_args(std::vector<std::string>& args) {
            args.clear();
            for(auto& it : desc_->args) {
                std::string cur_arg = it.first + "=" + get_arg(it.first);
		args.push_back(cur_arg);
            }
        }
    }
}

// =================== Console logging utilies =====================

namespace {

    using namespace GEO;
    using namespace CmdLine;

    /** Keeps length of a feature name */
    bool ui_separator_opened = false;

    /** Width of the current terminal */
    index_t ui_term_width = 79;

    /** Terminal left margin */
    index_t ui_left_margin = 0;

    /** Terminal right margin */
    index_t ui_right_margin = 0;

    /** Characters of the progress wheel */
    const char working[] = {'|', '/', '-', '\\'};

    /** Current index in the progress wheel */
    index_t working_index = 0;

    /** Characters of the progress wave */
    const char waves[] = {',', '.', 'o', 'O', '\'', 'O', 'o', '.', ','};

    /**
     * \brief Gets the console output stream
     * \return a reference to a std::ostream
     */
    inline std::ostream& ui_out() {
        return std::cout;
    }

    /**
     * \brief Prints a sequence of identical chars
     * \param[in] c the character to print
     * \param[in] nb the number of characters to print
     */
    inline void ui_pad(char c, size_t nb) {
        for(index_t i = 0; i < nb; i++) {
            std::cout << c;
        }
    }

    /**
     * \brief Checks if the standard output is redirected
     * \details It is used by the various console formatting functions to
     * determine if messages are printed in pretty mode or not.
     * \retval true if the console is redirected to a file
     * \retval false otherwise
     */
    bool is_redirected() {
        static bool initialized = false;
        static bool result;
        if(!initialized) {
#ifdef GEO_OS_WINDOWS
            result = !_isatty(1);
#else
            result = !isatty(1);
#endif
            initialized = true;
        }
        return result || !Logger::instance()->is_pretty();
    }

    /**
     * \brief Recomputes the width of the terminal
     */
    void update_ui_term_width() {
#ifdef GEO_OS_EMSCRIPTEN
        return; // ioctl not implemented under emscripten
#else
#ifndef GEO_OS_WINDOWS
        if(is_redirected()) {
            return;
        }
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        ui_term_width = w.ws_col;
        if(ui_term_width < 20) {
            ui_term_width = 79;
        }
        if(ui_term_width <= 82) {
            ui_left_margin = 0;
            ui_right_margin = 0;
        } else if(ui_term_width < 90) {
            ui_left_margin = 2;
            ui_right_margin = 2;
        } else {
            ui_left_margin = 4;
            ui_right_margin = 4;
        }
#endif
#endif        
    }

    /**
     * \brief Safe unsigned subtraction
     * \return a - b if a > b, 0 otherwise
     */
    inline size_t sub(size_t a, size_t b) {
        return a > b ? a - b : 0;
    }
}

namespace GEO {

    namespace CmdLine {

        index_t ui_terminal_width() {
            index_t ui_term_width_bkp = ui_term_width;
            update_ui_term_width();
            ui_term_width = std::min(ui_term_width, ui_term_width_bkp);
            return ui_term_width;
        }

        void ui_separator() {
            if(Logger::instance()->is_quiet() || is_redirected()) {
                return;
            }

            update_ui_term_width();
            ui_separator_opened = true;

            ui_out() << " ";
            ui_pad(' ', ui_left_margin);
            ui_pad(
                '_',
                sub(ui_terminal_width(), 2 + ui_left_margin + ui_right_margin)
            );
            ui_out() << " " << std::endl;

            // Force a blank line under the separator
            ui_message("\n");
        }

        void ui_separator(
            const std::string& title,
            const std::string& short_title
        ) {
            if(Logger::instance()->is_quiet()) {
                return;
            }

            if(man_mode) {
                if(title == "") {
                    return;
                }
                ui_out() << std::endl;
                std::string shortt = short_title;
                if(shortt.length() > 0 && shortt[0] == '*') {
                    shortt = shortt.substr(1, shortt.length()-1);
                    ui_out() << title << " (\"" << shortt << ":*\" options, advanced)"
                             << std::endl;
                } else {
                    ui_out() << title << " (\"" << shortt << ":*\" options)"
                             << std::endl;
                }
                ui_out() << std::endl << std::endl;                
                return;
            }
            
            if(is_redirected()) {
                ui_out() << std::endl;
                if(short_title != "" && title != "") {
                    ui_out() << "=[" << short_title << "]=["
                        << title << "]=" << std::endl;
                } else {
                    std::string s = title + short_title;
                    ui_out() << "=[" << s << "]=" << std::endl;
                }
                return;
            }

            update_ui_term_width();
            ui_separator_opened = true;

            size_t L = title.length() + short_title.length();

            ui_out() << "   ";
            ui_pad(' ', ui_left_margin);
            ui_pad('_', L + 14);
            ui_out() << std::endl;

            ui_pad(' ', ui_left_margin);
            if(short_title != "" && title != "") {
                ui_out() << " _/ ==[" << short_title << "]====["
                    << title << "]== \\";
            } else {
                std::string s = title + short_title;
                ui_out() << " _/ =====[" << s << "]===== \\";
            }

            ui_pad(
                '_',
                sub(
                    ui_terminal_width(),
                    19 + L + ui_left_margin + ui_right_margin
                )
            );
            ui_out() << std::endl;

            // Force a blank line under the separator
            ui_message("\n");
        }

        void ui_message(const std::string& message) {
            // By default, wrap to the column that is right after the feature
            // name and its decorations.
            ui_message(message, feature_max_length + 5);
        }

        void ui_message(
            const std::string& message,
            index_t wrap_margin
        ) {
            if(Logger::instance()->is_quiet()) {
                return;
            }

            if(!ui_separator_opened) {
                ui_separator();
            }

            if(is_redirected()) {
                ui_out() << message;
                return;
            }

            std::string cur = message;
            size_t maxL =
                sub(ui_terminal_width(), 4 + ui_left_margin + ui_right_margin);
            index_t wrap = 0;

            for(;;) {
                std::size_t newline = cur.find('\n');
                if(newline != std::string::npos && newline < maxL) {
                    // Got a new line that occurs before the right border
                    // We cut before the newline and pad with spaces
                    ui_pad(' ', ui_left_margin);
                    ui_out() << "| ";
                    ui_pad(' ', wrap);
                    ui_out() << cur.substr(0, newline);
                    ui_pad(' ', sub(maxL,newline));
                    ui_out() << " |" << std::endl;
                    cur = cur.substr(newline + 1);
                } else if(cur.length() > maxL) {
                    // The line length runs past the right border
                    // We cut the string just before the border
                    ui_pad(' ', ui_left_margin);
                    ui_out() << "| ";
                    ui_pad(' ', wrap);
                    ui_out() << cur.substr(0, maxL);
                    ui_out() << " |" << std::endl;
                    cur = cur.substr(maxL);
                } else if(cur.length() != 0) {
                    // Print the remaining portion of the string
                    // and pad with spaces
                    ui_pad(' ', ui_left_margin);
                    ui_out() << "| ";
                    ui_pad(' ', wrap);
                    ui_out() << cur;
                    ui_pad(' ', sub(maxL,cur.length()));
                    ui_out() << " |";
                    break;
                } else {
                    // No more chars to print
                    break;
                }

                if(wrap == 0) {
                    wrap = wrap_margin;
                    maxL = sub(maxL,wrap_margin);
                }
            }
        }

        void ui_clear_line() {
            if(Logger::instance()->is_quiet() || is_redirected()) {
                return;
            }

            ui_pad('\b', ui_terminal_width());
            ui_out() << std::flush;
        }

        void ui_close_separator() {
            if(!ui_separator_opened) {
                return;
            }

            if(Logger::instance()->is_quiet() || is_redirected()) {
                return;
            }

            ui_pad(' ', ui_left_margin);
            ui_out() << '\\';
            ui_pad(
                '_',
                sub(ui_terminal_width(), 2 + ui_left_margin + ui_right_margin)
            );
            ui_out() << '/';
            ui_out() << std::endl;

            ui_separator_opened = false;
        }

        void ui_progress(
            const std::string& task_name, index_t val, index_t percent,
            bool clear
        ) {
            if(Logger::instance()->is_quiet() || is_redirected()) {
                return;
            }

            working_index++;

            std::ostringstream os;

            if(percent != val) {
                os << ui_feature(task_name)
                   << "("
                   << working[(working_index % sizeof(working))]
                   << ")-["
                   << std::setw(3) << percent
                   << "%]-["
                   << std::setw(3) << val
                   << "]--[";
            } else {
                os << ui_feature(task_name)
                   << "("
                   << working[(working_index % sizeof(working))]
                   << ")-["
                   << std::setw(3) << percent
                   << "%]--------[";
            }
                
            size_t max_L =
                sub(ui_terminal_width(), 43 + ui_left_margin + ui_right_margin);

            if(val > max_L) {
                // No space enough to expand the progress bar
                // Do some animation...
                for(index_t i = 0; i < max_L; i++) {
                    os << waves[((val - i + working_index) % sizeof(waves))];
                }
            } else {
                for(index_t i = 0; i < val; i++) {
                    os << "o";
                }
            }
            os << " ]";

            if(clear) {
                ui_clear_line();
            }
            ui_message(os.str());
        }

        void ui_progress_time(
            const std::string& task_name, double elapsed, bool clear
        ) {
            if(Logger::instance()->is_quiet()) {
                return;
            }

            std::ostringstream os;
            os << ui_feature(task_name)
                << "Elapsed time: " << elapsed
                << "s\n";

            if(clear) {
                ui_clear_line();
            }
            ui_message(os.str());
        }

        void ui_progress_canceled(
            const std::string& task_name,
            double elapsed, index_t percent, bool clear
        ) {
            if(Logger::instance()->is_quiet()) {
                return;
            }

            std::ostringstream os;
            os << ui_feature(task_name)
                << "Task canceled after " << elapsed
                << "s (" << percent << "%)\n";

            if(clear) {
                ui_clear_line();
            }
            ui_message(os.str());
        }

        std::string ui_feature(
            const std::string& feat_in, bool show
        ) {
            if(feat_in.empty()) {
                return feat_in;
            }

            if(!show) {
                return std::string(feature_max_length + 5, ' ');
            }

            std::string result = feat_in;
            if(!is_redirected()) {
                result = result.substr(0, feature_max_length);
            }
            if(result.length() < feature_max_length) {
                result.append(feature_max_length - result.length(), ' ');
            }
            return "o-[" + result + "] ";
        }
    }
}

#ifdef GEO_OS_ANDROID
namespace {
    android_app* android_app_ = nullptr;
}

namespace GEO {
    namespace CmdLine {
	void set_android_app(android_app* app) {
	    android_app_ = app;
	}
	
	android_app* get_android_app() {
	    return android_app_;
	}
    }
}
#endif

