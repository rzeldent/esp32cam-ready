#pragma once

#include <WebServer.h>
#include <DNSServer.h>
#include <WiFi.h>

class wifi_provisioning
{
private:
	const String &instance_name_;
	const String base_url_;

	WebServer server_;
	DNSServer dns_server_;

	void handle_unknown();
	void handle_root_get();
	void handle_root_post();

public:
	wifi_provisioning(const String &instance_name, const String& base_url = "/provisioning");
	wl_status_t connect(int seconds = 30);

	void start_portal(const String &ap_password = "");
	void doLoop();
};