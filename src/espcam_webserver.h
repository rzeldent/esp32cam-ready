#pragma once

#include <WebServer.h>
#include <OV2640.h>

class espcam_webserver : public WebServer
{
private:
	OV2640 &cam_;
	const String& instance_name_;
	void handle_root();
	void handle_reset();
	void handle_jpg_stream();
	void handle_jpg();
	void handle_light_on();
	void handle_light_off();

public:
	espcam_webserver(OV2640 &cam, const String& instance_name, int port = 80);
	void begin();
	void doLoop();
};
