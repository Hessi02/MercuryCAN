# MercuryCAN
An easy-to-use C++23 CAN library optimized for the AT90CAN128 microcontroller.

<p align="center">
    <img src="res/icons/logo.png" alt="MercuryCAN" width="180">
</p>

## Prerequisites

Different tools are required for compiling and flashing. A different toolchain may also be used if desired; however, there is no guarantee that all components of the library will be fully functional in that case.

### AVR-GCC
`AVR-GCC` is a compiler used to write C and C++ programs for AVR microcontrollers. It compiles the source code into machine code that an AVR microcontroller can execute. Make sure, that your selected version supports C++23.

[![GitHub](https://img.shields.io/github/v/release/ZakKemble/avr-gcc-build?logo=github)](https://github.com/ZakKemble/avr-gcc-build)

### AVRDUDE
`AVRDUDE` is a command-line tool used to program AVR microcontrollers. It can upload compiled firmware to the microcontroller and read or write its memory. AVRDUDE supports various programmers and interfaces.

[![GitHub](https://img.shields.io/github/v/release/avrdudes/avrdude?logo=github)](https://github.com/avrdudes/avrdude)

### avr-libstdcpp
`avr-libstdcpp` is a C++ library that provides standard C++ functionality for AVR microcontrollers.
It includes features such as classes, containers, and other parts of the C++ standard library.
It is designed for resource-constrained embedded systems, where memory and processing power are limited.

[![GitHub](https://img.shields.io/github/last-commit/modm-io/avr-libstdcpp?logo=github&label=commit)](https://github.com/modm-io/avr-libstdcpp)

## Usage

This library offers classes for receiving and transmitting CAN frames. The following examples show the general use of these classes. Both classes can be used independently of one another. Nevertheless, only one instance of each respective class should exist.

### Transmitting

For transmission, variables are initialized on the stack or heap. Under no circumstances may this memory be deallocated during usage. By passing an identifier and one ore multiple signal data variables, a single-shot message can be constructed:

```cpp
void sendXAxisData(volatile short xAcc, volatile short yAcc) {
    // BO_ 16 accelerations: 4 Vector__XXX
	//   SG_ xAcc :  0|16@1- (1,0) [0|32000] "mg" Vector__XXX
	//   SG_ yAcc : 16|16@1- (1,0) [0|32000] "mg" Vector__XXX

    Can::Model::Message accelerations(
        0x10,
        &xAcc,
        &yAcc
    );

    static Can::Controller::Transmitter tx; 
    tx.sendMessage(accelerations);
}
```

In combination with an additional timing information, these arbitrary parameters of the constructor are assembled into a cyclic message. These messages can then be assigned to the transmitter. The following  example shows an 100 Hz message transmission:

```cpp
int main(void) {
    volatile double voltage = 0.0;
    volatile double current = 0.0;

    // BO_ 18 powerMeasures: 8 Vector__XXX
	//   SG_ voltage :  0|32@1- (1,0) [0|80] "V" Vector__XXX
	//   SG_ current : 32|32@1- (1,0) [0|10] "A" Vector__XXX
    
    Can::Model::CyclicMessage powerMeasures{
        0x12,
        10,
        &voltage,
        &current
    };

    Can::Controller::Transmitter tx;
    tx.addCyclicMessage(powerMeasures);

    while (true) {
        voltage = measureVoltage();
        current = measureCurrent();
    }
}
```

For small, short-running tasks, a synchronous callback can be linked to a so-called signal.

The following example shows a complete initialization for a temperature sensor:

```cpp
int main(void) {
    volatile unsigned char systemState    = 0;
    volatile unsigned long systemTime     = 0;
    volatile signed short  temperature    = 0;
    volatile unsigned char rollingCounter = 0;

    // BO_ 86 status: 5 Vector__XXX
	//   SG_ systemState : 0|8@1+ (1,0) [0|10] "" Vector__XXX
	//   SG_ systemTime : 8|32@1+ (0.001,0) [0|0] "s" Vector__XXX
    
    Can::Model::CyclicMessage status(
        0x56,
        100,
        &systemState,
        &systemTime
    );
    
    SignalSlot::MetaObject::connect(
        &status,
        &Can::Model::CyclicMessage::Message::preSend,
        +[](unsigned long) {
            auto& driver = Can::Driver::DriverInterface::getInstance();
            systemTime = driver.getTickCountMs();
        }
    );

    // BO_ 256 measurement: 3 Vector__XXX
    //   SG_ temperature : 7|16@0- (0.01,0) [-20|80] "°C" Vector__XXX
    //   SG_ rollingCounter : 16|8@1+ (1,0) [0|0] "" Vector__XXX
    
    Can::Model::CyclicMessage measurement(
        0x100,         
        100,          
        &temperature,
        &rollingCounter
    );

    SignalSlot::MetaObject::connect(
        &measurement,
        &Can::Model::CyclicMessage::Message::preSend,
        +[](unsigned long) {
            static Sensor sensor;
            temperature = sensor.readTemperature();
        }
    );

    SignalSlot::MetaObject::connect(
        &measurement,
        &Can::Model::CyclicMessage::Message::sent,
        +[](unsigned long) {
            rollingCounter += 1;
        }
    );

    systemState = 1;

    Can::Controller::Transmitter tx; 
    tx.addCyclicMessage(status);
    tx.addCyclicMessage(measurement);
        
    auto& driver = Can::Driver::DriverInterface::getInstance();
    driver.enterIdleSleep();
}
```

### Receiving

The receiving process initially proceeds analogously to the sending process. However, the timing information is derived from the CAN peer and is therefore superfluous as a parameter; consequently, it will likely be removed from the signature. Here, too, it is important that the passed memory region is not deallocated. 

As with the transmitter, the cyclic message function is available. The transmitter and receiver can be conveniently used together and, for example, access shared memory areas.

A simple reception routine can be structured as follows:

```cpp
int main(void) {
    volatile unsigned short value = 0;
    
    // BO_ 1 peerFrame : 2 Vector__XXX
    //   SG_ peerValue : 16|0@1+ (1,0) [0|0] "" Vector__XXX
    
    Can::Model::CyclicMessage peerFrame(
        0x01,
        1,
        &value
    );

    // BO_ 2 repeatedFrame : 2 Vector__XXX
    //   SG_ repeatedValue : 16|0@1+ (1,0) [0|0] "" Vector__XXX
    
    Can::Model::CyclicMessage repeatedFrame(
        0x02,
        1,
        &value
    );
    
    Can::Controller::Receiver rx;
    rx.addCyclicMessage(peerFrame);

    Can::Controller::Transmitter tx;
    tx.addCyclicMessage(repeatedFrame);

    auto& driver = Can::Driver::DriverInterface::getInstance();
    driver.enterIdleSleep();
}
```

A single-shot message is available to the user here as well. The respective method waits for the specified duration until a message update occurs. If a timeout of zero milliseconds is passed, the system waits without a timeout. It should be noted that this is a synchronous event. A potential asynchronous implementation is a subject for future development.

The example below shows such an implementation:

```cpp
bool awaitTorqueMeasurement(volatile long& torque) {
    // BO_ 9 trqMeas : 4 Vector__XXX
	//   SG_ torque : 0|32@1- (0.001,0) [0|75] "Nm" Vector__XXX
    
    Can::Model::Message trqMeas(
        0x09,
        &torque
    );

    static Receiver rx;
    return rx.awaitMessage(trqMeas);
}
```

Signals—such as those triggered upon the receipt of a frame—are also available here for callbacks. These callbacks might also be a method:

```cpp
class TemperatureConverter : public SignalSlot::MetaObject
{
public:
    double temperatureC;
    double temperatureF;

    void onTemperatureReceived(unsigned long tickCountMs) {
        temperatureF = temperatureC * 9 / 5 + 32;
    }
};

int main(void) {
    TemperatureConverter conv;
    
    // BO_ 10 measurement : 4 Vector__XXX
    //   SG_ temperature : 0|32@1- (1,0) [-10|50] "°C" Vector__XXX
    
    Can::Model::CyclicMessage measRxFrame(
        0x0a,
        100,
        &conv.temperatureC
    );   
 
    SignalSlot::MetaObject::connect(
        &measRxFrame,
        &Can::Model::CyclicMessage::Message::received,
        &conv,
        &TemperatureConverter::onTemperatureReceived
    );

    Receiver rx;
    rx.addCyclicMessage(measRxFrame);

    while (true) {
        std::cout << conv.temperatureC << " °C => " << conv.temperatureF << " °F" << std::endl;
        _delay_ms(1);
    }
}
```

In general, MercuryCAN is intended to keep embedded CAN communication straightforward: define the message layout, bind the relevant signal variables, and then add the message to a transmitter or receiver depending on whether the device is sending or receiving data. This separation of frame definitions, signal access, and timing callbacks makes it easy to model periodic telemetry, event-driven updates, and reactive processing without mixing protocol details into the application logic.

## Development

The further development of the library focuses on two main aspects:

### Reduction of execution time

The library serves as a communication tool rather than the primary task of an embedded system; therefore, sufficient computing capacity must remain available for other tasks. The focus of these optimizations is the transmission cycle sequence. Furthermore, the interaction with other CAN devices must be tested to optimally align the timing of the CAN frames with other devices on the bus.

### Automatic generation from a database

Automated definition of the relevant frames and signals should be enabled via DBC input. Wherever possible, no wizard or generator application should be required. The library should provide mechanisms to parse such a DBC file at compile time and convert it into the corresponding code.
