# LmicWrap

A wrapper for the lmic library, created for the Arduino. 

## Installation

The use of the library of dragino's lmic is required. This library is smaller than the one of [Matthijs](https://github.com/matthijskooijman/arduino-lmic). 

```bash
git clone https://github.com/dragino/arduino-lmic.git
``` 

## Usage

```cpp
  LmicWrap* LW;
  LW->init();
  LW->sendData();
```


## Contributing

Pull requests are welcome. Open an issue to discuss further changes. 