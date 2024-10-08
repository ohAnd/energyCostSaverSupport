# energyCostSaverSupport

- [energyCostSaverSupport](#energycostsaversupport)
  - [idea](#idea)
  - [features](#features)
    - [display](#display)
    - [web ui](#web-ui)
  - [implementation](#implementation)
  - [hardware](#hardware)
      - [first installation to the ESP device (as an example for ESP8266)](#first-installation-to-the-esp-device-as-an-example-for-esp8266)
      - [first setup with access point](#first-setup-with-access-point)
      - [return to factory mode](#return-to-factory-mode)
    - [main](#main)
    - [snapshot](#snapshot)


## idea
With a dynamic energy tariff it is useful to start a energy intensive machine in the household e.g. washing machine, dryer, dishwasher, ... only at times where the price per kWh is cheap.

problem:

Only recent modern devices have already an API or direct application to use dynamic energy tariffs to delay the start of programs. But a lot of older machines have the feature to delay the start of the program per user entry.

So, there is a need to make it very easy for the end user to get the right delay time instantly to use the delay.

Therefore a small display in the space around the target device should show everytime and directly the needed delay, to reach the most cost efficient time period.

> btw. additionally the high energy consumption will be delayed in a "network-friendly" time zone to support a better average load on the surrounding energy net.

## features

The controller getting data from the EPEX with the day-ahead market data every 15 min - here used from [tado/ aWATTar](https://energy.tado.com/services/api) - and will calcuate the best time frame for the given program duration (configurable) and maximum delay time (configurable).

detail:
- program from e.g. a dryer runs 3 hours after start of the program
- maxiumum delay time e.g. 12 hours - means the cheapest time frame should be found with a latest start in 12 hours
- with the given runtime also the energy costs will be calculated and presented

### display
The display shows directly the needed delay hours for starting the program with some data to support the usage of the delay:
- delay hours - configurable mode: time to delay to start the program or time until the program should be finished (depends on the input type of the machine/ device)
- price with direct start
- price with delay
- current date and time
- last update of price data
  
<img src="doc/images/userdisplay.jpg" alt="energyCostSaverSupport_OLED" width="360"/>

### web ui
Shows some more information about the prices per kWh in the next 24 hours and all the shown values on the display. Also the settings for the specific use can be set here:
- energy costs settings
  - fix costs per kWh in € without additonal tax/ fee
  - fix costs per kWh in € non tax free
  - tax rate in % (e.g. 16%)
- consumer device settings
  - Name of your device (dishwasher, washing machine, ...) - only for later better distinguishing with further displays
  - duration / runtime of the program in hours
  - maximum wait time in hours for the start of the program

<img src="doc/images/screenshot_webui.png" alt="energyCostSaverSupport_OLED" width="650"/>
<img src="doc/images/screenshot_webui_mobile.png" alt="energyCostSaverSupport_OLED" width="200"/>

## implementation
Based on the "mini framework" of [dtuGateway](https://github.com/ohAnd/dtuGateway) and so all base features as first setup, display support, web update, user config, ... are supported. Also ESP32 support as a base.

## hardware

- ESP8266/ EPS32 based board
- display SSH1106 OLED 1,3" 128x64 (e.g. [link](https://de.aliexpress.com/item/32881408326.html)):
  - connect SSH1106 driven OLED display (128x64) with your ESP8266/ ESP32 board (VCC, GND, SCK, SCL)
  - pinning for different boards (display connector to ESPxx board pins)

    | dev board                                        | ESP family | VCC  | GND |        SCK       |       SDA        | tested |
    |--------------------------------------------------|------------|:----:|:---:|:----------------:|:----------------:|:------:|
    | AZDelivery D1 Board NodeMCU ESP8266MOD-12F       | ESP8266    | 3.3V | GND | D15/GPIO5/SCL/D3 | D14/GPIO4/SDA/D4 |   OK   |
    | AZDelivery NodeMCU V2 WiFi Amica ESP8266 ESP-12F | ESP8266    | 3.3V | GND | D1/GPIO5/SCL     | D2/GPIO4/SDA     |   OK   |
    | AZDelivery D1 Mini NodeMcu mit ESP8266-12F       | ESP8266    | 3V3  |  G  | D1/GPIO5/SCL     | D2/GPIO4/SDA     |   OK   |
    | ESP-WROOM-32 NodeMCU-32S                         | ESP32      | 3.3V | GND | D22/GPIO22/SCL   | D21/GPIO21/SDA   |   todo   |

#### first installation to the ESP device (as an example for ESP8266)
1. download the preferred release as binary (see below)
2. **HAS TO BE VERIFIED** [only once] flash the esp8266 board with the (esp download tool)[https://www.espressif.com/en/support/download/other-tools]
   1. choose bin file at address 0x0
   2. crystal frequency to 26 Mhz
   3. SPI speed 40 MHz
   4. SPI Mode QIO
   5. Flash Size 32 MBit-C1
   6. select your COM port and baudrate = 921600
   7. press start ;-)
3. all further updates are done by OTA (see chapters above) 

#### first setup with access point

1. connect with the AP energyCostSaverSupport_<chipID> (on smartphone sometimes you have to accept the connection explicitly with the knowledge there is no internet connectivity)
2. open the website http://192.168.4.1 (or http://energyCostSaverSupport.local) for the first configuration
3. choose your wifi
4. type in the wifi password - save
5. after this one time configuration, the device will reboot and try connect to your given network
6. open the webui with the given IP from your local router

#### return to factory mode
1. connect your ESP with serial (115200 baud) in a COM terminal
2. check if you receive some debug data from the device
3. type in `resetToFactory 1`
4. response of the device will be `reinitialize UserConfig data and reboot ...`
5. after reboot the device starting again in AP mode for first setup

### main
latest release - changes will documented by commit messages
https://github.com/ohAnd/energyCostSaverSupport/releases/latest

(to be fair, the amount of downloads is the count of requests from the client to check for new firmware for the OTA update)

![GitHub Downloads (all assets, latest release)](https://img.shields.io/github/downloads/ohand/energyCostSaverSupport/latest/total)
![GitHub (Pre-)Release Date](https://img.shields.io/github/release-date/ohand/energyCostSaverSupport)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/ohand/energyCostSaverSupport/main_build.yml)

### snapshot
snapshot with latest build
https://github.com/ohAnd/energyCostSaverSupport/releases/tag/snapshot

![GitHub Downloads (all assets, specific tag)](https://img.shields.io/github/downloads/ohand/energyCostSaverSupport/snapshot/total)
![GitHub (Pre-)Release Date](https://img.shields.io/github/release-date-pre/ohand/energyCostSaverSupport)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/ohand/energyCostSaverSupport/dev_build.yml)