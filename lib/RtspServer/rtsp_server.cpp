#include "rtsp_server.h"
#include <esp32-hal-log.h>
#include <ESPmDNS.h>
#include <OV2640Streamer.h>

rtsp_server::rtsp_client::rtsp_client(const WiFiClient &client, OV2640 &cam)
{
	wifi_client = client;
	streamer = std::shared_ptr<CStreamer>(new OV2640Streamer(&wifi_client, cam));
	session = std::shared_ptr<CRtspSession>(new CRtspSession(&wifi_client, streamer.get()));
}

// URI: e.g. rtsp://192.168.178.27:554/mjpeg/1
rtsp_server::rtsp_server(OV2640 &cam, int port /*= 554*/)
	: WiFiServer(port), cam_(cam)
{
}

void rtsp_server::begin()
{
	log_i("Starting RTSP server");
	WiFiServer::begin();

	// Add service to mDNS - rtsp
	MDNS.addService("rtsp", "tcp", 554);
}

void rtsp_server::doLoop()
{
	// Check if a client wants to connect
	auto new_client = accept();
	if (new_client)
		clients_.push_back(std::unique_ptr<rtsp_client>(new rtsp_client(new_client, cam_)));

	// Check if any client connected. If none: nothing to do
	if (clients_.empty())
		return;

	for (const auto &client : clients_)
		client->session->handleRequests(0);

	// 200ms => 5fps
	const auto msec_per_frame = 200;
	static auto last_image = millis();
	auto now = millis();
	if (now > last_image + msec_per_frame || now < last_image)
	{
		// Send the frame
		last_image = now;
		for (const auto &client : clients_)
			client->session->broadcastCurrentFrame(now);

		// check if we are overrunning our max frame rate
		now = millis();
		if (now > last_image + msec_per_frame)
			log_w("warning exceeding max frame rate of %lu ms", now - last_image);
	}

	clients_.remove_if(
		[](std::unique_ptr<rtsp_client> const &c)
		{ return c->session->m_stopped; });
}
