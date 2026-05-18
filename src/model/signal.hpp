#ifndef __CAN_MODEL_SIGNAL_HPP__
#define __CAN_MODEL_SIGNAL_HPP__

#include <variant>

namespace Can::Model
{

/**
 *  \typedef    AllowedSignalTypes_t
 *  \brief      A variant of valid template types for the class 'Signal'.
 *  \todo       Add the types bool and nibble. 
 * 
 *  The variant contains the types that are used by the additional concept 
 *  mechanism to make sure only the contained variables can be used as template
 *  parameters to create an instance of type 'Signal'. When adding a signal also 
 *  make sure that the physical representation in the CAN message is implemented 
 *  correctly. 
 */
typedef std::variant<
    char,
    short,
    int,
    long,
    unsigned char,
    unsigned short,
    unsigned int,
    unsigned long,
    float,   
    double
> AllowedSignalTypes_t;

/**
 *  \struct     InVariant
 *  \brief      Compile-time check whether a type exists inside a std::variant.
 *  \tparam     TypeToCheck passes the type that shall be checked.
 *  \tparam     VariantOfValids passes a variant containing valid types.
 * 
 *  This helper meta-structure is used to determine at compile time whether
 *  a specific type is contained within a given std::variant type definition or
 *  not. The primary template acts as the generic declaration, while the 
 *  specialized implementation for std::variant performs the actual type 
 *  comparison.
 */
template <typename TypeToCheck, typename VariantOfValids>
struct InVariant
{};

/**
 *  \struct     InVariant<TypeToCheck, std::variant<AllowedDataTypes...>>
 *  \brief      Specialized implementation of InVariant for std::variant.
 *  \tparam     TypeToCheck passes the type that shall be checked.
 *  \tparam     AllowedDataTypes passes a variadic list of all variant types.
 * 
 *  This specialization expands all types contained in the variant and compares 
 *  them individually against 'TypeToCheck'. The inheritance from 
 *  std::disjunction causes the compile-time result to evaluate to true as soon 
 *  as one matching type is found.
 */
template <typename TypeToCheck, typename... AllowedDataTypes>
struct InVariant<TypeToCheck, std::variant<AllowedDataTypes...>>
    : std::disjunction<std::is_same<TypeToCheck, AllowedDataTypes>...> 
{};

/**
 *  \concept    AllowedSignalDataType
 *  \brief      Restricts signal data types to a predefined set of valid types.
 *
 *  This concept is used to ensure that only explicitly supported data types
 *  can be used as the payload type of a CAN signal. The validation is performed 
 *  at compile time by checking whether the specified type 'SignalDataType' is 
 *  contained in the variant 'AllowedSignalTypes_t'. The check itself is 
 *  delegated to the helper meta-structure 'InVariant'.
 */
template <typename SignalDataType>
concept AllowedSignalDataType = 
    InVariant<SignalDataType, AllowedSignalTypes_t>::value;

/**
 *  \class      Signal
 *  \brief      A signal is a single piece of information in a CAN message.
 *  \tparam     SignalDataType specifies the type of the variable to represent.
 * 
 *  The 'Signal' class represents one piece of information, such as vehicle 
 *  speed, engine temperature, or battery voltage. Signals are encoded into 
 *  defined bits and bytes within a CAN message. The class can therefore be 
 *  viewed as a wrapper around the data variable in the MCU's RAM, through which 
 *  the actual data type is abstracted. Regardless of this, the usage of the 
 *  signal remains the same.
 */
template<typename SignalDataType>
requires AllowedSignalDataType<SignalDataType>
class Signal
{
public:
    /**
     *  \brief          Initializes instance by storing data's memorys address.
     *  \param[in|out]  signalPtr passes signal's data address as pointer.
     */
    Signal(SignalDataType* signalPtr) {
        _data = signalPtr;
    }
  
    /**
     *  \brief      Determines the memory size of the underlaying data type.
     *  \return     The stored data size represented as std::size_t.
     */
    constexpr std::size_t getDataSize(void) const {
        return sizeof(SignalDataType);
    }

    /**
     *  \brief      Getter method for access the signals data pointer.
     *  \return     The stored data pointer using the template parameter's type. 
     */
    SignalDataType* getDataPtr(void) const {
        return _data;
    }

private:
    SignalDataType* _data;
};

/**
 *  \struct     MakeSignalVariant
 *  \brief      Creates a signal-based type representation from a data type.
 *  \tparam     DataType specifies either single or a varaint of data types.
 *
 *  This helper meta-structure is used to transform plain data types into their
 *  corresponding 'Signal<...>' representation. The primary template handles
 *  single data types and maps them directly to a matching 'Signal' type.
 *
 *  A dedicated specialization for std::variant expands all contained types and
 *  creates a new std::variant consisting of the corresponding signal wrapper
 *  types. This mechanism is used to automatically generate a variant type that
 *  can store any supported signal instance.
 */
template <typename DataType>
struct MakeSignalVariant 
{
    using AnyType = Signal<DataType>; 
};

/**
 *  \struct     MakeSignalVariant<std::variant<DataType...>>
 *  \brief      Specialization of MakeSignalVariant for std::variant.
 *  \tparam     DataType passes a variadic list of allowed signal data types.
 *
 *  This specialization expands all data types contained in the provided
 *  std::variant and transforms each type into its corresponding
 *  'Signal<DataType>' representation. The resulting type alias 'AnyType'
 *  therefore becomes a std::variant capable of storing any supported signal
 *  type. The specialization is used together with 'AllowedSignalTypes_t' to 
 *  automatically generate the alias 'AnySignal_t', which represents a variant 
 *  containing all valid signal wrapper types.
 */
template <typename... DataType>
struct MakeSignalVariant<std::variant<DataType...>> 
{
    using AnyType = std::variant<Signal<DataType>...>;
};

/**
 *  \typedef    AnySignal_t
 *  \brief      Variant containing all supported signal wrapper types.
 *
 *  This type alias represents a std::variant composed of all valid
 *  'Signal<DataType>' specializations derived from the types listed in
 *  'AllowedSignalTypes_t'. The transformation is performed automatically by
 *  the helper meta-structure 'MakeSignalVariant'.
 *
 *  The resulting variant can therefore store any supported signal instance
 *  independent of its underlying payload type. It serves as a generic signal
 *  container that enables type-safe handling of heterogeneous signal objects.
 */
using AnySignal_t = MakeSignalVariant<AllowedSignalTypes_t>::AnyType;

/**
 *  \concept    AllowedSignal
 *  \brief      Restricts types to supported signal wrapper instances.
 *  \tparam     SignalDataType specifies the signal type to validate.
 *
 *  This concept is used to ensure that only valid signal wrapper types can be
 *  used in contexts expecting a supported CAN signal representation. The check
 *  is performed at compile time by verifying whether the specified
 *  'SignalDataType' exists inside the variant 'AnySignal_t'.
 *
 *  The validation itself is delegated to the helper meta-structure
 *  'InVariant', which compares the provided type against all signal wrapper
 *  types contained in the generated variant.
 */
template <typename SignalDataType>
concept AllowedSignal = InVariant<SignalDataType, AnySignal_t>::value;

}

#endif //__CAN_MODEL_SIGNAL_HPP__