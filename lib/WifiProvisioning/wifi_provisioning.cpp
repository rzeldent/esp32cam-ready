#include "wifi_provisioning.h"
#include <ESPmDNS.h>

wifi_provisioning::wifi_provisioning(const String &instance_name, const String &ap_password /* ="" */, int port /* = 80*/)
    : WebServer(port), instance_name_(instance_name), ap_password_(ap_password), provisioned_(false)
{
    onNotFound(std::bind(&wifi_provisioning::handle_root_get, this));
    on("/", HTTP_GET, std::bind(&wifi_provisioning::handle_root_get, this));
    on("/", HTTP_POST, std::bind(&wifi_provisioning::handle_root_post, this));

    WiFi.begin();
    auto connection_result = (wl_status_t)WiFi.waitForConnectResult();
    log_i("Connection result: %d", connection_result);
    if (connection_result != WL_CONNECTED)
    {
        start_portal();
        while (true)
        {
            handleClient();
            sleep(0);
        }
    }
}

void wifi_provisioning::start_ap()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(instance_name_.c_str(), ap_password_.c_str());
    auto ip = WiFi.softAPIP();
    log_i("AP IP address: %s", ip.toString().c_str());
    // Start DNS Server to respond and redirect to the address of the AP
    dns_server.setErrorReplyCode(DNSReplyCode::NoError);
    dns_server.start(53, "*", ip);
}

void wifi_provisioning::stop_ap()
{
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
}

void wifi_provisioning::start_portal()
{
    log_i("Starting portal");
    start_ap();
    begin();
    // Add service to MDNS - http
    MDNS.addService("http", "tcp", 80);
}

void wifi_provisioning::handle_root_get()
{
    log_i("handle_root_get");
    String ssid_options;
    auto ssid_items = WiFi.scanComplete();
    for (auto index = 0; index < ssid_items; ++index)
    {
        auto ssid = WiFi.SSID(index);
        ssid_options += "<option value=\"" + ssid + "\">" + ssid + "</option>";
    }

    String html(
        "<!DOCTYPE html>"
        "<html lang=\"en\">"
        "<head>"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\" />"
        "<meta http-equiv=\"Pragma\" content=\"no-cache\">"
        "<title>" +
        instance_name_ +
        "</title>"
        "<style>"
        "label {width:80px;display:block;float:left;clear:left;}"
        "select,input,button {display:block;border-radius:5px;outline:none;}"
        "</style>"
        "</head>"
        "<body>"
        "<h1>" +
        instance_name_ + "</h1>"
                         "<hr>"
                         "<label for=\"ssids\">Found:</label>"
                         "<select name=\"ssids\" onchange=\"document.getElementByName('ssid')[0].value=this.value\">" +
        ssid_options +
        "</select>"
        "<br />"
        "<form action=\"/\" method=\"POST\">"
        "<label for=\"ssid\">SSID:</label>"
        "<input name=\"ssid\" type=\"text\">"
        "<br />"
        "<label for=\"password\">Password:</label>"
        "<input name=\"password\" type=\"password\">"
        "<br />"
        "<button type=\"submit\">Submit</button>"
        "</form>"
        "</body>"
        "</html>");

    send(200, "text/html", html);
}

void wifi_provisioning::handle_root_post()
{
    log_i("handle_root_post");
    if (!hasArg("ssid") || !hasArg("password") || arg("ssid") == nullptr || arg("password") == nullptr)
    {
        send(400, "text/plain", "400: Invalid Request");
        return;
    }

    auto ssid = arg("ssid");
    auto password = arg("password");
    log_i("SSID: %s, password: %s", ssid.c_str(), password.c_str());

    stop_ap();
    WiFi.begin(ssid.c_str(), password.c_str());
    auto connection_result = (wl_status_t)WiFi.waitForConnectResult();
    log_i("Connection result: %d", connection_result);
    WiFi.setAutoConnect(connection_result == WL_CONNECTED);
    ESP.restart();
}