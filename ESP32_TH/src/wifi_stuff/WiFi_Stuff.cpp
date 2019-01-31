#include "WiFi_Stuff.h"
//#include "page.h"

WiFiStuff::WiFiStuff()
{
  //empty constructor
}

void WiFiStuff::setupPortal()
{
    dnsServer.reset(new DNSServer());
    wwwServer.reset(new WebServer(80));

    DEBUG_WM(F("SetUp"));

    DEBUG_WM(F("Configuring access point... "));
    DEBUG_WM(_apName);

    WiFi.softAP(_apName);
    delay(500); // Without delay I've seen the IP address blank
    DEBUG_WM(F("AP IP address: "));
    DEBUG_WM(WiFi.softAPIP());
    delay(500); // needed?

    /* Setup the DNS server redirecting all the domains to the apIP */
    dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

    /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
    wwwServer->on("/", std::bind(&WiFiStuff::handleRoot, this));
    wwwServer->on("/config",std::bind(&WiFiStuff::handleConfig,this));
    wwwServer->on("/configsave",std::bind(&WiFiStuff::handleConfigSave,this));

    wwwServer->on("/data",std::bind(&WiFiStuff::handleData,this));
    wwwServer->on("/wipe",std::bind(&WiFiStuff::handleWipe,this));

    wwwServer->on("/fwlink", std::bind(&WiFiStuff::handleRoot, this));
    //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.

    wwwServer->onNotFound (std::bind(&WiFiStuff::handleNotFound, this));
    wwwServer->begin(); // Web server start
    DEBUG_WM(F("HTTP server started"));
}


bool WiFiStuff::startPortal(char const *apName)
{
  //setup AP
  WiFi.mode(WIFI_AP_STA);
  DEBUG_WM("SET AP STA");

  _apName = apName;

  //notify we entered AP mode
  if ( _apcallback != NULL) {
    _apcallback(this);
  }
  connect = false;
  setupPortal();
  while(1)
  {
    //DNS
    dnsServer->processNextRequest();
    //HTTP
    wwwServer->handleClient();

    yield();
  }

  wwwServer.reset();
  dnsServer.reset();

  return  WiFi.status() == WL_CONNECTED;

}


/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
bool WiFiStuff::captivePortal() {
  if (!isIp(wwwServer->hostHeader()) ) {
    DEBUG_WM(F("Request redirected to captive portal"));
    wwwServer->sendHeader("Location", String("http://") + toStringIp(wwwServer->client().localIP()), true);
    wwwServer->send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    wwwServer->client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}





// Page handlers

/** Handle root or redirect to captive portal */
void WiFiStuff::handleRoot() {
  DEBUG_WM(F("Handle root"));
    if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
      return;
    }
  String pageHTML;
  SPIFFS.begin();
  File file = SPIFFS.open("/portal.html");
            if(!file || file.isDirectory())
            {
            wwwServer->send ( 500, "text/html", "- failed to open file for reading");
            DEBUG_WM(F("- failed to open file for reading"));
            return;
            }
  while(file.available())
    {
      pageHTML = file.readString();
    }
  wwwServer->send(200,"text/html",pageHTML);
            size_t sent = wwwServer->streamFile(file, "text/html");
            delay(500);
            DEBUG_WM(F("- read and sent file OK"));
            DEBUG_WM(sent);
            //Serial.println("- read file OK");
            //captiveDone = true;
  file.close();
}

void WiFiStuff::handleConfig() {
  DEBUG_WM(F("Handle Config"));
    if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
      return;
    }
  String pageHTML;
  SPIFFS.begin();
  File file = SPIFFS.open("/config.html");
            if(!file || file.isDirectory())
            {
            wwwServer->send ( 500, "text/html", "- failed to open file for reading");
            DEBUG_WM(F("- failed to open file for reading"));
            return;
            }
  while(file.available())
    {
      pageHTML = file.readString();
    }
  wwwServer->send(200,"text/html",pageHTML);
            size_t sent = wwwServer->streamFile(file, "text/html");
            delay(500);
            DEBUG_WM(F("- read and sent file OK"));
            DEBUG_WM(sent);
  file.close();
}

void WiFiStuff::handleConfigSave() {

  //save the values to config.txt

//  DEBUG_WM(F("Handle ConfigSave"));
//  _ssid = server->arg("s").c_str();
//  _pass = server->arg("p").c_str();

}




void WiFiStuff::handleData() {
  DEBUG_WM(F("Handle Data"));
    if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
      return;
    }
  String pageHTML;
  SPIFFS.begin();
  File file = SPIFFS.open("/data.html");
            if(!file || file.isDirectory())
            {
            wwwServer->send ( 500, "text/html", "- failed to open file for reading");
            DEBUG_WM(F("- failed to open file for reading"));
            return;
            }
  while(file.available())
    {
      pageHTML = file.readString();
    }
  wwwServer->send(200,"text/html",pageHTML);
            size_t sent = wwwServer->streamFile(file, "text/html");
            delay(500);
            DEBUG_WM(F("- read and sent file OK"));
            DEBUG_WM(sent);
  file.close();
}

void WiFiStuff::handleWipe() {
  DEBUG_WM(F("Handle Wipe"));
    if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
      return;
    }
  String pageHTML;
  SPIFFS.begin();
  File file = SPIFFS.open("/wipe.html");
            if(!file || file.isDirectory())
            {
            wwwServer->send ( 500, "text/html", "- failed to open file for reading");
            DEBUG_WM(F("- failed to open file for reading"));
            return;
            }
  while(file.available())
    {
      pageHTML = file.readString();
    }
  wwwServer->send(200,"text/html",pageHTML);
            size_t sent = wwwServer->streamFile(file, "text/html");
            delay(500);
            DEBUG_WM(F("- read and sent file OK"));
            DEBUG_WM(sent);
  file.close();
}

void WiFiStuff::handleNotFound() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
    return;
  }
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += wwwServer->uri();
  message += "\nMethod: ";
  message += ( wwwServer->method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += wwwServer->args();
  message += "\n";

  for ( uint8_t i = 0; i < wwwServer->args(); i++ ) {
    message += " " + wwwServer->argName ( i ) + ": " + wwwServer->arg ( i ) + "\n";
  }
  wwwServer->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  wwwServer->sendHeader("Pragma", "no-cache");
  wwwServer->sendHeader("Expires", "-1");
  wwwServer->sendHeader("Content-Length", String(message.length()));
  wwwServer->send ( 404, "text/plain", message );
}


// Utilities
template <typename Generic>
void WiFiStuff::DEBUG_WM(Generic text) {
  if (_debug) {
    Serial.print("*WM: ");
    Serial.println(text);
  }
}

/** Is this an IP? */
bool WiFiStuff::isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String WiFiStuff::toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
