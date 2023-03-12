# listusb

A tool that detailed listing USB device for macOS, Linux, MSYS2 shells and consoles.

## What is listusb

* macOS don't have tool to what USB device connected in shell.
* There's lsusb in brew usbtools, but not functional to check detailed USB information.
* listusb helps what USB device connected, and help to correctly identified to my macOS as real device supported speed - specially NVMe SSD devices easily not recognized to USB2 rather than USB3 !
* And better view for POSIX `lsusb`.

## Easy to read

* Build source, and install to user space, then type `listusb`.

```
>  listusb
listusb, version 0.2.0.10, (C)Copyrighted 2023 Raphael Kim, w/ libusb v1.0.26
Bus 001, Port 000 [1B21:1242] (no manufacturer)(no product name)
    + (SN not found)
    + Class = HUB device
    + bcdID = 0300, human readable = USB 3.0
    + config[ 0], interfaces = 1, ID = 0x01, max required power = 0 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=1:0x6)
Bus 001, Port 004 [2109:2812] (no manufacturer)(no product name)
    + (SN not found)
    + Class = HUB device
    + bcdID = 0210, human readable = USB 2.1
    + config[ 0], interfaces = 1, ID = 0x01, max required power = 0 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=1:-)
Bus 002, Port 004 [062A:4101] MOSART Semi., 2.4G Keyboard Mouse
    + (SN not found)
    + bcdID = 0110, human readable = USB 1.1
    + config[ 0], interfaces = 2, ID = 0x01, max required power = 100 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=1:-)
        - interface[1] : alt.settings = 1 -> (ep[0]=1:-)
Bus 001, Port 002 [2109:0812] (no manufacturer)(no product name)
    + (SN not found)
    + Class = HUB device
    + bcdID = 0300, human readable = USB 3.0
    + config[ 0], interfaces = 1, ID = 0x01, max required power = 0 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=1:0x6)
Bus 002, Port 022 [2109:0812] (no manufacturer)(no product name)
    + (SN not found)
    + Class = HUB device
    + bcdID = 0300, human readable = USB 3.0
    + config[ 0], interfaces = 1, ID = 0x01, max required power = 0 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=1:0x6)
Bus 002, Port 004 [05E3:0608] (no manufacturer)(no product name)
    + (SN not found)
    + Class = HUB device
    + bcdID = 0200, human readable = USB 2.0
    + config[ 0], interfaces = 1, ID = 0x01, max required power = 100 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=1:-)
Bus 002, Port 010 [046D:C332] Logitech, Gaming Mouse G502
    + Serial number = 188338753330
    + bcdID = 0200, human readable = USB 2.0
    + config[ 0], interfaces = 2, ID = 0x01, max required power = 300 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=1:-)
        - interface[1] : alt.settings = 1 -> (ep[0]=1:-)
Bus 002, Port 006 [2109:2812] (no manufacturer)(no product name)
    + (SN not found)
    + Class = HUB device
    + bcdID = 0210, human readable = USB 2.1
    + config[ 0], interfaces = 1, ID = 0x01, max required power = 0 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=1:-)
Bus 002, Port 000 [8086:A2AF] (no manufacturer)(no product name)
    + (SN not found)
    + Class = HUB device
    + bcdID = 0300, human readable = USB 3.0
    + config[ 0], interfaces = 1, ID = 0x01, max required power = 0 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=1:0x6)
Bus 001, Port 003 [0D8C:000C] (no manufacturer)C-Media USB Headphone Set
    + (SN not found)
    + bcdID = 0110, human readable = USB 1.1
    + config[ 0], interfaces = 3, ID = 0x01, max required power = 500 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=0:)
        - interface[1] : alt.settings = 2 -> (ep[0]=0:,ep[1]=1:0x7)
        - interface[2] : alt.settings = 1 -> (ep[0]=1:-)
Bus 002, Port 009 [04B4:0101] (no manufacturer)Cypress USB Keyboard / PS2 Mouse
    + (SN not found)
    + bcdID = 0110, human readable = USB 1.1
    + config[ 0], interfaces = 2, ID = 0x01, max required power = 100 mA
        - interface[0] : alt.settings = 1 -> (ep[0]=1:-)
        - interface[1] : alt.settings = 1 -> (ep[0]=1:-)
```

* There's more xterm escape coloring option for `-c` or `--color`.
* Also simple view with `-s` or `--simple`.

## Manual configuration

* edit `.config` file to where is libusb-1.0.26, or latest

## Reuired external library,

### libusb-1.0.26 ( for macOS )

* Clone [libusb tag v1.0.26](https://github.com/libusb/libusb/tree/v1.0.26) or [lisbusb latest](https://github.com/libusb/libusb) to your sources managing directory,
* Need to modify libusb-1.0.26 Makefile to including this options for
  - `-arch x86_64 -arch arm64 -mmacosx-version-min=11.0` to
  - each CFLAGS and CXXFLAGS on libusb-1.0.26/libusb/Makefile
* And build
