# listusb
A detailed listing USB device for macOS shell(console).

## What is listusb
* macOS don't have tool to what USB device connected in shell.
* There's lsusb in brew usbtools, but not functional to check detailed USB information.
* listusb helps what USB device connected, and help to correctly identified to my macOS as real device supported speed - specially NVMe SSD devices easily not recognized to USB2 rather than USB3 ! 

## Easy to read
* Build source, and install to user space, then type `listusb`.
```
>  listusb
Device VID:PID [152D:0583] JMicron, USB to PCIE Bridge
    + Serial number = 0123456789ABCDEF
    + bcdID=0210, human readable = USB 2.1
    + config[ 0] , interfaces=1, ID=0x01, max req. power=500 mA
Device VID:PID [0BDA:9210] Realtek, RTL9210
    + Serial number = 012345679021
    + bcdID=0320, human readable = USB 3.2
    + config[ 0] , interfaces=1, ID=0x01, max req. power=896 mA
Device VID:PID [05E3:0610] GenesysLogic, USB2.0 Hub
    + (SN not found)
    + CLASS=HUB device
    + bcdID=0210, human readable = USB 2.1
    + config[ 0] , interfaces=1, ID=0x01, max req. power=100 mA
Device VID:PID [05E3:0612] GenesysLogic, USB3.0 Hub
    + (SN not found)
    + CLASS=HUB device
    + bcdID=0300, human readable = USB 3.0
    + config[ 0] , interfaces=1, ID=0x01, max req. power=0 mA
```
* In case of un-successful recognization of NVMe SSD, as like above,
* There's same USB VID:PID HUB, but one is USB 3.0, and another is USB 2.1.
* This case is one of NVMe SSD enclosure failed to recognized as USB3 device, as like JMicron USB to PCIE Bridge - it shows USB 2.1 and power consume to 500mAh not as USB3.

## Manual configuration
* edit .config to where is libusb-1.0.26

## Reuired external library,
### libusb-1.0.26
* Clone [libusb tag v1.0.26](https://github.com/libusb/libusb/tree/v1.0.26) to your sources managing directory,
* Need to modify libusb-1.0.26 Makefile to including this options for 
    - `-arch x86_64 -arch arm64 -mmacosx-version-min=11.0` to 
	- each CFLAGS and CXXFLAGS on libusb-1.0.26/libusb/Makefile
* And build

