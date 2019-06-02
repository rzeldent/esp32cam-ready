# esp32cam-ready

esp32-ready cam combines other projects to have an out-the-box solution to use the Chinese (7 Euro) esp32cam module.

# Usage
Download the repo, open it in platformIO and flash it to the esp32cam.
The device should become available as an accesspoint with the name esp32cam-xxxx.
Connect to the accesspoint and configure the ssid/password in the browser on on the address [http://192.168.4.1](http://192.168.4.1).
When the credentials are valid and the device connects to the infrastructure, the device can be accessed over http using the link [http://esp32cam.local](http://esp32cam.local) (or the local ip address) from your browser.

Using the browser, you can
- Take a snapshow
- Stream video
- Turn the light on/off
- Remove the Wifi configuration.

# Credits
Esp32cam-ready depends on PlatformIO and two other libraries:
- Micro-RTSP by Kevin Hester
- IotWebConf by Balázs Kelemen

esp32-ready basically extends the Micro-RTSP with mutiple client connections and some interfacing.
Thanks for making these tools and libraries available.

