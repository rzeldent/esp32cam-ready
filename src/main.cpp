#include <Arduino.h>

#include <DNSServer.h>
#include <IotWebConf.h>

#include "espcam_webserver.h"
#include <rtsp_server.h>

#include <ESPmDNS.h>
#include <OV2640.h>

#include "soc/rtc_cntl_reg.h"

const char ap_name[] = "esp32cam";
const char ap_password[] = "esp32cam";

OV2640 cam;
rtsp_server rtsp(cam);
espcam_webserver espcam_web(cam);

// put your setup code here, to run once:
void setup()
{
	// Disable brownout
	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

	Serial.begin(115200);
	Serial.setDebugOutput(true);
	esp_log_level_set("*", ESP_LOG_VERBOSE);

	log_i("Starting ESP32Cam...");

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, false);

	auto chip_id = ESP.getEfuseMac();

	DNSServer dnsServer;
	WebServer server;
	auto thingname = String(ap_name) + "-" + String((unsigned int)(chip_id >> 32), HEX) + String((unsigned int)(chip_id), HEX);
	log_i("ChipId: %s", thingname.c_str());
	IotWebConf iotWebConf(thingname.c_str(), &dnsServer, &server, ap_password);
	iotWebConf.init();
	iotWebConf.setWifiConnectionTimeoutMs(1000 * 30);

	// Set up required URL handlers on the web server.
	server.onNotFound([&]() { iotWebConf.handleNotFound(); });
	server.on("/", [&]() { iotWebConf.handleConfig(); });

	while (iotWebConf.getState() != IOTWEBCONF_STATE_ONLINE)
	{
		iotWebConf.doLoop();
		sleep(0);
	}

	server.close();
	dnsServer.stop();

	if (!MDNS.begin(ap_name))
		log_w("Error setting up MDNS responder!");

	log_i("Starting servers...");
	// Initialize the camera
	esp32cam_aithinker_config.frame_size = FRAMESIZE_SVGA;
	cam.init(esp32cam_aithinker_config);
	rtsp.begin();
	espcam_web.begin();
}

void loop()
{
	rtsp.doLoop();
	espcam_web.doLoop();
	sleep(0);
}
