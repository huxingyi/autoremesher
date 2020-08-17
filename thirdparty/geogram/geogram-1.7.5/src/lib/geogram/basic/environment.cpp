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

#include <geogram/basic/environment.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/argused.h>
#include <algorithm>
#include <stdlib.h>

namespace {
    using namespace GEO;

    /**
     * \brief Root environment
     * \details The root environment stores properties as name-value pairs in
     * a dictionary.
     */
    class RootEnvironment : public Environment {
    protected:
        /** \copydoc GEO::Environment::get_local_value() */
        bool get_local_value(
            const std::string& name, std::string& value
        ) const override {
            auto it = values_.find(name);
            if(it != values_.end()) {
                value = it->second;
                return true;
            }
            return false;
        }

        /** \copydoc GEO::Environment::set_local_value() */
        bool set_local_value(
            const std::string& name, const std::string& value
        ) override {
            values_[name] = value;
            return true;
        }

        /** \brief ProcessEnvironment destructor */
        ~RootEnvironment() override {
        }

    private:
        /** \brief Stores the variable values by name */
        typedef std::map<std::string, std::string> ValueMap;
        ValueMap values_;
    };
}

namespace GEO {

    /************************************************************************/

    VariableObserver::VariableObserver(
        const std::string& var_name
    ) :
        observed_variable_(var_name),
        environment_(nullptr)
    {
        environment_ = Environment::instance()->find_environment(var_name);
        geo_assert(environment_ != nullptr);
        environment_->add_observer(var_name, this);
    }

    VariableObserver::~VariableObserver() {
        environment_->remove_observer(observed_variable_, this);
    }

    /************************************************************************/

    void VariableObserverList::notify_observers(
        const std::string& value
    ) {
        if(block_notify_) {
            return;
        }
        block_notify_ = true;
        for(size_t i = 0; i < observers_.size(); i++) {
            observers_[i]->value_changed(value);
        }
        block_notify_ = false;
    }

    void VariableObserverList::add_observer(
        VariableObserver* observer
    ) {
	auto it = std::find(observers_.begin(), observers_.end(), observer);
        geo_assert(it == observers_.end());
        observers_.push_back(observer);
    }

    void VariableObserverList::remove_observer(
        VariableObserver* observer
    ) {
        auto it = std::find(observers_.begin(), observers_.end(), observer);
        geo_assert(it != observers_.end());
        observers_.erase(it);
    }

    /************************************************************************/

    Environment::Environment_var Environment::instance_;

    Environment* Environment::instance() {
        if(instance_ == nullptr) {
            static bool created = false;
            if(created) {
                std::cerr
                    << "CRITICAL: Environment::instance() "
                    << "called after the instance was deleted"
                    << std::endl;
                geo_abort();
            }
            created = true;
            instance_ = new RootEnvironment();
            instance_->add_environment(new SystemEnvironment());
        }
        return instance_;
    }

    void Environment::terminate() {
        instance_.reset();
    }

    Environment::~Environment() {
    }

    bool Environment::add_environment(Environment* env) {
        environments_.push_back(env);
        return true;
    }

    bool Environment::has_value(const std::string& name) const {
        std::string value;
        return get_value(name, value);
    }

    bool Environment::set_value(
        const std::string& name, const std::string& value
    ) {
        for(size_t i = 0; i < environments_.size(); i++) {
            if(environments_[i]->set_value(name, value)) {
                notify_local_observers(name, value);
                return true;
            }
        }
        if(set_local_value(name, value)) {
            notify_local_observers(name, value);
            return true;
        }
        return false;
    }

    bool Environment::get_value(
        const std::string& name, std::string& value
    ) const {
        if(get_local_value(name, value)) {
            return true;
        }
        for(size_t i = 0; i < environments_.size(); i++) {
            if(environments_[i]->get_value(name, value)) {
                return true;
            }
        }
        return false;
    }

    std::string Environment::get_value(const std::string& name) const {
        std::string value;
        bool variable_exists = get_value(name, value);
        if(!variable_exists) {
            Logger::err("Environment")
                << "No such variable: " << name
                << std::endl;
            Logger::err("Environment")
                << "Probably missing CmdLine::import_arg_group(\"...\");"
                << std::endl;
        }
        geo_assert(variable_exists);
        return value;
    }

    Environment* Environment::find_environment(const std::string& name) {
        std::string value;
        if(get_local_value(name, value)) {
            return this;
        }
        for(index_t i=0; i<environments_.size(); ++i) {
            Environment* result = environments_[i]->find_environment(name);
            if(result != nullptr) {
                return result;
            }
        }
        return nullptr;
    }
    
    bool Environment::add_observer(
        const std::string& name, VariableObserver* observer
    ) {
        observers_[name].add_observer(observer);
        return true;
    }

    bool Environment::remove_observer(
        const std::string& name, VariableObserver* observer
    ) {
        auto obs = observers_.find(name);
        geo_assert(obs != observers_.end());
        obs->second.remove_observer(observer);
        return true;
    }

    bool Environment::notify_observers(
        const std::string& name, bool recursive
    ) {
        std::string value = get_value(name);
        return notify_observers(name, value, recursive);
    }

    bool Environment::notify_observers(
        const std::string& name, const std::string& value,
        bool recursive
    ) {
        if(recursive) {
            for(size_t i = 0; i < environments_.size(); i++) {
                environments_[i]->notify_observers(
                    name, value, true
                );
            }
        }
        return notify_local_observers(name, value);
    }

    bool Environment::notify_local_observers(
        const std::string& name, const std::string& value
    ) {
        auto it = observers_.find(name);
        if(it != observers_.end()) {
            it->second.notify_observers(value);
        }
        return true;
    }

    /************************************************************************/

    SystemEnvironment::~SystemEnvironment() {
    }

    bool SystemEnvironment::set_local_value(
        const std::string& name, const std::string& value
    ) {
        geo_argused(name);
        geo_argused(value);
        return false;
    }

    bool SystemEnvironment::get_local_value(
        const std::string& name, std::string& value
    ) const {
        // For the moment, deactivated under Windows
#ifdef GEO_OS_WINDOWS
        geo_argused(name);
        geo_argused(value);
        return false;
#else
        char* result = ::getenv(name.c_str());
        if(result != nullptr) {
            value = std::string(result);
        }
        return result != nullptr;
#endif
    }
}

