#include "wifi_provisioning.h"
#include <ESPmDNS.h>

wifi_provisioning::wifi_provisioning(const String &instance_name, const String &ap_password, int port /* = 80*/)
    : WebServer(port), instance_name_(instance_name), ap_password_(ap_password), provisioned_(false)
{
    onNotFound(std::bind(&wifi_provisioning::handle_root_get, this));
    on("/", HTTP_GET, std::bind(&wifi_provisioning::handle_root_get, this));
    on("/", HTTP_POST, std::bind(&wifi_provisioning::handle_root_post, this));
}

void wifi_provisioning::sta()
{
   WiFi.mode(WIFI_STA);
 }

void wifi_provisioning::ap()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(instance_name_.c_str(), ap_password_.c_str(), random(13));
    auto ip = WiFi.softAPIP();
    log_i("AP IP address: %s", ip.toString());
    // Start DNS Server to respond and redirect to the address of the AP
    dns_server.start(53, "*", ip);
}

bool wifi_provisioning::provisioned()
{
    return provisioned_;
}

bool wifi_provisioning::connect()
{
    sta();
    WiFi.begin();
    auto connection_result = WiFi.waitForConnectResult();
    log_i("Connection result: %d", connection_result);
    if (provisioned_ = connection_result == WL_CONNECTED)
        return true;

    // Start scanning for networks (async)
    WiFi.scanNetworks(true);
    return false;
}

void wifi_provisioning::begin()
{
    log_i("Starting wifi provisioning");
    connect();
}

void wifi_provisioning::start_portal()
{
    log_i("Starting portal");
    ap();
    WebServer::begin();
    // Add service to MDNS - http
    MDNS.addService("http", "tcp", 80);
}

void wifi_provisioning::stop_portal()
{
    log_i("Stopping web server");
}

void wifi_provisioning::doLoop()
{
    handleClient();
}

void wifi_provisioning::handle_root()
{
    log_i("handle_root");
    String html(
        "<!DOCTYPE html>"
        "<html lang=\"en\">"
        "<head>"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\"/>"
        "<meta http-equiv=\"Pragma\" content=\"no-cache\">"
        "<title>" +
        instance_name_ +
        "</title>"
        "</head>"
        "<body>"
        "</body>"
        "</html>");

    send(200, "text/html", html);
}
