#include <esp32-hal-log.h>
#include <IotWebConf.h>
#include <espcam_webserver.h>
#include <ESPmDNS.h>
#include <EEPROM.h>

espcam_webserver::espcam_webserver(OV2640 &cam, const String& instance_name, int port /*= 80*/)
	: WebServer(port), cam_(cam), instance_name_(instance_name)
{
	// Set up required URL handlers on the web server
	on("/reset", HTTP_GET, std::bind(&espcam_webserver::handle_reset, this));
	on("/stream", HTTP_GET, std::bind(&espcam_webserver::handle_jpg_stream, this));
	on("/jpg", HTTP_GET, std::bind(&espcam_webserver::handle_jpg, this));
	on("/lighton", HTTP_GET, std::bind(&espcam_webserver::handle_light_on, this));
	on("/lightoff", HTTP_GET, std::bind(&espcam_webserver::handle_light_off, this));
	onNotFound(std::bind(&espcam_webserver::handle_root, this));
}

void espcam_webserver::begin()
{
	log_i("Starting web server");
	WebServer::begin();
	// Add service to MDNS - http
	MDNS.addService("http", "tcp", 80);
}

void espcam_webserver::handle_root()
{
	log_i("handle_root");
	String html(
		"<!DOCTYPE html>"
		"<html lang=\"en\">"
		"<head>"
		"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\"/>"
		"<meta http-equiv=\"Pragma\" content=\"no-cache\">"
		"<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">"
		"<title>"
		"ESP32 CAM"
		"</title>"
		"</head>"
		"<body>"
		"<div class=\"container\">"
		"<h2 class=\"text-center\">ESP32CAM</h2>"
		"<div class=\"alert alert-primary\" role=\"alert\">"
		"rtsp stream available at: <a class=\"alert-link\" href=\"rtsp://" + instance_name_ +".local:554/mjpeg/1\">rtsp://"+ instance_name_ +".local:554/mjpeg/1</a>"
		"</div>"
		"<div class=\"list-group\">"
		"<button type=\"button\" class=\"list-group-item list-group-item-action active\">Options</button>"
		"<a class=\"list-group-item list-group-item-action\" href=\"jpg\">Single frame</a>"
		"<a class=\"list-group-item list-group-item-action\" href=\"stream\">Stream frames</a>"
		"<a class=\"list-group-item list-group-item-action\" href=\"lighton\">Light on</a>"
		"<a class=\"list-group-item list-group-item-action\" href=\"lightoff\">Light off</a>"
		"<a class=\"list-group-item list-group-item-action list-group-item-danger\" href=\"reset\">Reset configuration and restart</a>"
		"</div>"
		"</div>"
		"</body>"
		"</html>");

	send(200, "text/html", html);
}

void espcam_webserver::handle_reset()
{
	log_i("handle_reset");
	// Clear EEPROM
	for (auto t = 0; t < 512; t++)
		EEPROM.write(t, 0);

	sendHeader("Location", "/");
	// See Other
	send(303);
	ESP.restart();
}

void espcam_webserver::handle_jpg_stream()
{
	log_i("handle_jpg_stream");
	sendContent("HTTP/1.1 200 OK\r\n"
				"Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n");

	auto wifi_client = client();
	do
	{
		cam_.run();
		if (!wifi_client.connected())
			break;

		sendContent("--frame\r\n"
					"Content-Type: image/jpeg\r\n\r\n");
		wifi_client.write(reinterpret_cast<char *>(cam_.getfb()), cam_.getSize());
		sendContent("\r\n");
	} while (wifi_client.connected());
}

void espcam_webserver::handle_jpg()
{
	log_i("handle_jpg");
	auto wifi_client = client();
	cam_.run();
	if (!wifi_client.connected())
		return;

	sendContent("HTTP/1.1 200 OK\r\n"
				"Content-disposition: inline; filename=capture.jpg\r\n"
				"Content-type: image/jpeg\r\n\r\n");
	wifi_client.write(reinterpret_cast<const char *>(cam_.getfb()), cam_.getSize());
}

void espcam_webserver::handle_light_on()
{
	log_i("handle_light_on");
	digitalWrite(LED_BUILTIN, true);

	sendHeader("Location", "/");
	// See Other
	send(303);
}

void espcam_webserver::handle_light_off()
{
	log_i("handle_light_off");
	digitalWrite(LED_BUILTIN, false);

	sendHeader("Location", "/");
	// See Other
	send(303);
}

void espcam_webserver::doLoop()
{
	handleClient();
}