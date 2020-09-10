// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include "MetaMap.h"

#include "util/logging.h"
#include <sstream>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {

MetaMap::MetaMap(const MetaMap& other)
{
    this->insertMeta(other);
}


MetaMap::Ptr
MetaMap::copyMeta() const
{
    MetaMap::Ptr ret(new MetaMap);
    ret->mMeta = this->mMeta;
    return ret;
}


MetaMap::Ptr
MetaMap::deepCopyMeta() const
{
    return MetaMap::Ptr(new MetaMap(*this));
}


MetaMap&
MetaMap::operator=(const MetaMap& other)
{
    if (&other != this) {
        this->clearMetadata();
        // Insert all metadata into this map.
        ConstMetaIterator iter = other.beginMeta();
        for ( ; iter != other.endMeta(); ++iter) {
            this->insertMeta(iter->first, *(iter->second));
        }
    }
    return *this;
}


void
MetaMap::readMeta(std::istream &is)
{
    // Clear out the current metamap if need be.
    this->clearMetadata();

    // Read in the number of metadata items.
    Index32 count = 0;
    is.read(reinterpret_cast<char*>(&count), sizeof(Index32));

    // Read in each metadata.
    for (Index32 i = 0; i < count; ++i) {
        // Read in the name.
        Name name = readString(is);

        // Read in the metadata typename.
        Name typeName = readString(is);

        // Read in the metadata value and add it to the map.
        if (Metadata::isRegisteredType(typeName)) {
            Metadata::Ptr metadata = Metadata::createMetadata(typeName);
            metadata->read(is);
            insertMeta(name, *metadata);
        } else {
#if OPENVDB_ABI_VERSION_NUMBER >= 5
            UnknownMetadata metadata(typeName);
            metadata.read(is); // read raw bytes into an array
            // only add unknown metadata to the grid if not temporary,
            // denoted by a double underscore prefix (such as __metadata)
            bool temporary = typeName.compare(0, 2, "__") == 0;
            if (!temporary) {
                insertMeta(name, metadata);
            }
#else
            OPENVDB_LOG_WARN("cannot read metadata \"" << name
                << "\" of unregistered type \"" << typeName << "\"");
            UnknownMetadata metadata;
            metadata.read(is);
#endif
        }
    }
}


void
MetaMap::writeMeta(std::ostream &os) const
{
    // Write out the number of metadata items we have in the map. Note that we
    // save as Index32 to save a 32-bit number. Using size_t would be platform
    // dependent.
    Index32 count = static_cast<Index32>(metaCount());
    os.write(reinterpret_cast<char*>(&count), sizeof(Index32));

    // Iterate through each metadata and write it out.
    for (ConstMetaIterator iter = beginMeta(); iter != endMeta(); ++iter) {
        // Write the name of the metadata.
        writeString(os, iter->first);

        // Write the type name of the metadata.
        writeString(os, iter->second->typeName());

        // Write out the metadata value.
        iter->second->write(os);
    }
}


void
MetaMap::insertMeta(const Name &name, const Metadata &m)
{
    if (name.size() == 0)
        OPENVDB_THROW(ValueError, "Metadata name cannot be an empty string");

    // See if the value already exists, if so then replace the existing one.
    MetaIterator iter = mMeta.find(name);

    if (iter == mMeta.end()) {
        // Create a copy of the metadata and store it in the map
        Metadata::Ptr tmp = m.copy();
        mMeta[name] = tmp;
    } else {
        if (iter->second->typeName() != m.typeName()) {
            std::ostringstream ostr;
            ostr << "Cannot assign value of type "
                 << m.typeName() << " to metadata attribute " << name
                 << " of " << "type " << iter->second->typeName();
            OPENVDB_THROW(TypeError, ostr.str());
        }
        // else
        Metadata::Ptr tmp = m.copy();
        iter->second = tmp;
    }
}


void
MetaMap::insertMeta(const MetaMap& other)
{
    for (ConstMetaIterator it = other.beginMeta(), end = other.endMeta(); it != end; ++it) {
        if (it->second) this->insertMeta(it->first, *it->second);
    }
}


void
MetaMap::removeMeta(const Name &name)
{
    MetaIterator iter = mMeta.find(name);
    if (iter != mMeta.end()) {
        mMeta.erase(iter);
    }
}


bool
MetaMap::operator==(const MetaMap& other) const
{
    // Check if the two maps have the same number of elements.
    if (this->mMeta.size() != other.mMeta.size()) return false;
    // Iterate over the two maps in sorted order.
    for (ConstMetaIterator it = beginMeta(), otherIt = other.beginMeta(), end = endMeta();
        it != end; ++it, ++otherIt)
    {
        // Check if the two keys match.
        if (it->first != otherIt->first) return false;
        // Check if the two values are either both null or both non-null pointers.
        if (bool(it->second) != bool(otherIt->second)) return false;
        // If the two values are both non-null, compare their contents.
        if (it->second && otherIt->second && *it->second != *otherIt->second) return false;
    }
    return true;
}


std::string
MetaMap::str(const std::string& indent) const
{
    std::ostringstream ostr;
    char sep[2] = { 0, 0 };
    for (ConstMetaIterator iter = beginMeta(); iter != endMeta(); ++iter) {
        ostr << sep << indent << iter->first;
        if (iter->second) {
            const std::string value = iter->second->str();
            if (!value.empty()) ostr << ": " << value;
        }
        sep[0] = '\n';
    }
    return ostr.str();
}

std::ostream&
operator<<(std::ostream& ostr, const MetaMap& metamap)
{
    ostr << metamap.str();
    return ostr;
}

} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb
