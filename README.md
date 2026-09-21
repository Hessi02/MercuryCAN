# MercuryCAN

An easy-to-use C++23 CAN library optimized for the AT90CAN128 microcontroller.

## Build

The library uses CMake as its build system. To use this library just clone it and include it to your CMakeLists.txt as follows:

```cmake
cmake_minimum_required(VERSION 3.10)

project(MyProject LANGUAGES C CXX)

set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

set(MCU at90can128)
set(F_CPU 16000000UL)

add_subdirectory(lib/MercuryCAN)

add_executable(MyTarget
    src/main.cpp
)

target_compile_options(MyTarget PRIVATE
    -mmcu=${MCU}
    -Ofast
    -Wall
    -fno-threadsafe-statics
)

target_compile_definitions(MyTarget PRIVATE
    F_CPU=${F_CPU}
    BAUD=${BAUD}
)

target_include_directories(MyTarget PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/MercuryCAN/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/MercuryCAN/lib
)

target_link_libraries(MyTarget PRIVATE
    MercuryCAN
)
```

## Usage

The library offers classes for receiving and transmitting CAN frames. The following examples show the general use of these classes. Both classes can be used independently of one another. Nevertheless, only one instance of each respective class should exist.

### Transmitting

For transmission, variables are initialized on the stack or heap. Under no circumstances may this memory be deallocated during runtime. In combination with an identifier and timing information, these arbitrary parameters of the constructor are assembled into a cyclic message. These messages can then be assigned to the transmitter.

An example initialization can be structured as follows:

```cpp
static volatile unsigned char systemState    = 0;
static volatile unsigned long systemTime     = 0;
static volatile signed short  temperature    = 0;
static volatile unsigned char rollingCounter = 0;

int main(void) {

    // BO_ 86 status: 5 Vector__XXX
	//   SG_ systemState : 0|8@1+ (1,0) [0|10] "" Vector__XXX
	//   SG_ systemTime : 8|32@1+ (0.001,0) [0|0] "s" Vector__XXX
    
    Can::Model::CyclicMessage status(
        0x56,
        100,
        &systemState,
        &systemTime
    );
    
    BareSignal::MetaObject::connect(
        &status,
        &Can::Model::CyclicMessage::Message::preSend,
        +[](unsigned long) {
            auto& driver = Can::Controller::Driver::getInstance();
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

    BareSignal::MetaObject::connect(
        &measurement,
        &Can::Model::CyclicMessage::Message::preSend,
        +[](unsigned long) {
            static Sensor sensor;
            temperature = sensor.readTemperature();
        }
    );

    BareSignal::MetaObject::connect(
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
        
    auto& driver = Can::Controller::Driver::getInstance();
    driver.enterIdleSleep();
}
```

### Receiving

The receiving process initially proceeds analogously to the sending process. However, the timing information is derived from the CAN peer and is therefore superfluous as a parameter; consequently, it will likely be removed from the signature. Here, too, it is important that the passed memory region is not deallocated.

```cpp
static volatile unsigned short value = 0;

int main(void) {
    Can::Model::CyclicMessage peerFrame(
        0x01,
        1,
        &value
    );

    Can::Model::CyclicMessage repeatedFrame(
        0x02,
        1,
        &value
    );
    
    Can::Controller::Receiver rx;
    rx.addCyclicMessage(peerFrame);

    Can::Controller::Transmitter tx;
    tx.addCyclicMessage(repeatedFrame);

    auto& driver = Can::Controller::Driver::getInstance();
    driver.enterIdleSleep();
}
```
