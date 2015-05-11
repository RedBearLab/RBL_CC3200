# RedBearLab CC3200 Wi-Fi Series

Last updated: 2015/05/07

## Introduction

[RBL CC3200 and WiFi Mini](http://redbearlab.com/) are develop boards with low power WiFi capability, they are designed for fast prototyping of IoT projects.

This add-on allows you to start development using the [Energia](http://energia.nu/download/) IDE for these two boards on Windows, Mac OSX and Linus platforms.

** Tested with version 0101E0015 (3/24/2015).

## Getting Started

Follow this getting started guide to test the board out of the box:

  http://redbearlab.com/getting-start-with-cc3200/

## Install the Add-on Package

To start doing your own firmware development for the CC3200 boards, follow these steps:

1. Download and install the [Energia](http://energia.nu/download/) on your PC.
2. Clone or download the RedBearLab CC3200 add-on package for Energia.
3. Navigate to "Energia/hardware/cc3200", copy the file "boards.txt" and the folder "variants" to under the path "\<Energia installing path\>\hardware\cc3200", replacing the existing "boards.txt" and merging the folder "variants". It whould not change the existing boards' configuration.
4. Now you can select the RedBearLab CC3200 Wi-Fi boards on Energia IDE.

## Install USB CDC Driver (Windows Only)

Get [this driver](https://mbed.org/media/downloads/drivers/mbedWinSerial_16466.exe) and install it if you are using Windows, so that you can use the USB CDC (Virtual COM Port).  

## Install CC3200 LaunchPack Driver (Windows Only)

The uploader program -- cc3200prog.exe requires ftdxx.dll to run, already included in this add-on, but if you have any problems, download and install [CC3200 LaunchPack Driver](http://energia.nu/guide/guide_windows/) to try.

**Note that you do not need any driver for OSX and Linux platforms.**

## Start Coding

Now you can develop the RedBearLab CC3200 firmware using Energia IDE. Enjoy it!

## Known Issues

Since we are using MK20 as the interface chip, there are some issues and will be fixed soon.

1. Current version of Energia will show some error messages after uploading sketches, just ignore them for now and we will fix it soon, the sketches should be uploaded correctly to the CC3200.

2. After uploading sketches, you need to press the reset button.
