# esp32cam-ready

esp32-ready cam combines other projects to have an out-the-box solution to use the Chinese (7 Euro!) esp32cam module.
Suggestions and bug fixes are welcome!

## Usage
Download the repo, open it in platformIO and flash it to the esp32cam.
The device should become available as an accesspoint with the name esp32cam-xxxx.
Connect to the accesspoint and configure the ssid/password in the browser on on the address [http://192.168.4.1](http://192.168.4.1).
When the credentials are valid and the device connects to the infrastructure, the device can be accessed over http using the link [http://esp32cam.local](http://esp32cam.local) (or the local ip address) from your browser.

RTSP stream is available at: [rtsp://esp32cam.local:554/mjpeg/1](rtsp://esp32cam.local:554/mjpeg/1)

Using the browser, you can
- Take a snapshow
- Stream video
- Turn the light on/off
- Remove the Wifi configuration.

## Installing and running PlatformIO

Install platformIO (Debian based systems)
```
 sudo apt-get install python-pip
 sudo pip install platformio
 pio upgrade
```
for Windows users, install Visual Studio code, Python and install the PlatformIO plugin.
For command line usage Python and PlatformIO-Core is sufficient.

Clone this repository, go into the folder and type:
```
 pio run
```
Put a jumper between IO0 and GND, press reset and type:
```
 pio run -t upload
```
When done remove the jumper and press reset. To monitor the output, start a terminal using:
```
 pio device monitor
```

## Credits
Esp32cam-ready depends on PlatformIO and two other libraries:
- Micro-RTSP by Kevin Hester
- IotWebConf by Balázs Kelemen

esp32-ready basically extends the Micro-RTSP with mutiple client connections and some interfacing.

Thanks for the comminity making these tools and libraries available.

Also thanks to Espressif and the guys that created this device. You did a great job!
