# STM32WB55 BLE HCI Transparent Mode

This module allows the usage of a stm32wb55 board (eg. nucleo or dongle) as a USB/UART Bluetooth HCI Dongle.

This allows it to be used to provide bluetooth functionality with the unix micropython port.

It also has full support for use with STM32CubeMonitor-Rf app.

The native module can be compiled and deployed with:

```
cd examples/natmod/stm32wb55_transparent_vcp
make
mpremote cp rfcore_transparent.mpy :
```

Minimal usage:

```
import rfcore_transparent
rfcore_transparent.start()
```

By default stdio (repl) will be used / taken over by this
transparent mode.

Example `main.py`

```
import os
from pyb import Pin, LED

sw = Pin("SW3", Pin.IN, Pin.PULL_UP)

def activity(status):
    if status:
        LED(3).on()
    else:
        LED(3).off()

if sw.value():
    LED(2).on()
    import rfcore_transparent

    # Disconnect USB VCP from repl to use here
    usb = os.dupterm(None, 1)  # startup default is usb (repl) on slot 1

    rfcore_transparent.start(usb, activity)

```
