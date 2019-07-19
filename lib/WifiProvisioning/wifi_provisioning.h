#pragma once

#include <WebServer.h>
#include <DNSServer.h>
#include <WiFi.h>

class wifi_provisioning : public WebServer
{
public:
	wifi_provisioning(const String &instance_name, const String& ap_password, int port = 80);
	bool provisioned();

	void begin();

	void start_portal();
	void stop_portal();

	void doLoop();

private:
	DNSServer dns_server;
	const String &instance_name_;
	const String &ap_password_;
	bool provisioned_;

	bool connect();

void sta();
void ap();

	void handle_root_get();
	void handle_root_post();
};