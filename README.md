# pacific_scales
For the simulator you will need to install these packages
configparser
pyserial

Development setup:
Windows 10 running WSL/Ubuntu
2xSerial/USB 2102 chipset devices 
2102 driver installed for windows 10

Python 3.12 installed on windows
The scales_simulator_config.cfg modified to use USB/serial (COM13 in this case)
pacific_scales_simulator run in windows terminal

WSL/Ubuntu
You need to forward the USB/serial from Windows 10
download usbipd for windows 10 and run in administrator powershell
bind and attach the USB device you will use in Ubuntu as receiver
(see doco for usbipd)

You will need to ensure Ubuntu has chipset driver module installed to access the USB device
$ ls /sys/bus/usb-serial/drivers
If the drivers dont match your chipset you can see which are in the kernel
$ ls /lib/modules/5.15.146.1-microsoft-standard-WSL2/kernel/drivers/usb/serial/
If you dont have a driver matching the chipset for your USB device you will need to recompile the kernel with the appropriate driver
Otherwise you can install a supported module (2102 in my case)
$ sudo insmod /lib/modules/5.15.146.1-microsoft-standard-WSL2/kernel/drivers/usb/serial/cp210x.ko

You should be able to see the USB serial on /dev/ttyUSB0
$ lsusb
Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
Bus 001 Device 014: ID 10c4:ea60 Silicon Labs CP210x UART Bridge
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub

Repository:
git clone https://github.com/Camphibian/pacific_scales.git

$ cd pacific_scales/PacificScales_Client

$ make

$ ./psclient -l

will list all serial devices, select the one the simulator is connected to

$./psclient -c /dev/ttyUSB0

Good Luck!

