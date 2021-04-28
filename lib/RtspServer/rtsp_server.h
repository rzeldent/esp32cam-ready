#pragma once

#include <list>
#include <WiFiServer.h>
#include <ESPmDNS.h>
#include <OV2640.h>
#include <CRtspSession.h>

class rtsp_server : public WiFiServer
{
private:
	struct rtsp_client
	{
		WiFiClient wifi_client;
		// Streamer for UDP/TCP based RTP transport
		std::shared_ptr<CRtspSession> session;
		// RTSP session and state
		std::shared_ptr<CStreamer> streamer;
		rtsp_client(const WiFiClient &client, OV2640 &cam);
	};

	OV2640 &cam_;
	std::list<std::unique_ptr<rtsp_client>> clients_;

public:
	rtsp_server(OV2640 &cam, int port = 554);
	void begin();

	void doLoop();
};
