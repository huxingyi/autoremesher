// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file ParmFactory.h
/// @author FX R&D OpenVDB team
///
/// @brief A collection of factory methods and helper functions
/// to simplify Houdini plugin development and maintenance.

#ifndef HOUDINI_UTILS_PARM_FACTORY_HAS_BEEN_INCLUDED
#define HOUDINI_UTILS_PARM_FACTORY_HAS_BEEN_INCLUDED

#include <UT/UT_Version.h>
#include <GA/GA_Attribute.h>
#include <OP/OP_AutoLockInputs.h>
#include <OP/OP_Operator.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_SpareData.h>
#include <SOP/SOP_Node.h>
#include <SOP/SOP_NodeVerb.h>
#if defined(PRODDEV_BUILD) || defined(DWREAL_IS_DOUBLE)
  // OPENVDB_HOUDINI_API, which has no meaning in a DWA build environment but
  // must at least exist, is normally defined by including openvdb/Platform.h.
  // For DWA builds (i.e., if either PRODDEV_BUILD or DWREAL_IS_DOUBLE exists),
  // that introduces an unwanted and unnecessary library dependency.
  #ifndef OPENVDB_HOUDINI_API
    #define OPENVDB_HOUDINI_API
  #endif
#else
  #include <openvdb/Platform.h>
#endif
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>


#ifdef SESI_OPENVDB
  #ifdef OPENVDB_HOUDINI_API
    #undef OPENVDB_HOUDINI_API
    #define OPENVDB_HOUDINI_API
  #endif
#endif


class GU_Detail;
class OP_OperatorTable;
class PRM_Parm;

namespace houdini_utils {

class ParmFactory;

using SpareDataMap = std::map<std::string, std::string>;

/// @brief Return the spare data associated with the given operator.
/// @details Only operators created with OpFactory will have spare data.
/// @sa @link addOperatorSpareData() addOperatorSpareData@endlink,
///     @link OpFactory::addSpareData() OpFactory::addSpareData@endlink
const SpareDataMap& getOperatorSpareData(const OP_Operator&);

/// @brief Specify (@e key, @e value) pairs of spare data for the given operator.
/// @details For existing keys, the new value replaces the old one.
/// @throw std::runtime_error if the given operator does not support spare data
///     (only operators created with OpFactory will have spare data)
/// @sa @link getOperatorSpareData() getOperatorSpareData@endlink,
///     @link OpFactory::addSpareData() OpFactory::addSpareData@endlink
void addOperatorSpareData(OP_Operator&, const SpareDataMap&);


/// @brief Parameter template list that is always terminated.
class OPENVDB_HOUDINI_API ParmList
{
public:
    using PrmTemplateVec = std::vector<PRM_Template>;

    ParmList() {}

    /// @brief Return @c true if this list contains no parameters.
    bool empty() const { return mParmVec.empty(); }
    /// @brief Return the number of parameters in this list.
    /// @note Some parameter types have parameter lists of their own.
    /// Those nested lists are not included in this count.
    size_t size() const { return mParmVec.size(); }

    /// @brief Remove all parameters from this list.
    void clear() { mParmVec.clear(); mSwitchers.clear(); }

    /// @{
    /// @brief Add a parameter to this list.
    ParmList& add(const PRM_Template&);
    ParmList& add(const ParmFactory&);
    /// @}

    /// @brief Begin a collection of tabs.
    /// @details Tabs may be nested.
    ParmList& beginSwitcher(const std::string& token, const std::string& label = "");
    /// @brief Begin an exclusive collection of tabs.  Only one tab is "active" at a time.
    /// @details Tabs may be nested.
    ParmList& beginExclusiveSwitcher(const std::string& token, const std::string& label = "");
    /// @brief End a collection of tabs.
    /// @throw std::runtime_error if not inside a switcher or if no tabs
    /// were added to the switcher
    ParmList& endSwitcher();

    /// @brief Add a tab with the given label to the current tab collection.
    /// @details Parameters subsequently added to this ParmList until the next
    /// addFolder() or endSwitcher() call will be displayed on the tab.
    /// @throw std::runtime_error if not inside a switcher
    ParmList& addFolder(const std::string& label);

    /// Return a heap-allocated copy of this list's array of parameters.
    PRM_Template* get() const;

private:
    struct SwitcherInfo { size_t parmIdx; std::vector<PRM_Default> folders; bool exclusive; };
    using SwitcherStack = std::vector<SwitcherInfo>;

    void incFolderParmCount();
    SwitcherInfo* getCurrentSwitcher();

    PrmTemplateVec mParmVec;
    SwitcherStack mSwitchers;
}; // class ParmList


////////////////////////////////////////


/// @class ParmFactory
/// @brief Helper class to simplify construction of PRM_Templates and
/// dynamic user interfaces.
///
/// @par Example
/// @code
/// houdini_utils::ParmList parms;
///
/// parms.add(houdini_utils::ParmFactory(PRM_STRING, "group", "Group")
///     .setHelpText("Specify a subset of the input VDB grids to be processed.")
///     .setChoiceList(&houdini_utils::PrimGroupMenu));
///
/// parms.add(houdini_utils::ParmFactory(PRM_FLT_J, "tolerance", "Pruning Tolerance")
///     .setDefault(PRMzeroDefaults)
///     .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 1));
/// @endcode
class OPENVDB_HOUDINI_API ParmFactory
{
public:
    ParmFactory(PRM_Type, const std::string& token, const std::string& label);
    ParmFactory(PRM_MultiType, const std::string& token, const std::string& label);

    // Settings
    ParmFactory& setCallbackFunc(const PRM_Callback&);

    /// Specify a menu of values for this parameter.
    ParmFactory& setChoiceList(const PRM_ChoiceList*);
    /// @brief Specify a menu type and a list of token, label, token, label,... pairs
    /// for this parameter.
    /// @param typ     specifies the menu behavior (toggle, replace, etc.)
    /// @param items   a list of token, label, token, label,... string pairs
    ParmFactory& setChoiceListItems(PRM_ChoiceListType typ, const std::vector<std::string>& items);
    /// @brief Specify a menu type and a list of token, label, token, label,... pairs
    /// for this parameter.
    /// @param typ     specifies the menu behavior (toggle, replace, etc.)
    /// @param items   a list of token, label, token, label,... string pairs
    /// @note The @a items array must be null-terminated.
    ParmFactory& setChoiceListItems(PRM_ChoiceListType typ, const char* const* items);

    /// @brief Specify a menu of primitive group names for this parameter.
    ///
    /// @param inputIndex  the zero-based index of the input from which to get primitive groups
    /// @param typ         the menu behavior (toggle, replace, etc.)
    ///
    /// @details Calling this method with the default (toggle) behavior is equivalent
    /// to calling @c setChoiceList(&houdini_utils::PrimGroupMenuInput1),
    /// @c setChoiceList(&houdini_utils::PrimGroupMenuInput2), etc.
    ///
    /// @par Example
    /// To limit the user to choosing a single primitive group, replace
    /// @code
    /// parms.add(houdini_utils::ParmFactory(PRM_STRING, "reference", "Reference")
    ///     .setChoiceList(&houdini_utils::PrimGroupMenuInput2);
    /// @endcode
    /// with
    /// @code
    /// parms.add(houdini_utils::ParmFactory(PRM_STRING, "reference", "Reference")
    ///     .setGroupChoiceList(1, PRM_CHOICELIST_REPLACE); // input index is zero based
    /// @endcode
    ParmFactory& setGroupChoiceList(size_t inputIndex,
        PRM_ChoiceListType typ = PRM_CHOICELIST_TOGGLE);

    /// @brief Functor to filter a list of attributes from a SOP's input
    /// @details Arguments to the functor are an attribute to be filtered
    /// and the parameter and SOP for which the filter is being called.
    /// The functor should return @c true for attributes that should be added
    /// to the list and @c false for attributes that should be ignored.
    using AttrFilterFunc =
        std::function<bool (const GA_Attribute&, const PRM_Parm&, const SOP_Node&)>;

    /// @brief Specify a menu of attribute names for this parameter.
    ///
    /// @param inputIndex  the zero-based index of the input from which to get attributes
    /// @param attrOwner   the class of attribute with which to populate the menu:
    ///     either per-vertex (@c GA_ATTRIB_VERTEX), per-point (@c GA_ATTRIB_POINT),
    ///     per-primitive (@c GA_ATTRIB_PRIMITIVE), global (@c GA_ATTRIB_GLOBAL),
    ///     or all of the above (@c GA_ATTRIB_INVALID or any other value)
    /// @param typ         the menu behavior (toggle, replace, etc.)
    /// @param attrFilter  an optional filter functor that returns @c true for each
    ///     attribute that should appear in the menu; the functor will be moved,
    ///     if possible, or else copied
    ///
    /// @note This method is supported only for SOPs.
    ///
    /// @par Example
    /// Create a menu that allows multiple selection from among all the string attributes
    /// on a SOP's first input:
    /// @code
    /// houdini_utils::ParmList parms;
    /// parms.add(houdini_utils::ParmFactory(PRM_STRING, "stringattr", "String Attribute")
    ///     .setAttrChoiceList(/*input=*/0, GA_ATTRIB_INVALID, PRM_CHOICELIST_TOGGLE,
    ///         [](const GA_Attribute& attr, const PRM_Parm&, const SOP_Node&) {
    ///             return (attr.getStorageClass() == GA_STORECLASS_STRING);
    ///         }));
    /// @endcode
    ParmFactory& setAttrChoiceList(size_t inputIndex, GA_AttributeOwner attrOwner,
        PRM_ChoiceListType typ = PRM_CHOICELIST_TOGGLE,
        AttrFilterFunc attrFilter = AttrFilterFunc{});


#if defined(GCC3)
    #define IS_DEPRECATED __attribute__ ((deprecated))
#elif defined(_MSC_VER)
    #define IS_DEPRECATED __declspec(deprecated)
#else
    #define IS_DEPRECATED
#endif

    /// @brief Specify a menu type and either a list of menu item labels or a list of
    /// token, label, token, label,... pairs for this parameter.
    /// @param typ     specifies the menu behavior (toggle, replace, etc.)
    /// @param items   a list of menu item labels or token, label, token, label,... pairs
    /// @param paired  if @c false, treat all the elements of @a items as labels and assign
    ///     them numeric tokens starting from zero; otherwise, treat the elements of @a items
    ///     as token, label, token, label,... pairs
    /// @deprecated Use setChoiceListItems() instead.  Using unpaired items may mean
    /// less typing now, but it prevents you from reordering or deleting entries later.
    IS_DEPRECATED ParmFactory& setChoiceList(PRM_ChoiceListType typ,
        const std::vector<std::string>& items, bool paired = false);
    /// @brief Specify a menu type and either a list of menu item labels or a list of
    /// token, label, token, label,... pairs for this parameter.
    /// @param typ     specifies the menu behavior (toggle, replace, etc.)
    /// @param items   a list of menu item labels or token, label, token, label,... pairs
    /// @param paired  if @c false, treat all the elements of @a items as labels and assign
    ///     them numeric tokens starting from zero; otherwise, treat the elements of @a items
    ///     as token, label, token, label,... pairs
    /// @note The @a items array must be null-terminated.
    /// @deprecated Use setChoiceListItems() instead.  Using unpaired items may mean
    /// less typing now, but it prevents you from reordering or deleting entries later.
    IS_DEPRECATED ParmFactory& setChoiceList(PRM_ChoiceListType typ,
        const char* const* items, bool paired = false);

#undef IS_DEPRECATED

    ParmFactory& setConditional(const PRM_ConditionalBase*);

    /// @brief Specify a default value for this parameter.
    /// @details If the string is null, the floating-point value will be used
    /// (but rounded if this parameter is integer-valued).
    /// @note The string pointer must not point to a temporary.
    ParmFactory& setDefault(fpreal, const char* = nullptr, CH_StringMeaning = CH_STRING_LITERAL);
    /// @brief Specify a default string value for this parameter.
    ParmFactory& setDefault(const std::string&, CH_StringMeaning = CH_STRING_LITERAL);
    /// @brief Specify default numeric values for the vector elements of this parameter
    /// (assuming its vector size is > 1).
    /// @details Floating-point values will be rounded if this parameter is integer-valued.
    ParmFactory& setDefault(const std::vector<fpreal>&);
    /// @brief Specify default values for the vector elements of this parameter
    /// (assuming its vector size is > 1).
    ParmFactory& setDefault(const std::vector<PRM_Default>&);
    /// Specify a default value or values for this parameter.
    ParmFactory& setDefault(const PRM_Default*);

    /// @brief Specify a plain text tooltip for this parameter.
    /// @details This method is equivalent to setTooltip()
    ParmFactory& setHelpText(const char*);
    /// @brief Specify a plain text tooltip for this parameter.
    /// @details This method is equivalent to setHelpText()
    ParmFactory& setTooltip(const char*);
    /// @brief Add documentation for this parameter.
    /// @details Pass a null pointer or an empty string to inhibit
    /// the generation of documentation for this parameter.
    /// @details The text is parsed as wiki markup.
    /// See the Houdini <A HREF="http://www.sidefx.com/docs/houdini/help/format">
    /// Wiki Markup Reference</A> for the syntax.
    ParmFactory& setDocumentation(const char*);

    ParmFactory& setParmGroup(int);

    /// Specify a range for this parameter's values.
    ParmFactory& setRange(
        PRM_RangeFlag minFlag, fpreal minVal,
        PRM_RangeFlag maxFlag, fpreal maxVal);
    /// @brief Specify range for the values of this parameter's vector elements
    /// (assuming its vector size is > 1).
    ParmFactory& setRange(const std::vector<PRM_Range>&);
    /// Specify a range or ranges for this parameter's values.
    ParmFactory& setRange(const PRM_Range*);

    /// Specify (@e key, @e value) pairs of spare data for this parameter.
    ParmFactory& setSpareData(const SpareDataMap&);
    /// Specify spare data for this parameter.
    ParmFactory& setSpareData(const PRM_SpareData*);

    /// @brief Specify the list of parameters for each instance of a multiparm.
    /// @note This setting is ignored for non-multiparm parameters.
    /// @note Parameter name tokens should include a '#' character.
    ParmFactory& setMultiparms(const ParmList&);

    /// Specify an extended type for this parameter.
    ParmFactory& setTypeExtended(PRM_TypeExtended);

    /// @brief Specify the number of vector elements for this parameter.
    /// @details (The default vector size is one element.)
    ParmFactory& setVectorSize(int);

    /// @brief Mark this parameter as hidden from the UI.
    /// @note Marking parameters as obsolete is preferable to making them invisible as changing
    /// invisible parameter values will still trigger a re-cook, however this is not possible
    /// when using multi-parms.
    ParmFactory& setInvisible();

    /// Construct and return the parameter template.
    PRM_Template get() const;

private:
    struct Impl;
    std::shared_ptr<Impl> mImpl;

    // For internal use only, and soon to be removed:
    ParmFactory& doSetChoiceList(PRM_ChoiceListType, const std::vector<std::string>&, bool);
    ParmFactory& doSetChoiceList(PRM_ChoiceListType, const char* const* items, bool);
}; // class ParmFactory


////////////////////////////////////////


class OpPolicy;
using OpPolicyPtr = std::shared_ptr<OpPolicy>;


/// @brief Helper class to simplify operator registration
///
/// @par Example
/// @code
/// void
/// newPopOperator(OP_OperatorTable* table)
/// {
///     houdini_utils::ParmList parms;
///
///     parms.add(houdini_utils::ParmFactory(PRM_STRING, "group", "Group")
///         .setHelpText("Specify a subset of the input VDB grids to be processed.")
///         .setChoiceList(&houdini_utils::PrimGroupMenu));
///
///     parms.add(...);
///
///     ...
///
///     houdini_utils::OpFactory(MyOpPolicy(), My Node",
///         POP_DW_MyNode::factory, parms, *table, houdini_utils::OpFactory::POP)
///         .addInput("Input geometry")              // input 0 (required)
///         .addOptionalInput("Reference geometry"); // input 1 (optional)
/// }
/// @endcode
class OPENVDB_HOUDINI_API OpFactory
{
public:
    enum OpFlavor { SOP, POP, ROP, VOP, HDA };

    /// @brief Return "SOP" for the SOP flavor, "POP" for the POP flavor, etc.
    /// @details Useful in OpPolicy classes for constructing type and icon names.
    static std::string flavorToString(OpFlavor);

    /// @brief Construct a factory that on destruction registers a new operator type.
    /// @param english  the operator's UI name, as it should appear in menus
    /// @param ctor     a factory function that creates operators of this type
    /// @param parms    the parameter template list for operators of this type
    /// @param table    the registry to which to add this operator type
    /// @param flavor   the operator's class (SOP, POP, etc.)
    /// @details @c OpPolicyType specifies the type of OpPolicy to be used to control
    /// the factory's behavior.  The (unused) @c OpPolicyType argument is required
    /// to enable the compiler to infer the type of the template argument
    /// (there is no other way to invoke a templated constructor).
    template<typename OpPolicyType>
    OpFactory(const OpPolicyType& /*unused*/, const std::string& english,
        OP_Constructor ctor, ParmList& parms, OP_OperatorTable& table, OpFlavor flavor = SOP)
    {
        this->init(OpPolicyPtr(new OpPolicyType), english, ctor, parms, table, flavor);
    }

    /// @note Factories initialized with this constructor use the DWAOpPolicy.
    OpFactory(const std::string& english, OP_Constructor ctor,
        ParmList& parms, OP_OperatorTable& table, OpFlavor flavor = SOP);

    /// Register the operator.
#if OPENVDB_ABI_VERSION_NUMBER >= 7
    virtual ~OpFactory();
#else
    ~OpFactory();
#endif

    OpFactory(const OpFactory&) = delete;
    OpFactory& operator=(const OpFactory&) = delete;

    /// @brief Return the new operator's flavor (SOP, POP, etc.).
    /// @details This accessor is mainly for use by OpPolicy objects.
    OpFlavor flavor() const;
    /// @brief Return the new operator's flavor as a string ("SOP", "POP", etc.).
    /// @details This accessor is mainly for use by OpPolicy objects.
    std::string flavorString() const;
    /// @brief Return the new operator's type name.
    /// @details This accessor is mainly for use by OpPolicy objects.
    const std::string& name() const;
    /// @brief Return the new operator's UI name.
    /// @details This accessor is mainly for use by OpPolicy objects.
    const std::string& english() const;
    /// @brief Return the new operator's icon name.
    /// @details This accessor is mainly for use by OpPolicy objects.
    const std::string& iconName() const;
    /// @brief Return the new operator's help URL.
    /// @details This accessor is mainly for use by OpPolicy objects.
    /// @note A help URL takes precedence over help text.
    /// @sa helpText(), setHelpText()
    const std::string& helpURL() const;
    /// @brief Return the new operator's documentation.
    /// @note If the help URL is nonempty, the URL takes precedence over any help text.
    /// @sa helpURL(), setDocumentation()
    const std::string& documentation() const;
    /// @brief Return the operator table with which this factory is associated.
    /// @details This accessor is mainly for use by OpPolicy objects.
    const OP_OperatorTable& table() const;

    /// @brief Construct a type name for this operator from the given English name
    /// and add it as an alias.
    /// @details For backward compatibility when an operator needs to be renamed,
    /// add the old name as an alias.
    OpFactory& addAlias(const std::string& english);
    /// @brief Add an alias for this operator.
    /// @details For backward compatibility when an operator needs to be renamed,
    /// add the old name as an alias.
    /// @note This variant takes an operator type name rather than an English name.
    OpFactory& addAliasVerbatim(const std::string& name);
    /// @brief Add documentation for this operator.
    /// @details The text is parsed as wiki markup.
    /// @note If this factory's OpPolicy specifies a help URL, that URL
    /// takes precedence over documentation supplied with this method.
    OpFactory& setDocumentation(const std::string&);
    /// Add a required input with the given name.
    OpFactory& addInput(const std::string& name);
    /// Add an optional input with the given name.
    OpFactory& addOptionalInput(const std::string& name);
    /// @brief Set the maximum number of inputs allowed by this operator.
    /// @note It is only necessary to set this limit if there are inputs
    /// that have not been named with addInput() or addOptionalInput().
    OpFactory& setMaxInputs(unsigned = 9999);
    /// Specify obsolete parameters to this operator.
    OpFactory& setObsoleteParms(const ParmList&);
    /// Add one or more local variables to this operator.
    OpFactory& setLocalVariables(CH_LocalVariable*);
    OpFactory& setFlags(unsigned);
    OpFactory& setInternalName(const std::string& name);
    OpFactory& setOperatorTable(const std::string& name);

    /// @brief Functor that returns newly-allocated node caches
    /// for instances of this operator
    /// @details A node cache encapsulates a SOP's cooking logic for thread safety.
    /// Input geometry and parameter values are baked into the cache.
    using CacheAllocFunc = std::function<SOP_NodeCache* (void)>;

    /// @brief Register this operator as a
    /// <A HREF="http://www.sidefx.com/docs/houdini/model/compile">compilable</A>&nbsp;SOP.
    /// @details "Verbifying" a SOP separates its input and parameter management
    /// from its cooking logic so that cooking can be safely threaded.
    /// @param cookMode   how to initialize the output detail
    /// @param allocator  a node cache allocator for instances of this operator
    /// @throw std::runtime_error if this operator is not a SOP
    /// @throw std::invalid_argument if @a allocator is empty
    OpFactory& setVerb(SOP_NodeVerb::CookMode cookMode, const CacheAllocFunc& allocator);

    /// @brief Mark this node as hidden from the UI tab menu.
    /// @details This is equivalent to using the hscript ophide method.
    OpFactory& setInvisible();

    /// @brief Specify (@e key, @e value) pairs of spare data for this operator.
    /// @details If a key already exists, its corresponding value will be
    /// overwritten with the new value.
    /// @sa @link addOperatorSpareData() addOperatorSpareData@endlink,
    ///     @link getOperatorSpareData() getOperatorSpareData@endlink
    OpFactory& addSpareData(const SpareDataMap&);

protected:
    /// @brief Return the operator table with which this factory is associated.
    /// @details This accessor is mainly for use by derived OpFactory classes.
    OP_OperatorTable& table();

private:
    void init(OpPolicyPtr, const std::string& english, OP_Constructor,
        ParmList&, OP_OperatorTable&, OpFlavor);

    struct Impl;
    std::shared_ptr<Impl> mImpl;
}; // class OpFactory


////////////////////////////////////////


/// @brief An OpPolicy customizes the behavior of an OpFactory.
/// This base class specifies the required interface.
class OPENVDB_HOUDINI_API OpPolicy
{
public:
    OpPolicy() {}
    virtual ~OpPolicy() {}

    /// @brief Return a type name for the operator defined by the given factory.
    std::string getName(const OpFactory& factory) { return getName(factory, factory.english()); }

    /// @brief Convert an English name into a type name for the operator defined by
    /// the given factory, and return the result.
    /// @details In this base class implementation, the operator's type name is generated
    /// by calling @c UT_String::forceValidVariableName() on the English name.
    /// @note This function might be called (from OpFactory::addAlias(), for example)
    /// with an English name other than the one returned by
    /// factory.@link OpFactory::english() english()@endlink.
    virtual std::string getName(const OpFactory& factory, const std::string& english);

    /// @brief Return an icon name for the operator defined by the given factory.
    /// @details Return an empty string to use Houdini's default icon naming scheme.
    virtual std::string getIconName(const OpFactory&) { return ""; }

    /// @brief Return a help URL for the operator defined by the given factory.
    virtual std::string getHelpURL(const OpFactory&) { return ""; }

    /// @brief Return a label name for the operator defined by the given factory.
    /// @details In this base class implementation, this method simply returns
    /// factory.@link OpFactory::english() english()@endlink.
    virtual std::string getLabelName(const OpFactory&);

    /// @brief Return the inital default name of the operator.
    /// @note An empty first name will disable, reverting to the usual rules.
    virtual std::string getFirstName(const OpFactory&) { return ""; }

    /// @brief Return the tab sub-menu path of the op.
    /// @note An empty path will disable, reverting to the usual rules.
    virtual std::string getTabSubMenuPath(const OpFactory&) { return ""; }
};


////////////////////////////////////////


/// @brief Helper class to manage input locking.
class OPENVDB_HOUDINI_API ScopedInputLock
{
public:
    ScopedInputLock(SOP_Node& node, OP_Context& context)
    {
        mLock.setNode(&node);
        if (mLock.lock(context) >= UT_ERROR_ABORT) {
            throw std::runtime_error("failed to lock inputs");
        }
    }
    ~ScopedInputLock() {}

    void markInputUnlocked(exint input) { mLock.markInputUnlocked(input); }

private:
    OP_AutoLockInputs mLock;
};


////////////////////////////////////////


// Extended group name drop-down menu incorporating "@<attr>=<value" syntax

OPENVDB_HOUDINI_API extern const PRM_ChoiceList PrimGroupMenuInput1;
OPENVDB_HOUDINI_API extern const PRM_ChoiceList PrimGroupMenuInput2;
OPENVDB_HOUDINI_API extern const PRM_ChoiceList PrimGroupMenuInput3;
OPENVDB_HOUDINI_API extern const PRM_ChoiceList PrimGroupMenuInput4;

/// @note   Use this if you have more than 4 inputs, otherwise use
///         the input specific menus instead which automatically
///         handle the appropriate spare data settings.
OPENVDB_HOUDINI_API extern const PRM_ChoiceList PrimGroupMenu;


} // namespace houdini_utils

#endif // HOUDINI_UTILS_PARM_FACTORY_HAS_BEEN_INCLUDED
