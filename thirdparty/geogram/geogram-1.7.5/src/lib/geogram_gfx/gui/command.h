/*
 *  Copyright (c) 2012-2016, Bruno Levy
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

#ifndef GEOGRAM_GFX_GUI_COMMAND
#define GEOGRAM_GFX_GUI_COMMAND

#include <geogram_gfx/basic/common.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/string.h>

/**
 * \file geogram_gfx/gui/command.h
 * \brief A simple system to interface C++ functions 
 *   with a ImGui interface. Used mainly by geobox.
 */

namespace GEO {

    /**
     * \brief Abstract class for calling functions or 
     *  calling member functions.
     * \details Used internally by Command.
     */
    class GEOGRAM_GFX_API CommandInvoker : public Counted {
    public:
        /**
         * \brief CommandInvoker constructor.
         */
        CommandInvoker();

        /**
         * \brief CommandInvoker destructor.
         */
        virtual ~CommandInvoker();

        /**
         * \brief Invokes the target function.
         */
        virtual void invoke() = 0;

        /**
         * \brief Creates the arguments in the target
         *  command.
         * \details This function is used when client code
         *  did not provide a function prototype to the
         *  constructor of Command. 
         */
        virtual void auto_create_args() = 0;
    };

    /**
     * \brief Automatic reference-counted pointer to a CommandInvoker.
     * \details Used internally by Command.
     */
    typedef SmartPointer<CommandInvoker> CommandInvoker_var;
    
    /*****************************************************************/

    /**
     * \brief Manages the GUI of a command with ImGUI.
     * \details Client code will only need to use set_current()
     */
    class GEOGRAM_GFX_API Command : public Counted {
    public:

        /**
         * \brief Binds the current command to a function.
         * \details This makes the command dialog box display 
         *  parameters that correspond to the arguments of the function,
         *  and whenever the 'apply' button is pushed, the function is
         *  invoked with the arguments.
         *  Example:
         *  \code
         *     void my_command_impl(float x, float y, bool normalize) {
         *     ... do something
         *     }
         *     ...
         *     ... 
         *     if(ImGui::MenuItem("my command")) {
         *         GEO::Command::set_current(
         *            "void my_command_impl(float x, float y, bool normalize)",
         *            &my_command_impl
         *         )
         *     }
         *  \endcode
         *  The first argument (the string with the function prototype) is 
         *  necessary to retrieve the names of the parameters. In addition,
         *  default values and tooltips may be specified, as follows:
         *  \code
         *         GEO::Command::set_current(
         *            "void my_command_impl(                        "
         *            "   float x=0 [this is the x coordinate],     "
         *            "   float y=0 [this is the y coordinate],     "
         *            "   bool normalize=true [normalize the vector]"
         *            ") [does something with a vector]             ",
         *            &my_command_impl
         *         )
         *  \endcode
         *  Each text in the square brackets corresponds to a tooltip attached
         *  to an argument. There can be also one for the function. The default
         *  values are those that are obtained at initialization, or those that
         *  are set when the 'default' button is pushed.
         * \tparam FPTR function pointer type
         * \param[in] prototype a string with the prototype of the function,
         *  as written in C++. In addition, the function and each parameter 
         *  can be documented in square brackets.
         * \param[in] tfun the function pointer.
         */
        template<class FPTR> static void set_current(
            const std::string& prototype, FPTR tfun
        );
        
        /**
         * \brief Binds the current command to a member function
         *  of an object.
         * \details This makes the command dialog box display 
         *  parameters that correspond to the arguments of the function,
         *  and whenever the 'apply' button is pushed, the function is
         *  invoked with the arguments.
         *  Example:
         *  \code
         *     class MyCommands {
         *        void my_command_impl(float x, float y, bool normalize) {
         *        ... do something
         *        }
         *     };
         *     MyCommands my_commands;
         *     ...
         *     ... 
         *     if(ImGui::MenuItem("my command")) {
         *         GEO::Command::set_current(
         *            "void my_command_impl(float x, float y, bool normalize)",
         *            &my_commands, &MyCommands::my_command_impl
         *         )
         *     }
         *  \endcode
         *  The first argument (the string with the function prototype) is 
         *  necessary to retrieve the names of the parameters. In addition,
         *  default values and tooltips may be specified, as follows:
         *  \code
         *         GEO::Command::set_current(
         *            "void my_command_impl(                        "
         *            "   float x=0 [this is the x coordinate],     "
         *            "   float y=0 [this is the y coordinate],     "
         *            "   bool normalize=true [normalize the vector]"
         *            ") [does something with a vector]             ",
         *            &my_commands, &MyCommands::my_command_impl
         *         )
         *  \endcode
         *  Each text in the square brackets corresponds to a tooltip attached
         *  to an argument. There can be also one for the function. The default
         *  values are those that are obtained at initialization, or those that
         *  are set when the 'default' button is pushed.
         * \tparam T object class
         * \tparam TFPTR function pointer type, should be a member function
         *  of class T
         * \param[in] prototype a string with the prototype of the function,
         *  as written in C++. In addition, the function and each parameter 
         *  can be documented in square brackets.
         * \param[in] target a pointer to the object, of class T
         * \param[in] tfun the pointer to the member function
         */
        template<class T, class TFPTR> static void set_current(
            const std::string& prototype, T* target, TFPTR tfun
        );

        /**
         * \brief Flushes the potentially queued command invokation.
         * \details When the user pushes the 'apply' button, the command
         *  is not invoked immediatly, because we are still in the ImGUI
         *  handling function. This function is called by the framework
         *  at the end of the frame, when the ImGUI handler is already
         *  finished. It can potentially re-trigger frame rendering operations,
         *  through the Logger and ProgressLogger. Without this mechanism,
         *  it would nest two ImGUI handlers, which is not allowed.
         */
        static void flush_queue();
        
        /**
         * \brief Command constructor.
         * \param[in] prototype a const reference to a string with
         *  the prototype of the function that implements the callback, 
         *  as declared in the C++ sources.
         * \note Regular client code should not need to use this function.
         */
        Command(const std::string& prototype);


        /**
         * \brief Gets the name of this command.
         * \return a const reference to the name of this command.
         */
        const std::string& name() const {
            return name_;
        }
        
        /**
         * \brief Sets the invoker.
         * \details The invoker is used internally to transmit the stored
         *  arguments to the parameters of a function. 
         * \param[in] invoker a pointer to the CommandInvoker. Ownership
         *  is transferred to this Command.
         * \note Regular client code should not need to use this function.
         */
        void set_invoker(CommandInvoker* invoker) {
            invoker_ = invoker;
            if(auto_create_args_) {
                invoker_->auto_create_args();
                auto_create_args_ = false;
            }
        }
        
        /**
         * \brief Command destructor.
         */
        virtual ~Command();

        /**
         * \brief Tests whether this Command is visible.
         * \retval true if this Command is visible
         * \retval false otherwise
         */
        bool is_visible() const {
            return visible_;
        }

        /**
         * \brief Gets a pointer to the visibility flag of
         *   this command.
         * \return a pointer to the visibility flag
         */
        bool* is_visible_ptr() {
            return &visible_;
        }
        
        /**
         * \brief Displays and manages the GUI of this 
         *  Command.
         * \note Regular client code should not need to use this function.
         */
        virtual void draw();

        /**
         * \brief Restores default parameter values for
         *  all parameters.
         * \details This is the function that is called when the user pushes
         *  the 'default' button.
         * \note Regular client code should not need to use this function.
         */
        virtual void reset_factory_settings();

        /**
         * \brief Gets the value of the parameters and
         *  does the task. 
         * \details This is the function that is called when the user pushes
         *  the 'apply' button. It does not invoke the command immediatly,
         *  the command invocation is queued, and executed later by
         *  Command::flush_queue(), once we are no longer in the ImGui 
         *  handler (else we would have two nested ImGui handlers, which is
         *  not correct).
         * \note Regular client code should not need to use this function.
         */
        virtual void apply() ;

        /**
         * \brief Gets the current command.
         * \return a pointer to the current command
         * \note Regular client code should not need to use this function.
         */
        static Command* current() {
            return current_;
        }

        /**
         * \brief Resets the current command.
         */
        static void reset_current() {
            current_.reset();
        }
        
        /**
         * \brief Sets the current command.
         * \param[in] command a pointer to the command
         *  to be set as current
         * \note Regular client code should not need to use this function.
         */
        static void set_current(Command* command) {
            current_ = command;
            command->visible_ = true;
        }

        /**
         * \brief Gets the value of a boolean argument by index.
         * \details If the index is out of range, or if the
         *  argument is not of the correct type, then an assertion
         *  failure is triggered.
         * \param[in] i the index of the argument
         * \return the value of the argument
         * \note Regular client code should not need to use this function.
         */
        bool bool_arg_by_index(index_t i) const {
            const Arg& arg = find_arg_by_index(i);
            geo_assert(arg.type == Arg::ARG_BOOL);
            return arg.val.bool_val;
        }

        /**
         * \brief Gets the value of an integer argument by index.
         * \details If the index is out of range, or if the
         *  argument is not of the correct type, then an assertion
         *  failure is triggered.
         * \param[in] i the index of the argument
         * \return the value of the argument
         * \note Regular client code should not need to use this function.
         */
        int int_arg_by_index(index_t i) const;

        /**
         * \brief Gets the value of an unsigned integer argument by index.
         * \details If the index is out of range, or if the
         *  argument is not of the correct type, then an assertion
         *  failure is triggered. If the stored value is negative, then
         *  it is clamped to 0, and a warning message is displayed on
         *  the console.
         * \param[in] i the index of the argument
         * \return the value of the argument
         * \note Regular client code should not need to use this function.
         */
        unsigned int uint_arg_by_index(index_t i) const; 

        /**
         * \brief Gets the value of a floating-point argument by index.
         * \details If the index is out of range, or if the
         *  argument is not of the correct type, then an assertion
         *  failure is triggered.
         * \param[in] i the index of the argument
         * \return the value of the argument
         * \note Regular client code should not need to use this function.
         */
        float float_arg_by_index(index_t i) const {
            const Arg& arg = find_arg_by_index(i);
            geo_assert(arg.type == Arg::ARG_FLOAT);
            return arg.val.float_val;
        }

        /**
         * \brief Gets the value of a floating-point argument by index
         *  and converts it to a double.
         * \details If the index is out of range, or if the
         *  argument is not of the correct type, then an assertion
         *  failure is triggered.
         * \param[in] i the index of the argument
         * \return the value of the argument
         * \note Regular client code should not need to use this function.
         */
        double double_arg_by_index(index_t i) const {
            return double(float_arg_by_index(i));
        }

        /**
         * \brief Gets the value of a string argument by index.
         * \details If the index is out of range, or if the
         *  argument is not of the correct type, then an assertion
         *  failure is triggered.
         * \param[in] i the index of the argument
         * \return the value of the argument
         * \note Regular client code should not need to use this function.
         */
        std::string string_arg_by_index(index_t i) const {
            const Arg& arg = find_arg_by_index(i);
            geo_assert(arg.type == Arg::ARG_STRING);
            return std::string(arg.val.string_val);
        }

        /**
         * \brief Gets the value of an argument by index.
         * \details This function is generic, and has several
         *  specializations for bool, int, unsigned int, float, double and
         *  std::string. For all other types, an assertion failure is 
         *  triggered.
         * \tparam T type of the argument
         * \param[in] i the index of the argument
         * \param[out] val a reference to the argument
         * \note Regular client code should not need to use this function.
         */
        template<class T> void get_arg_by_index(index_t i, T& val) {
            geo_argused(val);
            Logger::err("Cmd") << "Attempted to read argument #"
                               << i
                               << " to variable of unknown type"
                               << std::endl;
            geo_assert_not_reached;
        }

        /***************************************************************/

        /**
         * \brief Invokes a member function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam T class of the target object
         * \param[in] target the target object
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        template <class T> void invoke(
            T* target, void (T::*fptr)(void)
        ) {
            this->assert_nb_args_matches(0);
            if(target != nullptr && fptr != nullptr) {
                (*target.*fptr)();
            }
        }

        /**
         * \brief Invokes a member function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam T class of the target object
         * \tparam ARG0 type of the argument
         * \param[in] target the target object
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        template <
            class T,
            class ARG0
        > void invoke(
            T* target,
            void (T::*fptr)(ARG0)
        ) {
            this->assert_nb_args_matches(1);            
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            if(target != nullptr && fptr != nullptr) {            
                (*target.*fptr)(a0);
            }
        }

        /**
         * \brief Invokes a member function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam T class of the target object
         * \tparam ARG0 , ARG1 types of the arguments
         * \param[in] target the target object
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        template <
            class T,
            class ARG0, class ARG1
        > void invoke(
            T* target,
            void (T::*fptr)(ARG0,ARG1)
        ) {
            this->assert_nb_args_matches(2);                        
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            if(target != nullptr && fptr != nullptr) {
                (*target.*fptr)(a0,a1);
            }
        }

        /**
         * \brief Invokes a member function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam T class of the target object
         * \tparam ARG0 , ARG1 , ARG2 types of the arguments
         * \param[in] target the target object
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class T,
            class ARG0, class ARG1, class ARG2
        > void invoke(
            T* target,
            void (T::*fptr)(ARG0,ARG1,ARG2)
        ) {
            this->assert_nb_args_matches(3);                                    
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            if(target != nullptr && fptr != nullptr) {            
                (*target.*fptr)(a0,a1,a2);
            }
        }

        /**
         * \brief Invokes a member function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam T class of the target object
         * \tparam ARG0 , ARG1 , ARG2 , ARG3 types of the arguments
         * \param[in] target the target object
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class T,
            class ARG0, class ARG1, class ARG2, class ARG3
        > void invoke(
            T* target,
            void (T::*fptr)(ARG0,ARG1,ARG2,ARG3)
        ) {
            this->assert_nb_args_matches(4);            
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            ARG3 a3;
            this->get_arg_by_index(3,a3);
            if(target != nullptr && fptr != nullptr) {
                (*target.*fptr)(a0,a1,a2,a3);
            }
        }

        /**
         * \brief Invokes a member function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam T class of the target object
         * \tparam ARG0 , ARG4 types of the arguments
         * \param[in] target the target object
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class T,
            class ARG0, class ARG1, class ARG2, class ARG3,
            class ARG4
        > void invoke(
            T* target,
            void (T::*fptr)(ARG0,ARG1,ARG2,ARG3,ARG4)
        ) {
            this->assert_nb_args_matches(5);                        
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            ARG3 a3;
            this->get_arg_by_index(3,a3);
            ARG4 a4;
            this->get_arg_by_index(4,a4);
            if(target != nullptr && fptr != nullptr) {
                (*target.*fptr)(a0,a1,a2,a3,a4);
            }
        }

        /**
         * \brief Invokes a member function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam T class of the target object
         * \tparam ARG0 , ARG5 types of the arguments
         * \param[in] target the target object
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class T,
            class ARG0, class ARG1, class ARG2, class ARG3,
            class ARG4, class ARG5
        > void invoke(
            T* target,
            void (T::*fptr)(ARG0,ARG1,ARG2,ARG3,ARG4,ARG5)
        ) {
            this->assert_nb_args_matches(6);                                    
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            ARG3 a3;
            this->get_arg_by_index(3,a3);
            ARG4 a4;
            this->get_arg_by_index(4,a4);
            ARG5 a5;
            this->get_arg_by_index(5,a5);
            if(target != nullptr && fptr != nullptr) {
                (*target.*fptr)(a0,a1,a2,a3,a4,a5);
            }
        }

        /**
         * \brief Invokes a member function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam T class of the target object
         * \tparam ARG0 , ARG6 types of the arguments
         * \param[in] target the target object
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class T,
            class ARG0, class ARG1, class ARG2, class ARG3,
            class ARG4, class ARG5, class ARG6
        > void invoke(
            T* target,
            void (T::*fptr)(ARG0,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6)
        ) {
            this->assert_nb_args_matches(7);            
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            ARG3 a3;
            this->get_arg_by_index(3,a3);
            ARG4 a4;
            this->get_arg_by_index(4,a4);
            ARG5 a5;
            this->get_arg_by_index(5,a5);
            ARG6 a6;
            this->get_arg_by_index(6,a6);
            if(target != nullptr && fptr != nullptr) {
                (*target.*fptr)(a0,a1,a2,a3,a4,a5,a6);
            }
        }

        /**
         * \brief Invokes a member function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam T class of the target object
         * \tparam ARG0 , ARG7 types of the arguments
         * \param[in] target the target object
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class T,
            class ARG0, class ARG1, class ARG2, class ARG3,
            class ARG4, class ARG5, class ARG6, class ARG7
        > void invoke(
            T* target,
            void (T::*fptr)(ARG0,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7)
        ) {
            this->assert_nb_args_matches(8);                        
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            ARG3 a3;
            this->get_arg_by_index(3,a3);
            ARG4 a4;
            this->get_arg_by_index(4,a4);
            ARG5 a5;
            this->get_arg_by_index(5,a5);
            ARG6 a6;
            this->get_arg_by_index(6,a6);
            ARG7 a7;
            this->get_arg_by_index(7,a7);
            if(target != nullptr && fptr != nullptr) {
                (*target.*fptr)(a0,a1,a2,a3,a4,a5,a6,a7);
            }
        }

        /**************************************************************/

        /**
         * \brief Invokes a function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        void invoke(
            void (*fptr)(void)
        ) {
            this->assert_nb_args_matches(0);                                    
            if(fptr != nullptr) {
                (*fptr)();
            }
        }

        /**
         * \brief Invokes a function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam ARG0 type of the argument
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class ARG0
        > void invoke(
            void (*fptr)(ARG0)
        ) {
            this->assert_nb_args_matches(1);            
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            if(fptr != nullptr) {
                (*fptr)(a0);
            }
        }

        /**
         * \brief Invokes a function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam ARG0 , ARG1 types of the arguments
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class ARG0, class ARG1
        > void invoke(
            void (*fptr)(ARG0,ARG1)
        ) {
            this->assert_nb_args_matches(2);                        
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            if(fptr != nullptr) {
                (*fptr)(a0,a1);
            }
        }

        /**
         * \brief Invokes a function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam ARG0 , ARG1 , ARG2 types of the arguments
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class ARG0, class ARG1, class ARG2
        > void invoke(
            void (*fptr)(ARG0,ARG1,ARG2)
        ) {
            this->assert_nb_args_matches(3);                                    
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            if(fptr != nullptr) {
                (*fptr)(a0,a1,a2);
            }
        }

        /**
         * \brief Invokes a function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam ARG0 , ARG1 , ARG2 , ARG3 types of the arguments
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class ARG0, class ARG1, class ARG2, class ARG3
        > void invoke(
            void (*fptr)(ARG0,ARG1,ARG2,ARG3)
        ) {
            this->assert_nb_args_matches(4);             
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            ARG3 a3;
            this->get_arg_by_index(3,a3);
            if(fptr != nullptr) {
                (*fptr)(a0,a1,a2,a3);
            }
        }

        /**
         * \brief Invokes a function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam ARG0 , ARG1 , ARG2 , ARG3 , ARG4 types of the arguments
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class ARG0, class ARG1, class ARG2, class ARG3,
            class ARG4
        > void invoke(
            void (*fptr)(ARG0,ARG1,ARG2,ARG3,ARG4)
        ) {
            this->assert_nb_args_matches(5);                         
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            ARG3 a3;
            this->get_arg_by_index(3,a3);
            ARG4 a4;
            this->get_arg_by_index(4,a4);
            if(fptr != nullptr) {
                (*fptr)(a0,a1,a2,a3,a4);
            }
        }

        /**
         * \brief Invokes a function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam ARG0 , ARG5 types of the arguments
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class ARG0, class ARG1, class ARG2, class ARG3,
            class ARG4, class ARG5
        > void invoke(
            void (*fptr)(ARG0,ARG1,ARG2,ARG3,ARG4,ARG5)
        ) {
            this->assert_nb_args_matches(6);             
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            ARG3 a3;
            this->get_arg_by_index(3,a3);
            ARG4 a4;
            this->get_arg_by_index(4,a4);
            ARG5 a5;
            this->get_arg_by_index(5,a5);
            if(fptr != nullptr) {
                (*fptr)(a0,a1,a2,a3,a4,a5);
            }
        }

        /**
         * \brief Invokes a function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam ARG0 , ARG6 types of the arguments
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class ARG0, class ARG1, class ARG2, class ARG3,
            class ARG4, class ARG5, class ARG6
        > void invoke(
            void (*fptr)(ARG0,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6)
        ) {
            this->assert_nb_args_matches(7);                         
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            ARG3 a3;
            this->get_arg_by_index(3,a3);
            ARG4 a4;
            this->get_arg_by_index(4,a4);
            ARG5 a5;
            this->get_arg_by_index(5,a5);
            ARG6 a6;
            this->get_arg_by_index(6,a6);
            if(fptr != nullptr) {
                (*fptr)(a0,a1,a2,a3,a4,a5,a6);
            }
        }

        /**
         * \brief Invokes a function with the stored arguments.
         * \details If the type or number of stored arguments do not match
         *  the function pointer, then an assertion failure is triggered.
         * \tparam ARG0 , ARG7 types of the arguments
         * \param[in] fptr the pointer to the member function to be called.
         * \note Used by internal CommandInvoker mechanism.
         */
        
        template <
            class ARG0, class ARG1, class ARG2, class ARG3,
            class ARG4, class ARG5, class ARG6, class ARG7
        > void invoke(
            void (*fptr)(ARG0,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7)
        ) {
            this->assert_nb_args_matches(8);             
            ARG0 a0;
            this->get_arg_by_index(0,a0);
            ARG1 a1;
            this->get_arg_by_index(1,a1);
            ARG2 a2;
            this->get_arg_by_index(2,a2);
            ARG3 a3;
            this->get_arg_by_index(3,a3);
            ARG4 a4;
            this->get_arg_by_index(4,a4);
            ARG5 a5;
            this->get_arg_by_index(5,a5);
            ARG6 a6;
            this->get_arg_by_index(6,a6);
            ARG7 a7;
            this->get_arg_by_index(7,a7);
            if(fptr != nullptr) {
                (*fptr)(a0,a1,a2,a3,a4,a5,a6,a7);
            }
        }

        /**************************************************************/
        
    protected:

        /**
         * \brief Tests whether the number of declared arguments
         *  matches a specified number.
         * \details If the number of arguments differs from the expected
         *  number, then an assertion failure is triggered. 
         *  When auto_create_args_ is set, number of stored arguments should
         *  be 0.
         * \param[in] nb expected number of arguments.
         */
        void assert_nb_args_matches(index_t nb) {
            geo_assert(
                (auto_create_args_ && args_.size() == 0) ||
                (args_.size() == nb)
            );
        }

        
        /**
         * \brief Adds a parameter to this command
         * \tparam T type of the parameter, deduced from
         *  \p default_val
         * \param[in] name name of the parameter
         * \param[in] default_val default value of the parameter
         * \param[in] help optionnal text, displayed in
         *  a tooltip
         */
        template<class T> void add_arg(
            const std::string& name, const T& default_val,
            const std::string& help = ""
        ) {
            args_.push_back(Arg(name, default_val, help));
        }

        /**
         * \brief Creates an argument at a given index
         * \details Used by the auto_create_args mechanism, used
         *  when client code did not provide any function prototype
         * \tparam T type of the parameter, deduced from
         *  \p default_val
         * \param[in] default_val default value of the parameter
         */
        template<class T> void create_arg(
            index_t i, const T& default_val
        ) {
            if(i >= args_.size()) {
                args_.resize(i+1);
            }
            args_[i] = Arg("arg " + String::to_string(i), default_val);
        }

	static void set_queued(Command* command) {
	    queued_ = command;
	}
	
    private:
        
        /**
         * \brief Internal representation of an argument
         *   value.
         */
        struct GEOGRAM_GFX_API ArgVal {
            /**
             * \brief Resets all values stored to zero.
             */
            void clear();

            /**
             * \brief Argval constructor.
             */
            ArgVal() {
                clear();
            }

            /**
             * \brief Argval copy constructor.
             * \param[in] rhs a const reference to the ArgVal
             *  to be copied
             */
            ArgVal(const ArgVal& rhs);

            /**
             * \brief Argval assignment operator.
             * \param[in] rhs a const reference to the ArgVal
             *  to be copied
             * \return a reference to this ArgVal after assignment
             */
            ArgVal& operator=(const ArgVal& rhs);
            
            bool bool_val;
            int int_val;
            float float_val;
            char string_val[64];
        };

        /**
         * \brief Internal representation of an argument.
         * \details An argument has a type, a name, a
         *  default value, and an optionnal tooltip.
         */
        struct GEOGRAM_GFX_API Arg {

            /**
             * \brief Arg default constructor.
             */
            Arg();
            
            /**
             * \brief Arg constructor from bool.
             * \param[in] name_in the name of the argument
             * \param[in] x the default value
             * \param[in] help_in an optional text, displayed
             *  as a tooltip
             */
            Arg(
                const std::string& name_in, bool x,
                const std::string& help_in=""
            );

            /**
             * \brief Arg constructor from int.
             * \param[in] name_in the name of the argument
             * \param[in] x the default value
             * \param[in] help_in an optional text, displayed
             *  as a tooltip
             */
            Arg(
                const std::string& name_in, int x,
                const std::string& help_in=""
            );

            /**
             * \brief Arg constructor from unsigned int.
             * \details Stored internally as (signed) int
             * \param[in] name_in the name of the argument
             * \param[in] x the default value
             * \param[in] help_in an optional text, displayed
             *  as a tooltip
             */
            Arg(
                const std::string& name_in, unsigned int x,
                const std::string& help_in=""
            );

            /**
             * \brief Arg constructor from float.
             * \param[in] name_in the name of the argument
             * \param[in] x the default value
             * \param[in] help_in an optional text, displayed
             *  as a tooltip
             */
            Arg(
                const std::string& name_in, float x,
                const std::string& help_in=""                
            );

            /**
             * \brief Arg constructor from double.
             * \details Stored internally as a float
             * \param[in] name_in the name of the argument
             * \param[in] x the default value
             * \param[in] help_in an optional text, displayed
             *  as a tooltip
             */
            Arg(
                const std::string& name_in, double x,
                const std::string& help_in=""                                
            );

            /**
             * \brief Arg constructor from string.
             * \details String length is limited to 64 characters.
             * \param[in] name_in the name of the argument
             * \param[in] x the default value
             * \param[in] help_in an optional text, displayed
             *  as a tooltip
             */
            Arg(
                const std::string& name_in, const std::string& x,
                const std::string& help_in=""
            );

            /**
             * \brief Displays and manages the GUI for this Arg.
             */
            void draw();
            
            enum { ARG_BOOL, ARG_INT, ARG_UINT, ARG_FLOAT, ARG_STRING } type;
            std::string name;
            std::string help;
            ArgVal val;
            ArgVal default_val;
        };

        /**
         * \brief Finds an argument value by name.
         * \details Triggers an assertion failure if no such
         *  argument exists.
         * \param[in] name name of the argument
         * \return a const reference to the Arg
         */
        const Arg& find_arg(const std::string& name) const {
            for(index_t i=0; i<args_.size(); ++i) {
                if(args_[i].name == name) {
                    return args_[i];
                }
            }
            geo_assert_not_reached;
        }

        /**
         * \brief Gets an Arg by its index.
         * \param[in] i the index of the Arg
         * \return a const reference to the Arg
         */
        const Arg& find_arg_by_index(index_t i) const {
            geo_assert(i < args_.size());
            return args_[i];
        }
        
    private:
        std::string name_;
        std::string help_;
        vector<Arg> args_;
        CommandInvoker_var invoker_;
        bool visible_;
        /**
         * \brief If no prototype was specified, then
         *  arguments are automatically created.
         */
        bool auto_create_args_; 
        static SmartPointer<Command> current_;
        static SmartPointer<Command> queued_;
    };

/***********************************************************************/

    /**
     * \copydoc Command::get_arg_by_index()
     */
    template<> inline void Command::get_arg_by_index(
        index_t i, bool& val
    ) {
        if(auto_create_args_) {
            val = false;
            this->create_arg(i, val);
        } else {
            val = this->bool_arg_by_index(i);
        }
    }

    /**
     * \copydoc Command::get_arg_by_index()
     */
    template<> inline void Command::get_arg_by_index(
        index_t i, int& val
    ) {
        if(auto_create_args_) {
            val = 0;
            this->create_arg(i, val);
        } else {        
            val = this->int_arg_by_index(i);
        }
    }

    /**
     * \copydoc Command::get_arg_by_index()
     */
    template<> inline void Command::get_arg_by_index(
        index_t i, unsigned int& val
    ) {
        if(auto_create_args_) {
            val = 0;
            this->create_arg(i, val);
        } else {
            val = this->uint_arg_by_index(i);
        }
    }

    /**
     * \copydoc Command::get_arg_by_index()
     */
    template<> inline void Command::get_arg_by_index(
        index_t i, float& val
    ) {
        if(auto_create_args_) {
            val = 0.0f;
            this->create_arg(i, val);
        } else {
            val = this->float_arg_by_index(i);
        }
    }

    /**
     * \copydoc Command::get_arg_by_index()
     */
    template<> inline void Command::get_arg_by_index(
        index_t i, double& val
    ) {
        if(auto_create_args_) {
            val = 0.0;
            this->create_arg(i, val);
        } else {        
            val = this->double_arg_by_index(i);
        }
    }

    /**
     * \copydoc Command::get_arg_by_index()
     */
    template<> inline void Command::get_arg_by_index(
        index_t i, std::string& val
    ) {
        if(auto_create_args_) {
            val = "";
            this->create_arg(i, val);
        } else {                
            val = this->string_arg_by_index(i);
        }
    }

    /*****************************************************************/

    /**
     * \brief An implementation of CommandInvoker that calls 
     *  a function.
     * \tparam FPTR function pointer type for the function to be called
     */
    template <class FPTR>
    class FunctionCommandInvoker : public CommandInvoker {
    public:

        /**
         * \brief FunctionCommandInvoker constructor.
         * \param[in] command a pointer to the Command object
         * \param[in] fun the function pointer
         */
        FunctionCommandInvoker(
            Command* command,
            FPTR fun
        ) :
            command_(command),
            fun_(fun) {
        }

        /**
         * \copydoc CommandInvoker::invoke()
         */
        virtual void invoke() {
            command_->invoke(fun_);
        }

        /**
         * \copydoc CommandInvoker::auto_create_args()
         */
        virtual void auto_create_args() {
            command_->invoke(FPTR(nullptr));            
        }
        
    private:
        Command* command_;
        FPTR fun_; 
    };

    /*****************************************************************/

    /**
     * \brief An implementation of CommandInvoker that calls 
     *  a member function of an object.
     * \tparam T class of the object
     * \tparam TFPTR function pointer type for the function to be called
     */
    
    template <class T, class TFPTR>
    class MemberFunctionCommandInvoker : public CommandInvoker {
    public:

        /**
         * \brief MemberFunctionCommandInvoker constructor.
         * \param[in] command a pointer to the Command object
         * \param[in] target a pointer to the object
         * \param[in] target_fun the member function pointer
         */
        
        MemberFunctionCommandInvoker(
            Command* command,
            T* target,
            TFPTR target_fun
        ) :
            command_(command),
            target_(target),
            target_fun_(target_fun) {
        }

        /**
         * \copydoc CommandInvoker::invoke()
         */
        
        virtual void invoke() {
            command_->invoke(target_, target_fun_);
        }

        /**
         * \copydoc CommandInvoker::auto_create_args()
         */
        
        virtual void auto_create_args() {
            command_->invoke((T*)(nullptr), (TFPTR)(nullptr));            
        }

        
    private:
        Command* command_;
        T* target_;
        TFPTR target_fun_; 
    };

    /*****************************************************************/
    
    template<class FPTR> inline void Command::set_current(
        const std::string& prototype,
        FPTR fun
    ) {
        set_current(new Command(prototype));
        current()->set_invoker(
            new FunctionCommandInvoker<FPTR>(current(), fun)
        );
    }

    /*****************************************************************/    
    
    template<class T, class TFPTR> inline void Command::set_current(
        const std::string& prototype,
        T* target,
        TFPTR tfun
    ) {
        set_current(new Command(prototype));
        current()->set_invoker(
            new MemberFunctionCommandInvoker<T, TFPTR>(current(), target, tfun)
        );
    }

    /*****************************************************************/    
    
}

#endif
