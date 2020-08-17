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

#ifndef GEOGRAM_BASIC_ENVIRONMENT
#define GEOGRAM_BASIC_ENVIRONMENT

#include <geogram/basic/common.h>
#include <geogram/basic/smart_pointer.h>
#include <geogram/basic/counted.h>
#include <string>
#include <vector>
#include <map>

/**
 * \file geogram/basic/environment.h
 * \brief Provides a mechanism to store global
 *  variables, retrieve them by their names and
 *  attach observers to them.
 */

namespace GEO {

    class Environment;
    
    /************************************************************************/

    /**
     * \brief Observes Environment variables.
     * \details VariableObserver offers the possibility to receive
     * notifications when a variable is changed in an Environment.
     *
     * To listen to a variable:
     * -# Create a class derived class from VariableObserver
     * and implement function value_changed().
     * -# Create a MyObserver instance with name "my_variable", it will be
     * automatically attached to the variable in the root Environment.
     *
     * \code
     * struct MyObserver : VariableObserver {
     *     MyObserver(const std::string& name) : VariableObserver(name) {}
     *     virtual void value_changed(const std::string& new_value) {
     * };
     * std::auto_ptr<MyObserver> myobserver =
     *     new MyObserver("my_variable");
     * \endcode
     */
    class GEOGRAM_API VariableObserver {
    public:
        /**
         * \brief Creates a new variable observer.
         * \details This creates a new observer for variable \p var_name and
         * automatically adds itself to the variable's observers in the root
         * environment.
         * \param[in] var_name name of the variable observed.
         */
        VariableObserver(const std::string& var_name);

        /**
         * \brief Receives a change notification.
         * \details This function is called by the Environment when
         * the variable observed by this observer is modified.
         * \param[in] new_value the new value of the observed variable.
         */
        virtual void value_changed(const std::string& new_value) = 0;

        /**
         * \brief Deletes the observer.
         * \details This automatically removes this observer from the
         * root environment.
         */
        virtual ~VariableObserver();

        /**
         * \brief Gets the observed variable.
         * \return The name of the variable observed by this observer.
         */
        const std::string& observed_variable() const {
            return observed_variable_;
        }

    private:
        std::string observed_variable_;
        Environment* environment_;
    };

    /************************************************************************/

    /**
     * \brief List of VariableObserver%s
     * \details List of variable observers are attached to observed variables
     * in the Environment%s.
     */
    class GEOGRAM_API VariableObserverList {
    public:
        /**
         * \brief Creates an empty list of variable observers.
         */
        VariableObserverList() :
            block_notify_(false) {
        }

        /**
         * \brief Notifies all observers in the list.
         * \param[in] value the value of the variable being changed.
         */
        void notify_observers(const std::string& value);

        /**
         * \brief Adds an observer to the list.
         * This adds observer \p observer at the end of the list only if it is
         * not already present.
         * \param[in] observer a pointer to the VariableObserver to add.
         */
        void add_observer(VariableObserver* observer);

        /**
         * \brief Removes an observer from the list.
         * \param[in] observer a pointer to the VariableObserver to remove.
         */
        void remove_observer(VariableObserver* observer);

    private:
        /** List of VariableObserver%s */
        typedef std::vector<VariableObserver*> Observers;
        Observers observers_;
        bool block_notify_;
    };

    /************************************************************************/

    /**
     * \brief Application environment
     * \details
     * Environment is a flexible framework for storing and retrieving
     * application properties. Most important client functions are:
     * - get_value() to retrieve a property
     * - set_value() to store a property
     *
     * By default, the framework provides a single root Environment that can
     * be accessed with function instance(). This root environment uses a
     * dictionary for storing application properties as name-value pairs.
     *
     * But developers can define custom Environment classes to access
     * properties differently. For this, the custom Environment classes must
     * reimplement low-level access functions get_local_value() and
     * set_local_value(). For instance, one can redefine low-level functions
     * to:
     *
     * - access properties in a file database
     * - access properties as system environment variables (see
     *   SystemEnvironment)
     * - expose/control a software module configuration:
     *   - get_local_value() exposes the module configuration as properties
     *   - set_local_value() allows to control the module configuration
     *   through properties.
     *
     *   This technique is widely used in Vorpaline, for instance:
     *   - Process has a dedicated environment to control Process
     *     configuration (multithreading, FPE, ...)
     *   - Logger also has a dedicated environment to configure the Logger
     *     behavior, allowed features, ...
     *
     * Plugging custom Environment%s in the framework is as simple as adding
     * the custom Environment as a child of the root Environment with function
     * add_environment(). Setting a property in an environment affects
     * this environment locally \b only if no child environment can store the
     * property. Similarily, retrieving a property from an environment first
     * checks if the property exists locally, then in all child environments.
     *
     * In addition, the Environment framework provides a mechanism for being
     * notified when a property is modified: VariableObserver%s can be
     * attached to specific properties to capture modifications of their value
     * (for more details see VariableObserver).
     */
    class GEOGRAM_API Environment : public Counted {
    public:
        /**
         * \brief Gets the root environment
         * \details If the root environment does not yet exists, it is created
         * on the fly.
         * \return A pointer to the root environment
         */
        static Environment* instance();

        /**
         * \brief Cleans up the environment
         * \details This destroys the whole root Environment hierarchy.
         */
        static void terminate();

        /**
         * \brief Adds a child environment
         * \details Environment \p env is added as a child of this
         * environment which takes ownership of \p env. The child environment
         * will be deleted when this environment is deleted.
         * \param[in] env the child environment
         * \retval true if the child has been successfully added
         * \retval false otherwise
         */
        virtual bool add_environment(Environment* env);

        /**
         * \brief Tests if a variable exists
         * \param[in] name the name of the variable
         * \retval true if the variable exists
         * \retval false otherwise
         */
        bool has_value(const std::string& name) const;

        /**
         * \brief Retrieves the value of a variable
         * \details Searches variable \p name and stores its value in the
         * output string \p value. The function first checks if the variable
         * exists locally, then in all child environments recursively.
         * \param[in] name the name of the variable
         * \param[out] value is set the variable value if it was found
         * either locally or in a child environment.
         * \retval true if the variable was found
         * \retval false otherwise
         */
        virtual bool get_value(
            const std::string& name, std::string& value
        ) const;

        /**
         * \brief Retrieves the value of a variable
         * \details This is a variant of get_value(name, value) that returns
         * the variable value directly it it exists. If the variable is not
         * found, then the function calls abort().
         * \param[in] name the name of the variable
         * \return the variable value if it exists.
         */
        std::string get_value(const std::string& name) const;

        /**
         * \brief Sets a variable value
         * \details Sets the variable named \p name to the given \p value. The
         * function first visits all child environments recursively until one
         * of them accepts the variable. If no child environment can store the
         * variable, the variable is set locally in this environment. If a
         * variable is set in an environment, all the variable observers are
         * notified, starting from the modified environment up to the root
         * environment.
         * \param[in] name the name of the variable
         * \param[in] value the value of the variable
         * \retval true if the variable was successfully added, either locally
         * or in a child environment.
         * \retval false otherwise
         */
        virtual bool set_value(
            const std::string& name, const std::string& value
        );

        /**
         * \brief Finds the environment that declares a variable as
         *  a local name.
         * \param[in] name the name of the variable
         * \return a pointer to the Environment that has \p name as a 
         *  local variable, or nullptr if no such environment exists
         */
        virtual Environment* find_environment(const std::string& name);
        
        /**
         * \brief Attaches an observer to a variable
         * \details Adds observer \p observer to the list of observers
         * attached to variable \p name. If the observer is already attached
         * to the variable, the function calls abort(). The environment does
         * \b not take ownership of the observer, it is the responsibility of
         * the caller to delete all the variable observers added to an
         * Environment.
         * \param[in] name the name of the variable
         * \param[in] observer a variable observer to add
         * \retval true if \p observer has been successfully added
         * \retval false otherwise
         */
        virtual bool add_observer(
            const std::string& name, VariableObserver* observer
        );

        /**
         * \brief Detaches an observer from a variable
         * \details Removes observer \p observer from the list of observers
         * attached to variable \p name. If the observer is not attached to
         * the variable, the function calls abort(). The environment does \b
         * not delete the removed observer, it is the responsibility of the
         * caller to delete all the variable observers added to an
         * Environment.
         * \param[in] name the name of the variable
         * \param[in] observer a variable observer to remove
         * \retval true if \p observer has been successfully removed
         * \retval false otherwise
         */
        virtual bool remove_observer(
            const std::string& name, VariableObserver* observer
        );

        /**
         * \brief Notifies observers
         * \details This notifies the observers attached to variable \p
         * name in this environment, passing them the current value of the
         * variable. If \p recursive is set to \c true, then the function
         * recursively notifies observers in the child contexts.
         * \param[in] name the name of the variable
         * \param[in] recursive if \c true, notifies observers in the child
         * contexts. This is \c false by default.
         * \return \c true
         */
        virtual bool notify_observers(
            const std::string& name, bool recursive = false
        );

    protected:
        /**
         * \brief Environment destructor
         * \details This deletes all the child environments, but it does \b
         * not delete the variable observers.
         */
        virtual ~Environment();

        /**
         * \brief Retrieves a variable value locally
         * \details This function is used internally. It searches variable \p
         * name \b locally and stores its value in the output string \p value.
         * \param[in] name the name of the variable
         * \param[out] value is set the variable value if it was found \b
         * locally.
         * \retval true if the variable was found
         * \retval false if not
         * \note This function must be reimplemented in derived custom
         * environments.
         */
        virtual bool get_local_value(
            const std::string& name, std::string& value
        ) const = 0;

        /**
         * \brief Sets a variable value locally
         * \details This function is used internally. It sets the variable
         * named \p name to the given \p value \b locally.
         * \param[in] name the name of the variable
         * \param[in] value the value of the variable
         * \retval true if the variable was successfully added \b locally
         * \retval false otherwise
         * \note This function must be reimplemented in derived custom
         * environments.
         */
        virtual bool set_local_value(
            const std::string& name, const std::string& value
        ) = 0;

        /**
         * \brief Notifies observers
         * \details This function is used internally. It notifies the
         * observers attached to variable \p name in this environment, passing
         * them the modified value \p value. If \p recursive is \c true, then
         * the function recursively notifies observers in the child contexts.
         * \param[in] name the name of the variable
         * \param[in] value the modified value
         * \param[in] recursive if \c true, notifies observers in the child
         * contexts.
         * \return \c true
         */
        bool notify_observers(
            const std::string& name, const std::string& value,
            bool recursive
        );

        /**
         * \brief Notifies local observers
         * \details This function is used internally. It notifies the
         * observers attached to variable \p name in this environment, passing
         * them the modified value \p value. Observers in child environments
         * are \b not notified.
         * \param[in] name the name of the variable
         * \param[in] value the modified value
         * \return \c true
         */
        bool notify_local_observers(
            const std::string& name, const std::string& value
        );

    private:
        /** Smart pointer that contains a Environment object */
        typedef SmartPointer<Environment> Environment_var;

        /** List of child environments */
        typedef std::vector<Environment_var> Environments;

        /** Stores VariableObserverList indexed by name */
        typedef std::map<std::string, VariableObserverList> ObserverMap;

        static Environment_var instance_;
        Environments environments_;
        ObserverMap observers_;
    };

    /************************************************************************/

    /**
     * \brief System environment
     * \details
     * This class is a specialization of Environment that retrieves the
     * variable values from the system environment, but does not allow to set
     * them.
     */
    class SystemEnvironment : public Environment {
    protected:
        /** SystemEnvironment destructor */
        virtual ~SystemEnvironment();

        /**
         * \copydoc Environment::set_local_value()
         * This function does actually \b not update the system environment
         * and always returns \c false.
         */
        virtual bool set_local_value(
            const std::string& name, const std::string& value
        );

        /**
         * \copydoc Environment::get_local_value()
         * The value is retrieved from the system environment using the system
         * function getenv().
         */
        virtual bool get_local_value(
            const std::string& name, std::string& value
        ) const;
    };
}

#endif

