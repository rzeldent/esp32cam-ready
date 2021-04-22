#include <Arduino.h>

#include <DNSServer.h>
#include <wifi_provisioning.h>

#include <espcam_webserver.h>

#include <OV2640.h>

#include "soc/rtc_cntl_reg.h"

const char app_name[] = "esp32cam";
const char ap_password[] = "esp32cam#";

String get_mac_address()
{
	auto mac = WiFi.macAddress();
	mac.replace(":", "");
	mac.toLowerCase();
	return mac;
}

auto instance_name = String(app_name) + "-" + get_mac_address();

OV2640 cam;
espcam_webserver espcam_web(cam, instance_name);

// put your setup code here, to run once:
void setup()
{
	// Disable brownout
	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

	Serial.begin(115200);
	Serial.setDebugOutput(true);
	esp_log_level_set("*", ESP_LOG_VERBOSE);

	log_i("CPU Freq = %d Mhz", getCpuFrequencyMhz());
	log_i("Starting ESP32Cam...");

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, false);

	log_i("Initialize the camera");
	esp32cam_aithinker_config.frame_size = FRAMESIZE_UXGA;
	if (cam.init(esp32cam_aithinker_config) != ESP_OK)
		log_e("Initializing the camera failed");

	log_i("Instance_name: %s", instance_name.c_str());

	log_i("Connecting...");
	wifi_provisioning provisioning(instance_name);
	auto const portal_timeout_milliseconds = (uint)3 * 60 * 1000;
	if (provisioning.connect() != WL_CONNECTED)
	{
		log_i("Provisioning...");
		provisioning.start_portal(ap_password);
		auto start = millis();
		while (WiFi.softAPgetStationNum() > 0 || millis() - start < portal_timeout_milliseconds)
			provisioning.doLoop();

		log_i("Provisioning timeout. Restarting...");
		ESP.restart();
	}

	log_i("Connected! IP address: %s", WiFi.localIP().toString().c_str());

	log_i("Starting servers...");

	espcam_web.begin();
}

void loop()
{
	espcam_web.doLoop();
	yield();
}
