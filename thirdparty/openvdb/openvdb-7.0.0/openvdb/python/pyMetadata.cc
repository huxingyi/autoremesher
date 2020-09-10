// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <boost/python.hpp>
#include "openvdb/openvdb.h"

namespace py = boost::python;
using namespace openvdb::OPENVDB_VERSION_NAME;

namespace {

class MetadataWrap: public Metadata, public py::wrapper<Metadata>
{
public:
    Name typeName() const { return static_cast<const Name&>(this->get_override("typeName")()); }
    Metadata::Ptr copy() const {
        return static_cast<const Metadata::Ptr&>(this->get_override("copy")());
    }
    void copy(const Metadata& other) { this->get_override("copy")(other); }
    std::string str() const {return static_cast<const std::string&>(this->get_override("str")());}
    bool asBool() const { return static_cast<const bool&>(this->get_override("asBool")()); }
    Index32 size() const { return static_cast<const Index32&>(this->get_override("size")()); }

protected:
    void readValue(std::istream& is, Index32 numBytes) {
        this->get_override("readValue")(is, numBytes);
    }
    void writeValue(std::ostream& os) const {
        this->get_override("writeValue")(os);
    }
};

// aliases disambiguate the different versions of copy
Metadata::Ptr (MetadataWrap::*copy0)() const = &MetadataWrap::copy;
void (MetadataWrap::*copy1)(const Metadata&) = &MetadataWrap::copy;

} // end anonymous namespace


void exportMetadata();

void
exportMetadata()
{
    py::class_<MetadataWrap, boost::noncopyable> clss(
        /*classname=*/"Metadata",
        /*docstring=*/
            "Class that holds the value of a single item of metadata of a type\n"
            "for which no Python equivalent exists (typically a custom type)",
        /*ctor=*/py::no_init // can only be instantiated from C++, not from Python
    );
    clss.def("copy", py::pure_virtual(copy0),
            "copy() -> Metadata\n\nReturn a copy of this value.")
        .def("copy", py::pure_virtual(copy1),
            "copy() -> Metadata\n\nReturn a copy of this value.")
        .def("type", py::pure_virtual(&Metadata::typeName),
            "type() -> str\n\nReturn the name of this value's type.")
        .def("size", py::pure_virtual(&Metadata::size),
            "size() -> int\n\nReturn the size of this value in bytes.")
        .def("__nonzero__", py::pure_virtual(&Metadata::asBool))
        .def("__str__", py::pure_virtual(&Metadata::str))
        ;
    py::register_ptr_to_python<Metadata::Ptr>();
}
