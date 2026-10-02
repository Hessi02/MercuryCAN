/**
 * \file    signal.hpp
 * \brief   Contains an header-only implementation for a CAN Signal model class.
 * 
 * The file contains the 'Signal' class, which can hold various data types. 
 * To this end, various static checks and abstractions are applied, ultimately 
 * making 'AnySignal_t' available as an abstract signal alongside the 
 * 'AllowedSignal' concept.
 * 
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 * 
 * SPDX-License-Identifier: Apache-2.0 
 */

#ifndef __CAN_MODEL_SIGNAL_HPP__
#define __CAN_MODEL_SIGNAL_HPP__

#include <type_traits>
#include <variant>

namespace Can::Model {

/**
 * \brief   Variant of all data types that can be stored inside a signal object.
 * 
 * In general a CAN bus signal can represent data types like boolean, (unsigned) 
 * integer, fixed-point number, floating-point number, enumeration, bit field,
 * string or even raw data.
 * 
 * For now only (unsigned) integers, fixed-point numbers, floating-point numbers 
 * and raw data are supported. The remaining types are supported only indirectly 
 * yet. Enums can be added by defining an underlaying type like unsigned char. 
 * Variables with a number of bits not divisible by 8 are also not yet directly 
 * supported. The  conversion of a bit array must therefore currently be handled 
 * by the user. Booleans may be expressed as integer values for now. There may 
 * be added an interface to even add custom structs. For now, this remains as 
 * part of the to-do list.
 * 
 * \todo    Support defining the meaning of individual bits of a byte.
 * \todo    Support of self-defined structs without use of typecasts. 
 */
typedef std::variant<
    char,
    short,
    int,
    long,
    long long,
    unsigned char,
    unsigned short,
    unsigned int,
    unsigned long,
    unsigned long long,
    float,
    double
> AllowedSignalTypes_t;


/**
 * \brief   Checks whether a type is contained in a std::variant.
 * 
 * Evaluates to 'true' if \p TypeToCheck matches at least one of the types
 * contained in \p VariantOfValids. Top-level 'const' and 'volatile' qualifiers
 * of \p TypeToCheck are ignored.
 * 
 * \tparam  TypeToCheck passes a type to check against the types of the variant. 
 * \tparam  VariantOfValids passes a std::variant containing the allowed types.
 * 
 * \example
 * \code
 * using MyVariant = std::variant<int, double, std::string>
 * 
 * static_assert(InVariant<int, MyVariant>::value);
 * static_assert(InVariant<double, MyVariant>::value);
 * static_assert(InVariant<const int, MyVariant>::value);
 * static_assert(!InVariant<float, MyVariant>::value);
 * \endcode
 */
template<typename TypeToCheck, typename VariantOfValids>
struct InVariant {};


/**
 * \brief   Specialization for std::variant.
 * 
 * This specialization extracts all types contained in the given std::variant 
 * and checks whether \p TypeToCheck matches at least one of them. The types 
 * contained are captured by the parameter pack \p AllowedDataTypes.
 * 
 * Top-level const and volatile qualifiers are removed from @p TypeToCheck 
 * before performing the comparisons. Qualifiers of the types contained in the 
 * variant are not removed. 
 * 
 * \tparam  TypeToCheck passes a type to check against the types of the variant. 
 * \tparam  VariantOfValids passes a std::variant containing the allowed types.
 */
template<typename TypeToCheck, typename... AllowedDataTypes>
struct InVariant<TypeToCheck, std::variant<AllowedDataTypes...>>
    : std::disjunction<
          std::is_same<std::remove_cv_t<TypeToCheck>, AllowedDataTypes>...> {};


/**
 * \brief   Concept that constrains a type to the allowed signal data types.
 *
 * A type satisfies this concept if it is contained in the std::variant 
 * represented by \c AllowedSignalTypes_t.
 *
 * The actual type check is performed by \c InVariant. Top-level const and 
 * volatile qualifiers of \p SignalDataType are ignored during the check.
 *
 * This concept can be used to constrain functions, classes, or other templates 
 * to signal data types supported by the system.
 *
 * For example, if \c AllowedSignalTypes_t is defined as:
 *
 * \code
 * using AllowedSignalTypes_t = std::variant<int, double, std::string>;
 * \endcode
 *
 * the following types satisfy the concept:
 *
 * \code
 * static_assert(AllowedSignalDataType<int>);
 * static_assert(AllowedSignalDataType<volatile double>);
 * static_assert(AllowedSignalDataType<const int>);
 * \endcode
 *
 * whereas a type not contained in the variant does not:
 *
 * \code
 * static_assert(!AllowedSignalDataType<bool>);
 * \endcode
 *
 * \tparam  SignalDataType passes a type to check against the allowed data types.
 *
 * \see     InVariant
 * \see     AllowedSignalTypes_t
 */
template<typename SignalDataType>
concept AllowedSignalDataType =
    InVariant<SignalDataType, AllowedSignalTypes_t>::value;


/**
 * \brief   Represents a signal containing a pointer to signal data.
 *
 * A Signal is associated with a specific signal data type, which must be one of 
 * the types allowed by AllowedSignalDataType.
 *
 * The Signal class does not own the referenced data. The caller is responsible 
 * for ensuring that the pointed-to object remains valid for the lifetime of the 
 * Signal object.
 *
 * \tparam  SignalDataType passes the type of the signal's data.
 *
 * \see     AllowedSignalDataType
 * \see     AllowedSignal
 */
template<typename SignalDataType>
    requires AllowedSignalDataType<SignalDataType>
class Signal 
{
public:

    /**
     * \brief   Constructs a Signal from a pointer to signal data.
     *
     * The pointer is stored internally without taking ownership of the 
     * referenced object. Therefore the data can be present on stack, heap, or 
     * even in .bss / .data sections. Is just has to be valid over the signal's
     * lifetime. 
     * 
     * \param   signalPtr passes a pointer to the signal's data.
     */
    Signal(SignalDataType* signalPtr) {
        _data = signalPtr;
    }

    /**
     * \brief   Returns the size of the signal's data type.
     *
     * The returned size is the size of SignalDataType as reported by the C++ 
     * sizeof operator. It does not represent the size of the object referenced 
     * by the stored pointer at runtime.
     *
     * \return  Size of SignalDataType in bytes.
     */
    constexpr std::size_t getDataSize(void) const {
        return sizeof(SignalDataType);
    }

    /**
     * \brief   Returns the pointer to the internal signal data.
     *
     * The signal does not transfer ownership of the referenced object.
     *
     * \return  Pointer to the associated signal data.
     */
    SignalDataType* getDataPtr(void) const {
        return _data;
    }

private:

    /**
     * \brief   Pointer to the associated signal data.
     *
     * The pointed-to object is not owned by the Signal.
     */
    SignalDataType* _data;
};


/**
 * \brief   Maps a data type to its corresponding Signal type.
 *
 * The primary template creates a Signal for a single data type. This trait is 
 * used by MakeSignalVariant to construct a type that can represent one or more 
 * possible Signal types.
 *
 * \tparam  DataType passes a data type for which a Signal type is created.
 *
 * \see Signal
 * \see MakeSignalVariant
 */
template<typename DataType>
struct MakeSignalVariant {

    /**
     * \brief   Signal type associated with DataType.
     */
    using AnyType = Signal<DataType>;
};


/**
 * \brief   Specialization of MakeSignalVariant for std::variant.
 *
 * For a std::variant containing multiple allowed signal data types, this 
 * specialization creates a corresponding std::variant containing a Signal for 
 * each of those data types. In addition to the regular Signal<DataType> types, 
 * Signals using volatile-qualified data types are generated as well.
 *
 * For example:
 *
 * \code
 * using DataTypes = std::variant<int, double>;
 * \endcode
 *
 * results in:
 *
 * \code
 * using SignalTypes = std::variant<
 *     Signal<int>,
 *     Signal<double>,
 *     Signal<volatile int>,
 *     Signal<volatile double>
 * >;
 * \endcode
 *
 * The resulting variant can therefore represent a Signal for every
 * allowed data type, including its volatile-qualified counterpart.
 *
 * \tparam  DataType passes types contained in the input std::variant.
 *
 * \see     MakeSignalVariant
 * \see     Signal
 */
template<typename... DataType>
struct MakeSignalVariant<std::variant<DataType...>> 
{
    /**
     * \brief Variant containing the corresponding Signal types.
     *
     * For every type DataType contained in the input variant, this variant 
     * contains both Signal<DataType> and Signal<volatile DataType>.
     */
    using AnyType = std::variant<
        Signal<DataType>...,
        Signal<volatile DataType>...
    >;
};


/**
 * \brief   Variant containing all supported signal types.
 *
 * The type is generated from AllowedSignalTypes_t by MakeSignalVariant.
 *
 * For every allowed signal data type, the variant contains a Signal for the 
 * regular type and a Signal for its volatile-qualified counterpart.
 *
 * For example, if AllowedSignalTypes_t is:
 *
 * \code
 * using AllowedSignalTypes_t = std::variant<int, double>;
 * \endcode
 *
 * AnySignal_t becomes equivalent to:
 *
 * \code
 * std::variant<
 *     Signal<int>,
 *     Signal<double>,
 *     Signal<volatile int>,
 *     Signal<volatile double>
 * >
 * \endcode
 *
 * \see     AllowedSignalTypes_t
 * \see     MakeSignalVariant
 * \see     Signal
 */
using AnySignal_t = MakeSignalVariant<AllowedSignalTypes_t>::AnyType;


/**
 * \brief   Concept that constrains a type to the supported Signal types.
 *
 * A type satisfies this concept if it is contained in AnySignal_t. In other 
 * words, the concept can be used to determine whether a type represents one of 
 * the Signal specializations supported by the system.
 *
 * This concept is based on InVariant and therefore evaluates to true if 
 * SignalDataType matches at least one of the types contained in AnySignal_t.
 *
 * \tparam  SignalDataType passes a type to check against the supported types.
 *
 * \see     AnySignal_t
 * \see     InVariant
 * \see     Signal
 */
template<typename SignalDataType>
concept AllowedSignal = InVariant<SignalDataType, AnySignal_t>::value;
}

#endif  //__CAN_MODEL_SIGNAL_HPP__
