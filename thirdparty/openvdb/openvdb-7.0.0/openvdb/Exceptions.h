// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_EXCEPTIONS_HAS_BEEN_INCLUDED
#define OPENVDB_EXCEPTIONS_HAS_BEEN_INCLUDED

#include <openvdb/version.h>
#include <exception>
#include <sstream>
#include <string>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {

class OPENVDB_API Exception: public std::exception
{
public:
    Exception(const Exception&) = default;
    Exception(Exception&&) = default;
    Exception& operator=(const Exception&) = default;
    Exception& operator=(Exception&&) = default;
    ~Exception() override = default;

    const char* what() const noexcept override
    {
        try { return mMessage.c_str(); } catch (...) {}
        return nullptr;
    }

protected:
    Exception() noexcept {}
    explicit Exception(const char* eType, const std::string* const msg = nullptr) noexcept
    {
        try {
            if (eType) mMessage = eType;
            if (msg) mMessage += ": " + (*msg);
        } catch (...) {}
    }

private:
    std::string mMessage;
};


#define OPENVDB_EXCEPTION(_classname) \
class OPENVDB_API _classname: public Exception \
{ \
public: \
    _classname() noexcept: Exception( #_classname ) {} \
    explicit _classname(const std::string& msg) noexcept: Exception( #_classname , &msg) {} \
}


OPENVDB_EXCEPTION(ArithmeticError);
OPENVDB_EXCEPTION(IndexError);
OPENVDB_EXCEPTION(IoError);
OPENVDB_EXCEPTION(KeyError);
OPENVDB_EXCEPTION(LookupError);
OPENVDB_EXCEPTION(NotImplementedError);
OPENVDB_EXCEPTION(ReferenceError);
OPENVDB_EXCEPTION(RuntimeError);
OPENVDB_EXCEPTION(TypeError);
OPENVDB_EXCEPTION(ValueError);

#undef OPENVDB_EXCEPTION


/// @deprecated Use ValueError instead.
class OPENVDB_API IllegalValueException: public Exception {
public:
    OPENVDB_DEPRECATED IllegalValueException() noexcept: Exception("IllegalValueException") {}
    OPENVDB_DEPRECATED explicit IllegalValueException(const std::string& msg) noexcept:
        Exception("IllegalValueException", &msg) {}
};

} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb


#define OPENVDB_THROW(exception, message) \
{ \
    std::string _openvdb_throw_msg; \
    try { \
        std::ostringstream _openvdb_throw_os; \
        _openvdb_throw_os << message; \
        _openvdb_throw_msg = _openvdb_throw_os.str(); \
    } catch (...) {} \
    throw exception(_openvdb_throw_msg); \
} // OPENVDB_THROW

#endif // OPENVDB_EXCEPTIONS_HAS_BEEN_INCLUDED
