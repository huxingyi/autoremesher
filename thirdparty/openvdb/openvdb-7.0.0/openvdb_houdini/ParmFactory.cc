// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file ParmFactory.cc
/// @author FX R&D OpenVDB team

#include "ParmFactory.h"

#include <CH/CH_Manager.h>
#include <CMD/CMD_Args.h>
#include <CMD/CMD_Manager.h>
#include <GOP/GOP_GroupParse.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>
#include <GU/GU_Selection.h>
#include <GA/GA_AIFSharedStringTuple.h>
#include <GA/GA_Attribute.h>
#include <GA/GA_AttributeRef.h>
#include <HOM/HOM_Module.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Parm.h>
#include <PRM/PRM_SharedFunc.h>
#include <PY/PY_CPythonAPI.h>
#include <PY/PY_InterpreterAutoLock.h>
#include <PY/PY_Python.h>
#include <SOP/SOP_NodeParmsOptions.h>
#include <UT/UT_IntArray.h>
#include <UT/UT_WorkArgs.h>
#include <algorithm> // for std::for_each(), std::max(), std::remove(), std::sort()
#include <cstdint> // for std::uintptr_t()
#include <cstdlib> // for std::atoi()
#include <cstring> // for std::strcmp(), ::strdup()
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace houdini_utils {

namespace {

// PRM_SpareData token names

// SOP input index specifier
/// @todo Is there an existing constant for this token?
char const * const kSopInputToken = "sop_input";
// Parameter documentation wiki markup
char const * const kParmDocToken = "houdini_utils::doc";
// String-encoded GA_AttributeOwner
char const * const kAttrOwnerToken = "houdini_utils::attr_owner";
// Pointer to an AttrFilterFunc
char const * const kAttrFilterToken = "houdini_utils::attr_filter";


// Add an integer value (encoded into a string) to a PRM_SpareData map
// under the given token name.
inline void
setSpareInteger(PRM_SpareData* spare, const char* token, int value)
{
    if (spare && token) {
        spare->addTokenValue(token, std::to_string(value).c_str());
    }
}

// Retrieve the integer value with the given token name from a PRM_SpareData map.
// If no such token exists, return the specified default integer value.
inline int
getSpareInteger(const PRM_SpareData* spare, const char* token, int deflt = 0)
{
    if (!spare || !token) return deflt;
    char const * const str = spare->getValue(token);
    return str ? std::atoi(str) : deflt;
}


// Add a pointer (encoded into a string) to a PRM_SpareData map
// under the given token name.
inline void
setSparePointer(PRM_SpareData* spare, const char* token, const void* ptr)
{
    if (spare && token) {
        spare->addTokenValue(token,
            std::to_string(reinterpret_cast<std::uintptr_t>(ptr)).c_str());
    }
}

// Retrieve the pointer with the given token name from a PRM_SpareData map.
// If no such token exists, return the specified default pointer.
inline const void*
getSparePointer(const PRM_SpareData* spare, const char* token, const void* deflt = nullptr)
{
    if (!spare || !token) return deflt;
    if (sizeof(std::uintptr_t) > sizeof(unsigned long long)) {
        throw std::range_error{"houdini_utils::ParmFactory: can't decode pointer from string"};
    }
    if (const char* str = spare->getValue(token)) {
        auto intPtr = static_cast<std::uintptr_t>(std::stoull(str));
        return reinterpret_cast<void*>(intPtr);
    }
    return deflt;
}


// Copy elements from one spare data map to another,
// overwriting any existing elements with the same keys.
inline void
mergeSpareData(SpareDataMap& dst, const SpareDataMap& src)
{
    for (const auto& it: src) {
        dst[it.first] = it.second;
    }
}

} // anonymous namespace


ParmList&
ParmList::add(const PRM_Template& p)
{
    mParmVec.push_back(p);
    incFolderParmCount();
    return *this;
}


ParmList&
ParmList::add(const ParmFactory& f)
{
    add(f.get());
    return *this;
}


ParmList::SwitcherInfo*
ParmList::getCurrentSwitcher()
{
    SwitcherInfo* info = nullptr;
    if (!mSwitchers.empty()) {
        info = &mSwitchers.back();
    }
    return info;
}


ParmList&
ParmList::beginSwitcher(const std::string& token, const std::string& label)
{
    if (nullptr != getCurrentSwitcher()) {
        incFolderParmCount();
    }
    SwitcherInfo info;
    info.parmIdx = mParmVec.size();
    info.exclusive = false;
    mSwitchers.push_back(info);
    // Add a switcher parameter with the given token and name, but no folders.
    mParmVec.push_back(ParmFactory(PRM_SWITCHER, token, label).get());
    return *this;
}


ParmList&
ParmList::beginExclusiveSwitcher(const std::string& token, const std::string& label)
{
    if (nullptr != getCurrentSwitcher()) {
        incFolderParmCount();
    }
    SwitcherInfo info;
    info.parmIdx = mParmVec.size();
    info.exclusive = true;
    mSwitchers.push_back(info);
    // Add a switcher parameter with the given token and name, but no folders.
    mParmVec.push_back(ParmFactory(PRM_SWITCHER, token, label).get());
    return *this;
}


ParmList&
ParmList::endSwitcher()
{
    if (SwitcherInfo* info = getCurrentSwitcher()) {
        if (info->folders.empty()) {
            throw std::runtime_error("added switcher that has no folders");
        } else {
            // Replace the placeholder switcher parameter that was added to
            // mParmVec in beginSwitcher() with a new parameter that has
            // the correct folder count and folder info.
            PRM_Template& switcherParm = mParmVec[info->parmIdx];
            std::string token, label;
            if (const char* s = switcherParm.getToken()) token = s;
            if (const char* s = switcherParm.getLabel()) label = s;
            mParmVec[info->parmIdx] =
                ParmFactory(info->exclusive ? PRM_SWITCHER_EXCLUSIVE : PRM_SWITCHER,
                    token.c_str(), label.c_str())
                .setVectorSize(int(info->folders.size()))
                .setDefault(info->folders)
                .get();
        }
        mSwitchers.pop_back();
    } else {
        throw std::runtime_error("endSwitcher() called with no corresponding beginSwitcher()");
    }
    return *this;
}


ParmList&
ParmList::addFolder(const std::string& label)
{
    if (SwitcherInfo* info = getCurrentSwitcher()) {
        info->folders.push_back(PRM_Default(/*numParms=*/0, ::strdup(label.c_str())));
    } else {
        throw std::runtime_error("added folder to nonexistent switcher");
    }
    return *this;
}


void
ParmList::incFolderParmCount()
{
    if (SwitcherInfo* info = getCurrentSwitcher()) {
        if (info->folders.empty()) {
            throw std::runtime_error("added parameter to switcher that has no folders");
        } else {
            // If a parameter is added to this ParmList while a switcher with at least
            // one folder is active, increment the folder's parameter count.
            PRM_Default& def = *(info->folders.rbegin());
            def.setOrdinal(def.getOrdinal() + 1);
        }
    }
}


PRM_Template*
ParmList::get() const
{
    const size_t numParms = mParmVec.size();
    PRM_Template* ret = new PRM_Template[numParms + 1];
    for (size_t n = 0; n < numParms; ++n) {
        ret[n] = mParmVec[n];
    }
    return ret;
}


////////////////////////////////////////


struct ParmFactory::Impl
{
    Impl(const std::string& token, const std::string& label):
        callbackFunc(0),
        choicelist(nullptr),
        conditional(nullptr),
        defaults(PRMzeroDefaults),
        multiType(PRM_MULTITYPE_NONE),
        name(new PRM_Name(token.c_str(), label.c_str())),
        parmGroup(0),
        range(nullptr),
        spareData(nullptr),
        multiparms(nullptr),
        typeExtended(PRM_TYPE_NONE),
        vectorSize(1),
        invisible(false)
    {
        const_cast<PRM_Name*>(name)->harden();
    }

    static PRM_SpareData* getSopInputSpareData(size_t inp); ///< @todo return a const pointer?
    static void getAttrChoices(void* op, PRM_Name* choices, int maxChoices,
        const PRM_SpareData*, const PRM_Parm*);

    PRM_Callback               callbackFunc;
    const PRM_ChoiceList*      choicelist;
    const PRM_ConditionalBase* conditional;
    const PRM_Default*         defaults;
    std::string                tooltip;
    PRM_MultiType              multiType;
    const PRM_Name*            name;
    int                        parmGroup;
    const PRM_Range*           range;
    PRM_SpareData*             spareData;
    const PRM_Template*        multiparms;
    PRM_Type                   type;
    PRM_TypeExtended           typeExtended;
    int                        vectorSize;
    bool                       invisible;

    static PRM_SpareData* const sSOPInputSpareData[4];
};


PRM_SpareData* const ParmFactory::Impl::sSOPInputSpareData[4] = {
        &SOP_Node::theFirstInput, &SOP_Node::theSecondInput,
        &SOP_Node::theThirdInput, &SOP_Node::theFourthInput};


// Return one of the predefined PRM_SpareData maps that specify a SOP input number,
// or construct new PRM_SpareData if none exists for the given input number.
PRM_SpareData*
ParmFactory::Impl::getSopInputSpareData(size_t inp)
{
    if (inp < 4) return Impl::sSOPInputSpareData[inp];

    auto spare = new PRM_SpareData{SOP_Node::theFirstInput};
    spare->addTokenValue(kSopInputToken, std::to_string(inp).c_str());
    return spare;
}


// PRM_ChoiceGenFunc invoked by ParmFactory::setAttrChoiceList()
void
ParmFactory::Impl::getAttrChoices(void* op, PRM_Name* choices, int maxChoices,
    const PRM_SpareData* spare, const PRM_Parm* parm)
{
    if (!op || !choices || !parm) return;
    // This function can only be used in SOPs, because it calls SOP_Node::fillAttribNameMenu().
    if (static_cast<OP_Node*>(op)->getOpTypeID() != SOP_OPTYPE_ID) return;

    auto* sop = static_cast<SOP_Node*>(op);

    // Extract the SOP input number, the attribute class, and an optional
    // pointer to a filter functor from the spare data.
    const int inp = getSpareInteger(spare, kSopInputToken);
    const int attrOwner = getSpareInteger(spare, kAttrOwnerToken, GA_ATTRIB_INVALID);
    const auto* attrFilter =
        static_cast<const AttrFilterFunc*>(getSparePointer(spare, kAttrFilterToken));

    // Marshal pointers to the filter functor and the parameter and SOP for which this function
    // is being called into blind data that can be passed to SOP_Node::fillAttribNameMenu().
    struct AttrFilterData {
        const AttrFilterFunc* func;
        const PRM_Parm* parm;
        const SOP_Node* sop;
    };
    AttrFilterData cbData{attrFilter, parm, sop};

    // Define a filter callback function to be passed to SOP_Node::fillAttribNameMenu().
    // Because the latter uses a C-style callback mechanism, this callback must be
    // equivalent to a static function pointer (as a non-capturing lambda is).
    auto cb = [](const GA_Attribute* aAttr, void* aData) -> bool {
        if (!aAttr) return false;
        // Cast the blind data pointer supplied by SOP_Node::fillAttribNameMenu().
        const auto* data = static_cast<AttrFilterData*>(aData);
        if (!data || !data->func) return true; // no filter; accept all attributes
        // Invoke the filter functor and return the result.
        return (*(data->func))(*aAttr, *(data->parm), *(data->sop));
    };

    // Invoke SOP_Node::fillAttribNameMenu() for the appropriate attribute class.
    switch (attrOwner) {
        case GA_ATTRIB_VERTEX:
        case GA_ATTRIB_POINT:
        case GA_ATTRIB_PRIMITIVE:
        case GA_ATTRIB_DETAIL:
            if (cbData.func) {
                sop->fillAttribNameMenu(choices, maxChoices,
                    static_cast<GA_AttributeOwner>(attrOwner), inp, cb, &cbData);
            } else {
                sop->fillAttribNameMenu(choices, maxChoices,
                    static_cast<GA_AttributeOwner>(attrOwner), inp);
            }
            break;
        default: // all attributes
        {
            // To collect all classes of attributes, call SOP_Node::fillAttribNameMenu()
            // once for each class.  Each call appends zero or more PRM_Names to the list
            // as well as an end-of-list terminator.
            auto* head = choices;
            int count = 0, maxCount = maxChoices;
            for (auto owner:
                { GA_ATTRIB_VERTEX, GA_ATTRIB_POINT, GA_ATTRIB_PRIMITIVE, GA_ATTRIB_DETAIL })
            {
                int numAdded = (cbData.func ?
                    sop->fillAttribNameMenu(head, maxCount, owner, inp, cb, &cbData) :
                    sop->fillAttribNameMenu(head, maxCount, owner, inp));
                if (numAdded > 0) {
                    // SOP_Node::fillAttribNameMenu() returns the number of entries added
                    // to the list, not including the terminator.
                    // Advance the list head pointer so that the next entry to be added
                    // (if any) overwrites the terminator.
                    count += numAdded;
                    head += numAdded;
                    maxCount -= numAdded;
                }
            }
            if (count) {
                // Sort the list by name to reproduce the behavior of SOP_Node::allAttribMenu.
                std::sort(choices, choices + count,
                    [](const PRM_Name& n1, const PRM_Name& n2) {
                        return (0 > std::strcmp(n1.getToken(), n2.getToken()));
                    }
                );
            }
            break;
        }
    }
}


////////////////////////////////////////


ParmFactory::ParmFactory(PRM_Type type, const std::string& token, const std::string& label):
    mImpl(new Impl(token, label))
{
    mImpl->type = type;
}


ParmFactory::ParmFactory(PRM_MultiType multiType, const std::string& token,
    const std::string& label): mImpl(new Impl(token, label))
{
    mImpl->multiType = multiType;
}

ParmFactory&
ParmFactory::setCallbackFunc(const PRM_Callback& f) { mImpl->callbackFunc = f; return *this; }

ParmFactory&
ParmFactory::setChoiceList(const PRM_ChoiceList* c)
{
    mImpl->choicelist = c;

    if (c == &PrimGroupMenuInput1) {
        setSpareData(SOP_Node::getGroupSelectButton(GA_GROUP_PRIMITIVE,
            nullptr, 0, &SOP_Node::theFirstInput));
    } else if (c == &PrimGroupMenuInput2) {
        setSpareData(SOP_Node::getGroupSelectButton(GA_GROUP_PRIMITIVE,
            nullptr, 1, &SOP_Node::theSecondInput));
    } else if (c == &PrimGroupMenuInput3) {
        setSpareData(SOP_Node::getGroupSelectButton(GA_GROUP_PRIMITIVE,
            nullptr, 2, &SOP_Node::theThirdInput));
    } else if (c == &PrimGroupMenuInput4) {
        setSpareData(SOP_Node::getGroupSelectButton(GA_GROUP_PRIMITIVE,
            nullptr, 3, &SOP_Node::theFourthInput));
    }

    return *this;
}


/// @todo Merge this into setChoiceListItems() once the deprecated
/// setChoiceList() overloads have been removed.
ParmFactory&
ParmFactory::doSetChoiceList(PRM_ChoiceListType typ, const char* const* items, bool paired)
{
    size_t numItems = 0;
    for ( ; items[numItems] != nullptr; ++numItems) {}
    if (paired) numItems >>= 1;
    PRM_Name* copyOfItems = new PRM_Name[numItems + 1]; // extra item is list terminator
    if (paired) {
        for (size_t i = 0, n = 0; n < numItems; ++n, i += 2) {
            copyOfItems[n].setToken(items[i]);
            copyOfItems[n].setLabel(items[i+1]);
            copyOfItems[n].harden();
        }
    } else {
        for (size_t n = 0; n < numItems; ++n) {
            UT_String idx;
            idx.itoa(n);
            copyOfItems[n].setToken(idx.buffer());
            copyOfItems[n].setLabel(items[n]);
            copyOfItems[n].harden();
        }
    }
    mImpl->choicelist = new PRM_ChoiceList(typ, copyOfItems);
    return *this;
}

/// @todo Merge this into setChoiceListItems() once the deprecated
/// setChoiceList() overloads have been removed.
ParmFactory&
ParmFactory::doSetChoiceList(PRM_ChoiceListType typ,
    const std::vector<std::string>& items, bool paired)
{
    const size_t numItems = items.size() >> (paired ? 1 : 0);
    PRM_Name* copyOfItems = new PRM_Name[numItems + 1]; // extra item is list terminator
    if (paired) {
        for (size_t i = 0, n = 0; n < numItems; ++n, i += 2) {
            copyOfItems[n].setToken(items[i].c_str());
            copyOfItems[n].setLabel(items[i+1].c_str());
            copyOfItems[n].harden();
        }
    } else {
        for (size_t n = 0; n < numItems; ++n) {
            UT_String idx;
            idx.itoa(n);
            copyOfItems[n].setToken(idx.buffer());
            copyOfItems[n].setLabel(items[n].c_str());
            copyOfItems[n].harden();
        }
    }
    mImpl->choicelist = new PRM_ChoiceList(typ, copyOfItems);
    return *this;
}

ParmFactory&
ParmFactory::setChoiceList(PRM_ChoiceListType typ, const char* const* items, bool paired)
{
    return doSetChoiceList(typ, items, paired);
}

ParmFactory&
ParmFactory::setChoiceList(PRM_ChoiceListType typ,
    const std::vector<std::string>& items, bool paired)
{
    return doSetChoiceList(typ, items, paired);
}

ParmFactory&
ParmFactory::setChoiceListItems(PRM_ChoiceListType typ, const char* const* items)
{
    return doSetChoiceList(typ, items, /*paired=*/true);
}

ParmFactory&
ParmFactory::setChoiceListItems(PRM_ChoiceListType typ, const std::vector<std::string>& items)
{
    return doSetChoiceList(typ, items, /*paired=*/true);
}


ParmFactory&
ParmFactory::setGroupChoiceList(size_t inputIndex, PRM_ChoiceListType typ)
{
    mImpl->choicelist = new PRM_ChoiceList(typ, PrimGroupMenu.getChoiceGenerator());

    setSpareData(SOP_Node::getGroupSelectButton(GA_GROUP_PRIMITIVE, nullptr,
        static_cast<int>(inputIndex), mImpl->getSopInputSpareData(inputIndex)));

    return *this;
}


ParmFactory&
ParmFactory::setAttrChoiceList(size_t inputIndex, GA_AttributeOwner attrOwner,
    PRM_ChoiceListType typ, AttrFilterFunc attrFilter)
{
    setChoiceList(new PRM_ChoiceList{typ, Impl::getAttrChoices});

    mImpl->spareData = new PRM_SpareData;
    setSpareInteger(mImpl->spareData, kSopInputToken, int(inputIndex));
    setSpareInteger(mImpl->spareData, kAttrOwnerToken, static_cast<int>(attrOwner));
    if (attrFilter) {
        setSparePointer(mImpl->spareData, kAttrFilterToken, new AttrFilterFunc{attrFilter});
    }

    return *this;
}


ParmFactory&
ParmFactory::setConditional(const PRM_ConditionalBase* c) { mImpl->conditional = c; return *this; }

ParmFactory&
ParmFactory::setDefault(fpreal f, const char* s, CH_StringMeaning meaning)
{
    mImpl->defaults = new PRM_Default(f, s, meaning);
    return *this;
}

ParmFactory&
ParmFactory::setDefault(const std::string& s, CH_StringMeaning meaning)
{
    mImpl->defaults = new PRM_Default(0.0, ::strdup(s.c_str()), meaning);
    return *this;
}

ParmFactory&
ParmFactory::setDefault(const std::vector<fpreal>& v)
{
    const size_t numDefaults = v.size();
    PRM_Default* defaults = new PRM_Default[numDefaults + 1];
    for (size_t n = 0; n < numDefaults; ++n) {
        defaults[n] = PRM_Default(v[n]);
    }
    mImpl->defaults = defaults;
    return *this;
}

ParmFactory&
ParmFactory::setDefault(const std::vector<PRM_Default>& defaults)
{
    const size_t numDefaults = defaults.size();
    PRM_Default* copyOfDefaults = new PRM_Default[numDefaults + 1];
    for (size_t n = 0; n < numDefaults; ++n) {
        copyOfDefaults[n] = defaults[n];
    }
    mImpl->defaults = copyOfDefaults;
    return *this;
}

ParmFactory&
ParmFactory::setDefault(const PRM_Default* d) { mImpl->defaults = d; return *this; }

ParmFactory&
ParmFactory::setTooltip(const char* t) { mImpl->tooltip = (t ? t : ""); return *this; }

ParmFactory&
ParmFactory::setHelpText(const char* t) { return setTooltip(t); }

ParmFactory&
ParmFactory::setDocumentation(const char* doc)
{
    if (!mImpl->spareData) { mImpl->spareData = new PRM_SpareData; }
    mImpl->spareData->addTokenValue(kParmDocToken, ::strdup(doc ? doc : ""));
    return *this;
}

ParmFactory&
ParmFactory::setParmGroup(int n) { mImpl->parmGroup = n; return *this; }

ParmFactory&
ParmFactory::setRange(PRM_RangeFlag minFlag, fpreal minVal, PRM_RangeFlag maxFlag, fpreal maxVal)
{
    mImpl->range = new PRM_Range(minFlag, minVal, maxFlag, maxVal);
    return *this;
}

ParmFactory&
ParmFactory::setRange(const std::vector<PRM_Range>& ranges)
{
    const size_t numRanges = ranges.size();
    PRM_Range* copyOfRanges = new PRM_Range[numRanges + 1];
    for (size_t n = 0; n < numRanges; ++n) {
        copyOfRanges[n] = ranges[n];
    }
    mImpl->range = copyOfRanges;
    return *this;
}

ParmFactory&
ParmFactory::setRange(const PRM_Range* r) { mImpl->range = r; return *this; }

ParmFactory&
ParmFactory::setSpareData(const SpareDataMap& items)
{
    if (!items.empty()) {
        if (!mImpl->spareData) { mImpl->spareData = new PRM_SpareData; }
        for (SpareDataMap::const_iterator i = items.begin(), e = items.end(); i != e; ++i) {
            mImpl->spareData->addTokenValue(i->first.c_str(), i->second.c_str());
        }
    }
    return *this;
}

ParmFactory&
ParmFactory::setSpareData(const PRM_SpareData* d)
{
    if (!d) {
        if (mImpl->spareData) mImpl->spareData->clear();
    } else {
        mImpl->spareData = new PRM_SpareData{*d};
    }
    return *this;
}

ParmFactory&
ParmFactory::setMultiparms(const ParmList& p) { mImpl->multiparms = p.get(); return *this; }

ParmFactory&
ParmFactory::setTypeExtended(PRM_TypeExtended t) { mImpl->typeExtended = t; return *this; }

ParmFactory&
ParmFactory::setVectorSize(int n) { mImpl->vectorSize = n; return *this; }

ParmFactory&
ParmFactory::setInvisible() { mImpl->invisible = true; return *this; }

PRM_Template
ParmFactory::get() const
{
#ifdef SESI_OPENVDB
    // Help is maintained separately within Houdini
    const char *tooltip = nullptr;
#else
    const char *tooltip = mImpl->tooltip.c_str();
#endif

    PRM_Template parm;
    if (mImpl->multiType != PRM_MULTITYPE_NONE) {
        parm.initMulti(
            mImpl->multiType,
            const_cast<PRM_Template*>(mImpl->multiparms),
            PRM_Template::PRM_EXPORT_MIN,
            fpreal(mImpl->vectorSize),
            const_cast<PRM_Name*>(mImpl->name),
            const_cast<PRM_Default*>(mImpl->defaults),
            const_cast<PRM_Range*>(mImpl->range),
            0, // no callback
            mImpl->spareData,
            tooltip ? ::strdup(tooltip) : nullptr,
            const_cast<PRM_ConditionalBase*>(mImpl->conditional));
    } else {
        parm.initialize(
            mImpl->type,
            mImpl->typeExtended,
            PRM_Template::PRM_EXPORT_MIN,
            mImpl->vectorSize,
            const_cast<PRM_Name*>(mImpl->name),
            const_cast<PRM_Default*>(mImpl->defaults),
            const_cast<PRM_ChoiceList*>(mImpl->choicelist),
            const_cast<PRM_Range*>(mImpl->range),
            mImpl->callbackFunc,
            mImpl->spareData,
            mImpl->parmGroup,
            tooltip ? ::strdup(tooltip) : nullptr,
            const_cast<PRM_ConditionalBase*>(mImpl->conditional));
    }
    if (mImpl->invisible) {
        parm.setInvisible(true);
    }
    return parm;
}


////////////////////////////////////////


namespace {

/// @brief Output wiki markup documentation to the given stream for
/// a (possibly nested) list of parameters.
/// @return the address of the parameter list entry one past the last parameter
/// that was documented
inline const PRM_Template*
documentParms(std::ostream& os, PRM_Template const * const parmList, int level = 0,
    int numParms = std::numeric_limits<int>::max())
{
    if (level > 10) return parmList; // probably something wrong if there are 10 levels of nesting

    auto indent = [&level]() { return std::string(4 * std::max(0, level), ' '); };

    bool hasHeading = false;
    const PRM_Template* parm = parmList;
    for (int parmIdx = 0;
        parm && (parmIdx < numParms) && (parm->getType() != PRM_LIST_TERMINATOR);
        ++parmIdx, ++parm)
    {
        const auto parmType = parm->getType();
        if (parmType == PRM_LABEL || parm->getInvisible()) continue;

        const auto parmLabel = [parm]() {
            UT_String lbl = parm->getLabel();
            // Houdini's wiki markup parser aggressively expands square-bracketed text into
            // hyperlinks.  The following is one way to suppress that behavior, given that
            // there doesn't appear to be any native escaping mechanism.  Since we might want
            // to use brackets--but probably not hyperlinks--in parameter labels, and since
            // hacks like this don't render correctly in the parameter pane, we unconditionally
            // "escape" brackets in parameter labels, but only in the documentation markup.
            lbl.substitute("[", "&#91;", /*all=*/true); // 91 is the ISO-8859 code for "["
            lbl.substitute("]", "&#93;", /*all=*/true); // 93 is the ISO-8859 code for "]"
            return lbl;
        }();
        const bool hasLabel = parmLabel.isstring();

        if ((parmType == PRM_SEPARATOR) || ((parmType == PRM_HEADING) && !hasLabel)) {
            // A separator or empty heading removes one level of nesting.
            // (There are no begin/end grouping indicators, so this is just a best guess.)
            level = std::max(0, level - 1);
            hasHeading = false;
            continue;
        }

        UT_String parmDoc;
        const PRM_SpareData* const spare = parm->getSparePtr();
        if (spare && spare->getValue(kParmDocToken)) {
            // If the parameter was documented with setDocumentation(), use that text.
            // (This relies on kParmDocToken not being paired with nullptr.
            // ParmFactory::setDocumentation(), at least, ensures that it isn't.)
            parmDoc = spare->getValue(kParmDocToken);
            // If the text is empty, suppress this parameter.
            if (!parmDoc.isstring()) continue;
        } else {
            // Otherwise, if the parameter has a tool tip, use that.
            parmDoc = parm->getHelpText();

            // If the parameter has no tool tip but has a choice list, list the choices
            // (except if the parameter is a toggle--toggles seem to be implemented as
            // on/off choice lists).
            if (!parmDoc.isstring() && (parmType.getOrdinalType() != PRM_Type::PRM_ORD_TOGGLE)) {
                if (const PRM_ChoiceList* choices = parm->getChoiceListPtr()) {
                    for (const PRM_Name* choiceName =
                        const_cast<PRM_ChoiceList*>(choices)->choiceNamesPtr();
                        choiceName && choiceName->getToken(); ++choiceName)
                    {
                        if (const char* n = choiceName->getLabel()) {
                            parmDoc += (std::string{"* "} + n + "\n").c_str();
                        }
                    }
                }
            }
            // Otherwise, show the parameter without documentation.
            /// @todo Just suppress undocumented parameters?
            if ((parmType != PRM_HEADING) && !parm->isMultiType() && !parmDoc.isstring()) {
                parmDoc = "&nbsp;";
            }
        }
        const bool hasDoc = parmDoc.isstring();

        if (parmType == PRM_HEADING) {
            // Decrement the nesting level for a heading label if there was
            // a previous heading.  (This assumes that headings aren't nested.)
            if (hasHeading) --level;
            hasHeading = true;
            os << indent() << parmLabel.c_str() << ":\n";
            ++level; // increment the nesting level below a heading
            if (hasDoc) {
                parmDoc.substitute("\n", ("\n" + indent()).c_str(), /*all=*/true);
                os << indent() << parmDoc.c_str() << "\n\n";
            }

        } else if ((parmType == PRM_SWITCHER) || (parmType == PRM_SWITCHER_EXCLUSIVE)
            || (parmType == PRM_SWITCHER_REFRESH))
        {
            // The vector size of a switcher is the number of folders.
            const int numFolders = parm->getVectorSize();
            const PRM_Template* firstFolderParm = parm + 1;
            const PRM_Default* deflt = parm->getFactoryDefaults();
            for (int folder = 0; deflt && (folder < numFolders); ++folder, ++deflt) {
                // The default values of a switcher are per-folder (member count, title) pairs.
                const int numMembers = deflt->getOrdinal();
                char const * const title = deflt->getString();
                if (title) {
                    // If the folder has a title, show the title and increment
                    // the nesting level for the folder's members.
                    os << indent() << title << ":\n";
                    ++level;
                }
                firstFolderParm = documentParms(os, firstFolderParm, level, numMembers);
                if (title) { --level; }
            }
            parm = PRM_Template::getEndOfSwitcher(parm);
            --parm; // decrement to compensate for loop increment

        } else if (parm->isMultiType()) {
            if (hasLabel) { os << indent() << parmLabel.c_str() << ":\n"; }
            ++level; // increment the nesting level for the members of a multiparm
            if (hasDoc) {
                // Add the multiparm's documentation.
                parmDoc.substitute("\n", ("\n" + indent()).c_str(), /*all=*/true);
                os << indent() << parmDoc.c_str() << "\n\n";
            }
            // Add documentation for the members of the multiparm
            // (but not for members of native types such as ramps,
            // since those members have only generic descriptions).
            if ((parm->getMultiType() != PRM_MULTITYPE_RAMP_FLT)
                && (parm->getMultiType() != PRM_MULTITYPE_RAMP_RGB))
            {
                if (PRM_Template const * const subparms = parm->getMultiParmTemplate()) {
                    documentParms(os, subparms, level);
                }
            }
            --level;

        } else if (hasLabel && hasDoc) {
            // Add this parameter only if it has both a label and documentation.
            os << indent() << parmLabel.c_str() << ":\n";
            ++level;
            parmDoc.substitute("\n", ("\n" + indent()).c_str(), /*all=*/true);
            os << indent() << parmDoc.c_str() << "\n\n";
            --level;
        }
    }
    return parm;
}


/// @brief Operator class that adds the help link. Used by the OpFactory.
class OP_OperatorDW: public OP_Operator
{
public:
    OP_OperatorDW(
        OpFactory::OpFlavor flavor,
        const char* name,
        const char* english,
        OP_Constructor construct,
        PRM_Template* multiparms,
        const char* operatorTableName,
        unsigned minSources,
        unsigned maxSources,
        CH_LocalVariable* variables,
        unsigned flags,
        const char** inputlabels,
        const std::string& helpUrl,
        const std::string& doc)
        : OP_Operator(name, english, construct, multiparms,
            operatorTableName,
            minSources, maxSources, variables, flags, inputlabels)
        , mHelpUrl(helpUrl)
    {
#ifndef SESI_OPENVDB
        // Generate help page markup for this operator if the help URL is empty
        // and the documentation string is nonempty.
        if (mHelpUrl.empty() && !doc.empty()) {
            UT_String flavorStr{OpFactory::flavorToString(flavor)};
            flavorStr.toLower();

            std::ostringstream os;
            os << "= " << english << " =\n\n"
                << "#type: node\n"
                << "#context: " << flavorStr << "\n"
                << "#internal: " << name << "\n\n"
                << doc << "\n\n";
            {
                std::ostringstream osParm;
                documentParms(osParm, multiparms);
                const std::string parmDoc = osParm.str();
                if (!parmDoc.empty()) {
                    os << "@parameters\n\n" << parmDoc;
                }
            }

            const_cast<std::string*>(&mDoc)->assign(os.str());
        }
#endif
    }

    ~OP_OperatorDW() override {}

    bool getOpHelpURL(UT_String& url) override { url = mHelpUrl; return !mHelpUrl.empty(); }

    bool getHDKHelp(UT_String& txt) const override
    {
        if (!mHelpUrl.empty()) return false; // URL takes precedence over help text

        txt = mDoc;
        txt.hardenIfNeeded();
        return !mDoc.empty();
    }

#ifndef SESI_OPENVDB
    bool getVersion(UT_String &version) override
    {
        auto it = spareData().find("operatorversion");
        if (it != spareData().end()) {
            version = it->second;
            return true;
        }

        return OP_Operator::getVersion(version);
    }
#endif

    const SpareDataMap& spareData() const { return mSpareData; }
    SpareDataMap& spareData() { return mSpareData; }

private:
    const std::string mHelpUrl, mDoc;
    SpareDataMap mSpareData;
};


class OpFactoryVerb: public SOP_NodeVerb
{
public:
    OpFactoryVerb(const std::string& name, SOP_NodeVerb::CookMode cookMode,
        const OpFactory::CacheAllocFunc& allocator, PRM_Template* parms)
        : mName{name}
        , mCookMode{cookMode}
        , mAllocator{allocator}
        , mParms{parms}
    {}

    SOP_NodeParms* allocParms() const override { return new SOP_NodeParmsOptions{mParms}; }
    SOP_NodeCache* allocCache() const override { return mAllocator(); }

    UT_StringHolder name() const override { return mName; }
    CookMode cookMode(const SOP_NodeParms*) const override { return mCookMode; }

    void cook(const CookParms& cookParms) const override
    {
        if (auto* cache = static_cast<SOP_NodeCacheOptions*>(cookParms.cache())) {
            cache->doCook(this, cookParms);
        }
    }

private:
    std::string mName;
    SOP_NodeVerb::CookMode mCookMode;
    OpFactory::CacheAllocFunc mAllocator;
    PRM_Template* mParms;
}; // class OpFactoryVerb


} // anonymous namespace


////////////////////////////////////////


struct OpFactory::Impl
{
    Impl(const std::string& english, OP_Constructor& constructor, PRM_Template* parms,
        OP_OperatorTable& table, OpFactory::OpFlavor flavor):
        mFlavor(flavor),
        mEnglish(english),
        mConstruct(constructor),
        mTable(&table),
        mParms(parms),
        mObsoleteParms(nullptr),
        mMaxSources(0),
        mVariables(nullptr),
        mFlags(0)
    {
    }

    ~Impl()
    {
        std::for_each(mInputLabels.begin(), mInputLabels.end(), ::free);
        // Note: In get(), mOptInputLabels are appended to mInputLabels.
    }

    void init(const OpFactory& factory, OpPolicyPtr policy)
    {
        // Because mPolicy is supplied by this Impl's parent OpFactory
        // (which knows which OpPolicy subclass to use), initialization
        // of the following members must be postponed until both
        // the OpFactory and this Impl have been fully constructed.
        mPolicy = policy;
        mName = mPolicy->getName(factory);
        mLabelName = mPolicy->getLabelName(factory);
        mIconName = mPolicy->getIconName(factory);
        mHelpUrl = mPolicy->getHelpURL(factory);
        mFirstName = mPolicy->getFirstName(factory);
        mTabSubMenuPath = mPolicy->getTabSubMenuPath(factory);

        initScripting();
    }

    OP_OperatorDW* get()
    {
        // Get the number of required inputs.
        const unsigned minSources = unsigned(mInputLabels.size());

        // Append optional input labels to required input labels.
        mInputLabels.insert(mInputLabels.end(),
            mOptInputLabels.begin(), mOptInputLabels.end());

        // Ensure that the maximum number of inputs is at least as large
        // as the number of labeled inputs.
        mMaxSources = std::max<unsigned>(unsigned(mInputLabels.size()), mMaxSources);

        mInputLabels.push_back(nullptr);

        OP_OperatorDW* op = new OP_OperatorDW(mFlavor, mName.c_str(), mLabelName.c_str(),
            mConstruct, mParms,
            UTisstring(mOperatorTableName.c_str()) ? mOperatorTableName.c_str() : 0,
            minSources, mMaxSources, mVariables, mFlags,
            const_cast<const char**>(&mInputLabels[0]), mHelpUrl, mDoc);

        if (!mIconName.empty()) op->setIconName(mIconName.c_str());

        if (!mTabSubMenuPath.empty()) op->setOpTabSubMenuPath(mTabSubMenuPath.c_str());

        if (mObsoleteParms != nullptr) op->setObsoleteTemplates(mObsoleteParms);

        if (mVerb) SOP_NodeVerb::registerVerb(mVerb);

        mergeSpareData(op->spareData(), mSpareData);

        return op;
    }

    void initScripting()
    {
        // Install an HScript command to retrieve spare data from operators.
        if (auto* cmgr = CMD_Manager::getContext()) {
            if (!cmgr->isCommandDefined(kSpareDataCmdName)) {
                cmgr->installCommand(kSpareDataCmdName, "", cmdGetOperatorSpareData);
            }
        }

        // Install Python functions to retrieve spare data from operators.
        static bool sDidInstallHOMModule = false;
        if (!sDidInstallHOMModule) {
            // Install a _dwhoudiniutils module with a NodeType_spareData() function.
            static PY_PyMethodDef sMethods[] = {
                {"NodeType_spareData", homGetOperatorSpareData, PY_METH_VARARGS(), ""},
                { nullptr, nullptr, 0, nullptr }
            };
            {
                PY_InterpreterAutoLock interpreterLock;
                PY_Py_InitModule("_dwhoudiniutils", sMethods);
                sDidInstallHOMModule = true;
            }

            // Add methods to the hou.NodeType class.
            PYrunPythonStatementsAndExpectNoErrors("\
def _spareData(self, name):\n\
    '''\n\
    spareData(name) -> str or None\n\
    \n\
    Return the spare data with the given name, or None\n\
    if no data with that name is defined for this node type.\n\
    \n\
    Currently, only node types defined with OpenVDB's OpFactory\n\
    can have spare data.  See www.openvdb.org for more information.\n\
    '''\n\
    import _dwhoudiniutils\n\
    return _dwhoudiniutils.NodeType_spareData(self.category().name(), self.name(), name)\n\
\n\
def _spareDataDict(self):\n\
    '''\n\
    spareDataDict() -> dict of str to str\n\
    \n\
    Return a dictionary of the spare data for this node type.\n\
    \n\
    Currently, only node types defined with OpenVDB's OpFactory\n\
    can have spare data.  See www.openvdb.org for more information.\n\
    '''\n\
    import _dwhoudiniutils\n\
    return _dwhoudiniutils.NodeType_spareData(self.category().name(), self.name())\n\
\n\
nt = __import__('hou').NodeType\n\
nt.spareData = _spareData\n\
nt.spareDataDict = _spareDataDict\n\
del nt, _spareData, _spareDataDict\n");
        }
    }

    // HScript callback to retrieve spare data from an OP_OperatorDW-derived operator
    static void cmdGetOperatorSpareData(CMD_Args& args)
    {
        // The operator's network type ("Sop", "Dop", etc.)
        const char* const networkType = args[1];
        // The operator's name
        const char* const opName = args[2];
        // An optional spare data token
        const char* const token = args[3];

        if (!networkType || !opName) {
            /// @todo Install this as a command.help file?
            args.out() << kSpareDataCmdName << "\n\
\n\
    List spare data associated with an operator type.\n\
\n\
    USAGE\n\
      " << kSpareDataCmdName << " <networktype> <opname> [<token>]\n\
\n\
    When the token is omitted, all (token, value) pairs\n\
    associated with the operator type are displayed.\n\
\n\
    Currently, only operator types defined with OpenVDB's OpFactory\n\
    can have spare data.  See www.openvdb.org for more information.\n\
\n\
    EXAMPLES\n\
      > " << kSpareDataCmdName << " Sop DW_OpenVDBConvert\n\
        lists all spare data associated with the Convert VDB SOP\n\
      > " << kSpareDataCmdName << " Sop DW_OpenVDBClip nativename\n\
        displays the VDB Clip SOP's native name\n\
\n";
            return;
        }

        // Retrieve the operator table for the specified network type.
        const OP_OperatorTable* table = nullptr;
        {
            OP_OperatorTableList opTables;
            OP_OperatorTable::getAllOperatorTables(opTables);
            for (const auto& t: opTables) {
                if (t && (t->getName() == networkType)) {
                    table = t;
                    break;
                }
            }
        }
        if (table) {
            if (const auto* op = table->getOperator(opName)) {
                // Retrieve the operator's spare data map.
                // (The map is empty for operators that don't support spare data.)
                const auto& spare = getOperatorSpareData(*op);
                if (token) {
                    // If a token was provided and it exists in the map,
                    // print the corresponding value.
                    const auto it = spare.find(token);
                    if (it != spare.end()) {
                        args.out() << it->second << "\n";
                    }
                } else {
                    // If no token was provided, print all of the operator's
                    // (token, value) pairs.
                    for (const auto& it: spare) {
                        args.out() << it.first << " " << it.second << "\n";
                    }
                }
            }
        }
    }

    // Python callback to retrieve spare data from an OP_OperatorDW-derived operator
    static PY_PyObject* homGetOperatorSpareData(PY_PyObject* self, PY_PyObject* args)
    {
        // The operator's network type ("Sop", "Dop", etc.)
        const char* networkType = nullptr;
        // The operator's name
        const char* opName = nullptr;
        // An optional spare data token
        const char* token = nullptr;
        if (!PY_PyArg_ParseTuple(args, "ss|s", &networkType, &opName, &token)) {
            return nullptr;
        }

        if (!networkType || !opName) {
            return PY_Py_None();
        }

        try {
            HOM_AutoLock homLock;

            // Retrieve the operator table for the specified network type.
            const OP_OperatorTable* table = nullptr;
            {
                OP_OperatorTableList opTables;
                OP_OperatorTable::getAllOperatorTables(opTables);
                for (const auto& t: opTables) {
                    if (t && (t->getName() == networkType)) {
                        table = t;
                        break;
                    }
                }
            }
            if (table) {
                if (const auto* op = table->getOperator(opName)) {
                    // Retrieve the operator's spare data map.
                    // (The map is empty for operators that don't support spare data.)
                    const auto& spare = getOperatorSpareData(*op);
                    if (token) {
                        // If a token was provided and it exists in the map,
                        // return the corresponding value.
                        const auto it = spare.find(token);
                        if (it != spare.end()) {
                            return PY_Py_BuildValue("s", it->second.c_str());
                        }
                    } else {
                        // If no token was provided, return a dictionary
                        // of all of the operator's (token, value) pairs.
                        if (auto* dict = PY_Py_BuildValue("{}")) {
                            for (const auto& it: spare) {
                                PY_PyDict_SetItemString(dict, it.first.c_str(),
                                    PY_Py_BuildValue("s", it.second.c_str()));
                            }
                            return dict;
                        }
                    }
                }
            }
        } catch (HOM_Error& err) {
        }
        return PY_Py_None();
    }


    OpPolicyPtr mPolicy; // polymorphic, so stored by pointer
    OpFactory::OpFlavor mFlavor;
    std::string mEnglish, mName, mLabelName, mIconName, mHelpUrl, mDoc, mOperatorTableName;
    std::string mFirstName, mTabSubMenuPath;
    OP_Constructor mConstruct;
    OP_OperatorTable* mTable;
    PRM_Template *mParms, *mObsoleteParms;
    unsigned mMinSources;
    unsigned mMaxSources;
    CH_LocalVariable* mVariables;
    unsigned mFlags;
    std::vector<std::string> mAliases;
    std::vector<char*> mInputLabels, mOptInputLabels;
    OpFactoryVerb* mVerb = nullptr;
    bool mInvisible = false;
    SpareDataMap mSpareData;

    static constexpr auto* kSpareDataCmdName = "opsparedata";
};


OpFactory::OpFactory(const std::string& english, OP_Constructor ctor,
    ParmList& parms, OP_OperatorTable& table, OpFlavor flavor)
{
    this->init(OpPolicyPtr(new OpPolicy), english, ctor, parms, table, flavor);
}


OpFactory::~OpFactory()
{
    mImpl->mTable->addOperator(mImpl->get());

    for (size_t n = 0, N = mImpl->mAliases.size(); n < N; ++n) {
        const std::string& alias = mImpl->mAliases[n];
        if (!alias.empty()) {
            mImpl->mTable->setOpAlias(/*original=*/mImpl->mName.c_str(), alias.c_str());
        }
    }

    // apply first name if set

    if (!mImpl->mFirstName.empty()) {
        mImpl->mTable->setOpFirstName(mImpl->mName.c_str(), mImpl->mFirstName.c_str());
    }

    // hide node if marked as invisible

    if (mImpl->mInvisible) {
        mImpl->mTable->addOpHidden(mImpl->mName.c_str());
    }
}


void
OpFactory::init(OpPolicyPtr policy, const std::string& english, OP_Constructor ctor,
    ParmList& parms, OP_OperatorTable& table, OpFlavor flavor)
{
    mImpl.reset(new Impl(english, ctor, parms.get(), table, flavor));
    mImpl->init(*this, policy);
}


//static
std::string
OpFactory::flavorToString(OpFlavor flavor)
{
    switch (flavor) {
        case SOP: return "SOP";
        case POP: return "POP";
        case ROP: return "ROP";
        case VOP: return "VOP";
        case HDA: return "HDA";
    }
    return "";
}


OpFactory::OpFlavor
OpFactory::flavor() const
{
    return mImpl->mFlavor;
}


std::string
OpFactory::flavorString() const
{
    return flavorToString(mImpl->mFlavor);
}


const std::string&
OpFactory::name() const
{
    return mImpl->mName;
}


const std::string&
OpFactory::english() const
{
    return mImpl->mEnglish;
}


const std::string&
OpFactory::iconName() const
{
    return mImpl->mIconName;
}


const std::string&
OpFactory::helpURL() const
{
    return mImpl->mHelpUrl;
}


const std::string&
OpFactory::documentation() const
{
    return mImpl->mDoc;
}


const OP_OperatorTable&
OpFactory::table() const
{
    return *mImpl->mTable;
}


OP_OperatorTable&
OpFactory::table()
{
    return *mImpl->mTable;
}


OpFactory&
OpFactory::addAlias(const std::string& english)
{
    if (!english.empty()) {
        this->addAliasVerbatim(mImpl->mPolicy->getName(*this, english));
    }
    return *this;
}


OpFactory&
OpFactory::addAliasVerbatim(const std::string& name)
{
    if (!name.empty()) {
        mImpl->mAliases.push_back(name);
    }
    return *this;
}


OpFactory&
OpFactory::setDocumentation(const std::string& doc)
{
    mImpl->mDoc = doc;
    return *this;
}


OpFactory&
OpFactory::addInput(const std::string& name)
{
    mImpl->mInputLabels.push_back(::strdup(name.c_str()));
    return *this;
}


OpFactory&
OpFactory::addOptionalInput(const std::string& name)
{
    mImpl->mOptInputLabels.push_back(::strdup(name.c_str()));
    return *this;
}


OpFactory&
OpFactory::setMaxInputs(unsigned n) { mImpl->mMaxSources = n; return *this; }


OpFactory&
OpFactory::setObsoleteParms(const ParmList& parms)
{
    if (mImpl->mObsoleteParms != nullptr) delete mImpl->mObsoleteParms;
    mImpl->mObsoleteParms = parms.get();
    return *this;
}


OpFactory&
OpFactory::setLocalVariables(CH_LocalVariable* v) { mImpl->mVariables = v; return *this; }


OpFactory&
OpFactory::setFlags(unsigned f) { mImpl->mFlags = f; return *this; }


OpFactory&
OpFactory::setInternalName(const std::string& name)
{
    mImpl->mName = name;
    return *this;
}


OpFactory&
OpFactory::setOperatorTable(const std::string& name)
{
    mImpl->mOperatorTableName = name;
    return *this;
}


OpFactory&
OpFactory::setVerb(SOP_NodeVerb::CookMode cookMode, const CacheAllocFunc& allocator)
{
    if (flavor() != SOP) {
        throw std::runtime_error{"expected operator of type SOP, got " + flavorToString(flavor())};
    }
    if (!allocator) throw std::invalid_argument{"must provide a cache allocator function"};

    mImpl->mVerb = new OpFactoryVerb{name(), cookMode, allocator, mImpl->mParms};

    return *this;
}


OpFactory&
OpFactory::setInvisible()
{
    mImpl->mInvisible = true;
    return *this;
}


OpFactory&
OpFactory::addSpareData(const SpareDataMap& spare)
{
    mergeSpareData(mImpl->mSpareData, spare);
    return *this;
}


////////////////////////////////////////


const SpareDataMap&
getOperatorSpareData(const OP_Operator& op)
{
    static const SpareDataMap sNoSpareData;
    if (const auto* opdw = dynamic_cast<const OP_OperatorDW*>(&op)) {
        return opdw->spareData();
    }
    return sNoSpareData;
}

void
addOperatorSpareData(OP_Operator& op, SpareDataMap& spare)
{
    if (auto* opdw = dynamic_cast<OP_OperatorDW*>(&op)) {
        mergeSpareData(opdw->spareData(), spare);
    } else {
        throw std::runtime_error("spare data cannot be added to the \""
            + op.getName().toStdString() + "\" operator");
    }
}


////////////////////////////////////////


//virtual
std::string
OpPolicy::getName(const OpFactory&, const std::string& english)
{
    UT_String s(english);
    s.forceValidVariableName();
    return s.toStdString();
}


//virtual
std::string
OpPolicy::getLabelName(const OpFactory& factory)
{
    return factory.english();
}


////////////////////////////////////////


const PRM_ChoiceList PrimGroupMenuInput1 = SOP_Node::primGroupMenu;
const PRM_ChoiceList PrimGroupMenuInput2 = SOP_Node::primGroupMenu;
const PRM_ChoiceList PrimGroupMenuInput3 = SOP_Node::primGroupMenu;
const PRM_ChoiceList PrimGroupMenuInput4 = SOP_Node::primGroupMenu;

const PRM_ChoiceList PrimGroupMenu = SOP_Node::primGroupMenu;


} // namespace houdini_utils
