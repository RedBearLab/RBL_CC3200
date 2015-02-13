# RedBearLab CC3200 Wi-Fi Series

Last updated: 2015/02/13

## Install the Add-on Packet

1. Download and install the [Energia](http://energia.nu/download/) on your PC.
2. Clone or download the RedBearLab CC3200 add-on packet for Energia.
3. Navigate to "Energia/hardware/cc3200", copy the file "boards.txt" and the folder "variants" to under the path "\<Energia installing path\>\hardware\cc3200", replacing the existing "boards.txt" and merging the folder "variants". It whould not change the existing boards' configuration.
4. Now you can select the RedBearLab CC3200 Wi-Fi boards on Energia IDE.

## Install the USB CDC Driver

Get this driver and install it if you are using Windows, so that you can use the USB CDC (Virtual COM Port).  
[https://mbed.org/media/downloads/drivers/mbedWinSerial_16466.exe](https://mbed.org/media/downloads/drivers/mbedWinSerial_16466.exe)

**Note that you do not need any driver for OSX and Linux platforms.**

Now you can develop the RedBearLab CC3200 firmware using Energia IDE. Enjoy it!