#include "wifi_provisioning.h"

wifi_provisioning::wifi_provisioning(const String &instance_name, const String &base_url /* = "/provisioning" */)
    : instance_name_(instance_name), base_url_(base_url)
{
    server_.on(base_url_, HTTP_GET, std::bind(&wifi_provisioning::handle_root_get, this));
    server_.on(base_url_, HTTP_POST, std::bind(&wifi_provisioning::handle_root_post, this));
    server_.onNotFound(std::bind(&wifi_provisioning::handle_unknown, this));
}

wl_status_t wifi_provisioning::connect(int seconds /* = 30 */)
{
    WiFi.mode(WIFI_STA);
    auto connect_result = WiFi.begin();
    while ((connect_result != WL_CONNECTED) && seconds-- > 0)
    {
        log_i("Connecting %d seconds left...", seconds);
        delay(1000);
        connect_result = WiFi.status();
    }

    log_i("Connection result: %d", connect_result);
    return connect_result;
}

void wifi_provisioning::start_portal(const String &ap_password /*= "" */)
{
    log_i("Starting portal");
    WiFi.setAutoConnect(false);

    WiFi.softAP(instance_name_.c_str(), ap_password.length() ? ap_password.c_str() : nullptr);
    auto ip_address = WiFi.softAPIP();
    log_i("AP IP address: %s", ip_address.toString().c_str());

    // Start DNS Server to respond and redirect to the address of the AP
    dns_server_.setErrorReplyCode(DNSReplyCode::NoError);
    dns_server_.start(53, "*", ip_address);

    server_.begin();

    // Scan available networks (async)
    WiFi.scanNetworks(true);
}

void wifi_provisioning::doLoop()
{
    dns_server_.processNextRequest();
    server_.handleClient();
}

void wifi_provisioning::handle_unknown()
{
    log_i("handle_unknown");
    auto root = "http://" + WiFi.softAPIP().toString() + base_url_;
    server_.sendHeader("Location", root);
    // See Other
    server_.send(302);
}

void wifi_provisioning::handle_root_get()
{
    log_i("handle_root_get");
    String ssid_options;
    auto ssid_items = WiFi.scanComplete();
    log_i("ssid Items: %d", ssid_items);
    for (auto index = 0; index < ssid_items; ++index)
    {
        auto ssid = WiFi.SSID(index);
        auto rssi = WiFi.RSSI(index);
        log_i("Adding ssid: %s (%d dBm)", ssid.c_str(), rssi);
        ssid_options += "<option value=\"" + ssid + "\">" + ssid + "(" + rssi + ")</option>";
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
        instance_name_ +
        "</h1>"
        "<hr>"
        "<label for=\"ssids\">Found:</label>"
        "<select name=\"ssids\" onchange=\"document.getElementsByName('ssid')[0].value=this.value\">"
        "<option value=\"\"></option>" +
        ssid_options +
        "</select>"
        "<br />"
        "<form method=\"POST\">"
        "<label for=\"ssid\">SSID:</label>"
        "<input name=\"ssid\" type=\"text\">"
        "<br />"
        "<label for=\"password\">Password:</label>"
        "<input name=\"password\" type=\"password\">"
        "<br />"
        "<input type=\"submit\" value=\"Submit\">"
        "</form>"
        "</body>"
        "</html>");

    server_.send(200, "text/html", html);
}

void wifi_provisioning::handle_root_post()
{
    log_i("handle_root_post");
    String ssid, password;
    if (!server_.hasArg("ssid") || !server_.hasArg("password") || (ssid = server_.arg("ssid")) == nullptr || (password = server_.arg("password")) == nullptr)
    {
        server_.send(400, "text/plain", "400: Invalid Request");
        return;
    }

    log_i("SSID: %s, password: %s", ssid.c_str(), password.c_str());

    WiFi.softAPdisconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.setAutoConnect(true);

    auto connection_result = (wl_status_t)WiFi.waitForConnectResult();
    log_i("Connection result: %d", connection_result);
    ESP.restart();
}