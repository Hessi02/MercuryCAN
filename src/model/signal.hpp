#ifndef __CAN_MODEL_SIGNAL_HPP__
#define __CAN_MODEL_SIGNAL_HPP__

#include <type_traits>
#include <variant>

namespace Can::Model {

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
    double>
    AllowedSignalTypes_t;

template<typename TypeToCheck, typename VariantOfValids>
struct InVariant {};

template<typename TypeToCheck, typename... AllowedDataTypes>
struct InVariant<TypeToCheck, std::variant<AllowedDataTypes...>>
    : std::disjunction<std::is_same<TypeToCheck, AllowedDataTypes>...> {};

template<typename SignalDataType>
concept AllowedSignalDataType =
    InVariant<SignalDataType, AllowedSignalTypes_t>::value;

template<typename SignalDataType>
    requires AllowedSignalDataType<SignalDataType>
class Signal {
public:
    Signal(SignalDataType* signalPtr) {
        _data = signalPtr;
    }

    constexpr std::size_t getDataSize(void) const {
        return sizeof(SignalDataType);
    }

    SignalDataType* getDataPtr(void) const {
        return _data;
    }

private:
    SignalDataType* _data;
};

template<typename DataType>
struct MakeSignalVariant {
    using AnyType = Signal<DataType>;
};

template<typename... DataType>
struct MakeSignalVariant<std::variant<DataType...>> {
    using AnyType = std::variant<Signal<DataType>...>;
};

using AnySignal_t = MakeSignalVariant<AllowedSignalTypes_t>::AnyType;

template<typename SignalDataType>
concept AllowedSignal = InVariant<SignalDataType, AnySignal_t>::value;

}

#endif  //__CAN_MODEL_SIGNAL_HPP__