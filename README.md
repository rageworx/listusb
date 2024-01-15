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
>  bin/listusb -c
listusb, version 0.2.3.27, (C)Copyrighted 2023 Raphael Kim, w/ libusb v1.0.27
Bus 001, Port 002 [0BDA:9210] Realtek, RTL9210-VB
    + Serial number = 821027768393
    + bcdID = 0320, human readable = USB 3.2
    + config[ 0], interfaces = 1, ID = 0x01, max required power = 896 mA
        - interface[0] : alt.settings = 2 : Mass storage device, Mass storage device
            -> ep[0]=2:02 ( Bulk, Interrupt, ) EP:IN, 06300F000000
                       02 ( Bulk, Interrupt, ) EP:OUT, 06300F000000
            -> ep[1]=4:02 ( Bulk, Interrupt, ) EP:IN, 06300F05000004240300
                       02 ( Bulk, Interrupt, ) EP:OUT, 06300F05000004240400
                       02 ( Bulk, Interrupt, ) EP:IN, 06300F05000004240200
                       02 ( Bulk, Interrupt, ) EP:OUT, 06300000000004240100
total 1 device found.
```

* There's more xterm escape coloring option for `-c` or `--color`.
* Also simple view with `-s` or `--simple`.
* Tree view availed with `-t` or `--tree`.

## Manual configuration

* edit `.config` file to where is libusb-1.0.26, or latest

## Reuired external library,

### libusb-1.0.26 or later ( for macOS )

* Clone [libusb tag v1.0.26](https://github.com/libusb/libusb/tree/v1.0.26) or [lisbusb latest](https://github.com/libusb/libusb) to your sources managing directory,
* Need to modify libusb-1.0.26 Makefile to including this options for
  - `-arch x86_64 -arch arm64 -mmacosx-version-min=11.0` to
  - each CFLAGS and CXXFLAGS on libusb-1.0.26/libusb/Makefile
* And build
