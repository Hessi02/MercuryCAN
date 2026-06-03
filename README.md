# AVR-CAN

A easy-to-use C++23 CAN library optimized for the AT90CAN128 microcontroller.

## Usage

The library offers classes for receiving and transmitting CAN frames. The following examples show the general use of these classes. Both classes can be used independently of one another. Nevertheless, only one instance of each respective class should exist.

### Transmitting

For transmission, variables are initialized on the stack or heap. Under no circumstances may this memory be deallocated during runtime. In combination with an identifier and timing information, these arbitrary parameters of the constructor are assembled into a cyclic message. These messages can then be assigned to the transmitter.

```cpp
unsigned char*  state = new unsigned char(0); 
unsigned short* value = new unsigned short(0);
    
Can::Model::CyclicMessage measTxFrame (
    0x001, 10, state, value         
);

Can::Controller::Transmitter tx; 
tx.addCyclicMessage(measTxFrame);
```

### Receiving

The receiving process initially proceeds analogously to the sending process. However, the timing information is derived from the CAN peer and is therefore superfluous as a parameter; consequently, it will likely be removed from the signature. Here, too, it is important that the passed memory region is not deallocated.

```cpp
unsigned short* target = new unsigned short(0);
    
Can::Model::CyclicMessage measRxFrame (
    0x002, 100, target
);
    
Can::Controller::Receiver rx;
rx.addCyclicMessage(measRxFrame);
```
