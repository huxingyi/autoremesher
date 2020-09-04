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

#ifndef GEOGRAM_BASIC_FACTORY
#define GEOGRAM_BASIC_FACTORY

#include <geogram/basic/memory.h>
#include <geogram/basic/counted.h>
#include <string>
#include <map>
#include <vector>
#include <typeinfo>

/**
 * \file geogram/basic/factory.h
 * \brief Generic factory mechanism
 */

namespace GEO {

    /**
     * \brief Repository of unique instances
     * \details InstanceRepo is the central point to access unique singleton
     * instances using their type name. Instances are registered at their
     * creation using function add() and retrieved using get().
     * \internal
     * This class avoids a recurrent problem with templated singleton classes:
     * the C++ template instantiation mechanism does not guarantee that static
     * variables in template classes have a single occurrence. Storing
     * templated singleton instances in a central repository guarantees the
     * instances uniqueness.
     */
    class GEOGRAM_API InstanceRepo {
    public:
        /**
         * \brief Type of the Instances stored in the repository
         */
        typedef Counted Instance;

        /**
         * \brief Gets unique instance from the repository
         * \details This function returns a unique instance of type \p
         * InstanceType. If the instance is already registered in the
         * repository, it is returned, otherwise a new instance is created and
         * registered to the repository using the InstanceType name.
         * \tparam InstanceType type of the instance
         * \return a pointer to a \p InstanceType unique instance.
         */
        template <class InstanceType>
        static InstanceType& instance() {
            const std::string name = typeid(InstanceType).name();
            Instance* instance = get(name);
            if(instance == nullptr) {
                instance = new InstanceType;
                add(name, instance);
            }
            return *static_cast<InstanceType*>(instance);
        }

    private:
        /**
         * \brief Registers an instance to the repository
         * \param[in] name registration key of the instance
         * \param[in] instance the instance to register
         */
        static void add(const std::string& name, Instance* instance);

        /**
         * \brief Retrieves an instance from the repository
         * \param[in] name registration key of then instance
         * \retval the pointer to the stored instance.
         * \retval a null pointer otherwise
         */
        static Instance* get(const std::string& name);
    };

    /**************************************************************************/

    /**
     * \brief Factory of typed objects
     * \details A Factory is a mechanism to create objects without knowing
     * their types in advance. The object types that a Factory can instantiate
     * belong to a type hierarchy whose base type is defined by \p Type.
     * The Factory uses a registry of creator functions bound to user-defined
     * names. The user-defined names can be used to:
     * - register a creator function (see register_creator())
     * - create an object, using the creator function bound to the name (see
     *   create_object())
     * \tparam FactoryCreator the type of the creator used to create objects
     * in this Factory. FactoryCreator must define:
     * - the type CreatorType type of the creation function
     * - a static template function create(...) used to actually create
     *   objects of a given concrete type
     * \internal
     * \todo
     * The current implementation provides a FactoryCreator for constructor
     * with no arguments and with 1 argument, ... A better implementation
     * should use variadic templates when c++11 will be supported.
     */
    template <class FactoryCreator>
    class Factory : public InstanceRepo::Instance {
    public:
        typedef typename FactoryCreator::CreatorType CreatorType;

        /**
         * \brief Registers a creator.
         * \details This creates a new creator for objects of type \p
         * ConcreteType and registers it with the user-defined name \p name.
         * \param[in] name name of the \p ConcreteType creator in the Factory
         * \tparam ConcreteType the type of the objects to create
         * \par Usage example:
         * \code
         * struct MyBaseClass { ... };
         * typedef Factory<MyBaseClass> MyFactory;
         *
         * struct MyDerivedClass : MyBaseClass { ... };
         * MyFactory::register_creator<MyDerivedClass>("my_derived_class");
         * \endcode
         * \see RegisterCreator()
         */
        template <class ConcreteType>
        static void register_creator(const std::string& name) {
            Factory& self = instance();
            self.registry_[name] =
                FactoryCreator::template create<ConcreteType>;
        }

        /**
         * \brief Finds a creator by name.
         * \param[in] name a user-defined name identifying 
         *  a creator in the Factory
         * \retval the creator associated to \p name if \p name exists
         * \retval null pointer otherwise
         */
        static CreatorType find_creator(const std::string& name) {
            Factory& self = instance();
            auto i = self.registry_.find(name);
            return i == self.registry_.end() ? nullptr : i->second;
        }

        /**
         * \brief Lists all registered creators.
         * \details This stores the names of the registered creators to output
         * vector \p names.
         * \param[out] names output list of registered names
         */
        static void list_creators(std::vector<std::string>& names) {
            Factory& self = instance();
            for(auto& it : self.registry_) {
                names.push_back(it.first);
            }
        }

        /**
         * \brief Tests whether the factory has a creator.
         * \param[in] name name of the creator
         * \retval true if creator \p name is registered in the factory
         * \retval false otherwise
         */
        static bool has_creator(const std::string& name) {
            Factory& self = instance();
            for(auto& it : self.registry_) {
                if(it.first == name) {
                    return true;
                }
            }
            return false;
        }

        /**
         * \brief Helper class to register a creator
         * \details Declaring a static instance of this class with appropriate
         * parameters allows to register creators to the Factory at program
         * initialization, thus making them available before the program
         * actually starts.
         * \tparam ConcreteType the type of the objects to create
         * \par Usage example:
         * \code
         * struct MyBaseClass { ... };
         * typedef Factory<MyBaseClass> MyFactory;
         *
         * struct MyDerivedClass : MyBaseClass { ... };
         * static MyFactory::RegisterCreator<MyDerivedClass>
         *     register_derived("my_derived_class");
         * \endcode
         * \see geo_register_creator()
         */
        template <class ConcreteType>
        struct RegisterCreator {
            /**
             * \brief Constructs a registration object.
             * \details The constructor calls register_creator() to register a
             * \p ConcreteType creator bound to name \p name.
             * \param[in] name name of the ConcreteType creator in the Factory
             * \see Factory::register_creator()
             */
            RegisterCreator(const std::string& name) {
                Factory::template register_creator<ConcreteType>(name);
            }
        };

    protected:
        /**
         * \brief Factory destructor.
         */
        virtual ~Factory() {
        }

    private:
        /**
         * \brief Gets the Factory unique instance.
         * \see InstanceRepo
         */
        static inline Factory& instance() {
            return InstanceRepo::instance<Factory>();
        }

        /**
         * \brief Registry of object creator functions.
         */
        typedef std::map<std::string, CreatorType> Registry;
        Registry registry_;
    };

    /**
     * \brief Factory creator without constructor arguments.
     * \details This defines the function to create objects with no
     * constructor arguments.
     * \tparam Type base type of the created objects
     */
    template <class Type>
    struct FactoryCreator0 {
        /**
         * \brief Type of the creation function
         */
        typedef Type* (* CreatorType)();

        /**
         * \brief Creation function
         * \tparam ConcreteType actual type of the object to create.
         */
        template <class ConcreteType>
        static Type* create() {
            return new ConcreteType;
        }
    };

    /**
     * \brief Factory for types without constructor arguments.
     * \details This implements a Factory to create objects with no
     * constructor arguments.
     * \tparam Type base type of the created objects
     * \see FactoryCreator
     */
    template <class Type>
    class Factory0 : public Factory<FactoryCreator0<Type> > {
        typedef Factory<FactoryCreator0<Type> > BaseClass;

    public:
        /**
         * \brief Creates a new object.
         * \details This creates a new object using the creator function
         * bound to the specified user-defined \p name.
         * \param[in] name specifies wihch kind of object to create
         * \retval a pointer to a new object is \p name is associated to a
         * creator in this Factory
         * \retval a null pointer otherwise.
         */
        static Type* create_object(const std::string& name) {
            typename BaseClass::CreatorType creator =
                BaseClass::find_creator(name);
            return creator == nullptr ? nullptr : (* creator)();
        }
    };

    /**
     * \brief Factory creator with one argument.
     * \details This defines the function to create objects with one
     * argument in the constructor.
     * \tparam Type base type of the created objects
     * \tparam Param1 type of the constructor argument
     */
    template <class Type, class Param1>
    struct FactoryCreator1 {
        /**
         * \brief Type of the creation function
         */
        typedef Type* (* CreatorType)(const Param1&);

        /**
         * \brief Creation function
         * \tparam ConcreteType actual type of the object to create.
         */
        template <class ConcreteType>
        static Type* create(const Param1& param1) {
            return new ConcreteType(param1);
        }
    };

    /**
     * \brief Factory for types with one constructor argument.
     * \details This implements a Factory to create objects with no
     * constructor arguments.
     * \tparam Type base type of the created objects
     * \tparam Param1 type of the constructor argument
     * \see FactoryCreator1
     */
    template <class Type, class Param1>
    class Factory1 : public Factory<FactoryCreator1<Type, Param1> > {
        typedef Factory<FactoryCreator1<Type, Param1> > BaseClass;

    public:
        /**
         * \brief Creates a new object with parameter(s).
         * \details This creates a new object using the creator function
         * bound to the specified user-defined \p name.
         * \param[in] name specifies which kind of object to create
         * \param[in] param1 parameter passed to the object constructor
         * \retval a pointer to a new object is \p name is associated to a
         * creator in this Factory
         * \retval a null pointer otherwise.
         */
        static Type* create_object(const std::string& name, const Param1& param1) {
            typename BaseClass::CreatorType creator =
                BaseClass::find_creator(name);
            return creator == nullptr ? nullptr : (* creator)(param1);
        }
    };

    /**
     * \brief Helper macro to register a creator
     * \details This declares a static instance of
     * FactoryType::RegisterCreator to register a \p ConcreteType creator in
     * \p FactoryType at program initialization time.
     * \par Usage example:
     * \code
     * struct MyBaseClass { ... };
     * typedef Factory<MyBaseClass> MyFactory;
     *
     * struct MyDerivedClass : MyBaseClass { ... };
     * geo_register_creator(MyFactory, MyDerivedClass, "my_derived_class");
     * \endcode
     * \param[in] FactoryType identifies the target Factory
     * \param[in] ConcreteType the type of the object to create
     * \param[in] name name of the \p ConcreteType creator in the Factory
     * \see Factory::RegisterCreator
     */
#define geo_register_creator(FactoryType, ConcreteType, name) \
    static FactoryType::RegisterCreator<ConcreteType> \
    CPP_CONCAT(Factory_register_creator_, __LINE__) (name); \
    geo_argused(CPP_CONCAT(Factory_register_creator_, __LINE__))
}

#endif

