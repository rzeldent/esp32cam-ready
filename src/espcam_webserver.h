#pragma once

#include <WebServer.h>
#include <OV2640.h>
#include <rtsp_server.h>

class espcam_webserver
{
private:
	const String &instance_name_;
	OV2640 &cam_;
	rtsp_server rtsp_server_;

	WebServer server_;

	void handle_root();
	void handle_reset();
	void handle_jpg_stream();
	void handle_jpg();
	void handle_light_on();
	void handle_light_off();

public:
	espcam_webserver(OV2640 &cam, const String &instance_name);
	void begin();
	void doLoop();
};
