#include "WiFi_Stuff.h"
#include <time.h>

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
    if (captivePortal()) { // If captive portal redirect instead of displaying the page.
      return;
    }
    String _sensorID;
    unsigned int _interval;
    int8_t _caloffset;
    File configfile = SPIFFS.open("/config.txt");
    if(!configfile || configfile.isDirectory())
      {
      // file not found, so use some defaults
       _sensorID = "Sensor";
       _interval = 300;
       _caloffset = 0;
      }

    while(configfile.available())
      {
      _sensorID = configfile.readStringUntil('\n');
      _interval = configfile.readStringUntil('\n').toInt();
      _caloffset = configfile.readStringUntil('\n').toInt();
      }

      String pageHTML = prepConfigPage();
      
      // there is an error here, it's replacing the wrong parts.
      // fill in the values loadde from config.txt or defaults
      pageHTML.replace("{title}","Configure Sensor");
      pageHTML.replace("{s}", _sensorID);
      pageHTML.replace("{i}", String(_interval));
      pageHTML.replace("{hc}", String(_caloffset));

      // get current date and time from RTC and fill those in too.
      char buff[25];
      time_t now = time(0);
      strftime(buff, 25, "%d-%m-%Y", localtime(&now));
      pageHTML.replace("{date}", String(buff));
      strftime(buff, 25, "%H:%M:%S", localtime(&now));
      pageHTML.replace("{time}", String(buff));
      wwwServer->send(200, "text/html", pageHTML);
      //size_t sent = wwwServer->streamFile(file, "text/html");
      delay(100);
      DEBUG_WM(F("- read and sent file OK"));
}

void WiFiStuff::handleConfigSave() {
String _sensorID;
unsigned int _interval;
int8_t _caloffset;
DEBUG_WM("Save");
DEBUG_WM(wwwServer->arg("name").c_str());
DEBUG_WM(wwwServer->arg("int").c_str());
DEBUG_WM(wwwServer->arg("hc").c_str());

DEBUG_WM("Saved values");
_sensorID = wwwServer->arg("name");
_interval = std::strtoul(wwwServer->arg("int").c_str(),nullptr,0);
_caloffset = std::strtoul(wwwServer->arg("hc").c_str(),nullptr,0);

DEBUG_WM("are:");
DEBUG_WM(_sensorID.c_str());
DEBUG_WM(_interval);
DEBUG_WM(_caloffset);


SPIFFS.begin();
File file = SPIFFS.open("/config.txt","w");
if(!file)
  {
    DEBUG_WM("Can't overwrite config.txt");
    return;
  }

// TODO : Should probably check these write complete
file.println(_sensorID);
file.println(_interval)+10;
file.println(_caloffset);
file.close();

// Save Date and Time to RTC
setDateTime(wwwServer->arg("date"),wwwServer->arg("time"));

//debugging - comment out
//File checkfile = SPIFFS.open("/config.txt","r");
//DEBUG_WM("Reading from file:");

//    while(checkfile.available())
//    {
//      Serial.write(checkfile.read());
//   }
//checkfile.close();

 String pageHTML;
      pageHTML = prepConfigPage();
      pageHTML.replace("{title}","Values Saved");
      pageHTML.replace("{s}", _sensorID);
      pageHTML.replace("{i}", String(_interval));
      pageHTML.replace("{hc}", String(_caloffset));

      // get current date and time from RTC and fill those in too.
      char buff[25];
      time_t now = time(0);
      strftime(buff, 25, "%d-%m-%Y", localtime(&now));
      pageHTML.replace("{date}", String(buff));
      strftime(buff, 25, "%H:%M:%S", localtime(&now));
      pageHTML.replace("{time}", String(buff));
      wwwServer->send(200, "text/html", pageHTML);
}

bool WiFiStuff::setDateTime(String _date, String _time)
{
  /*
  *WM 24-12-2019
  *WM 21:37:56
  */
  DEBUG_WM("setDateTime:");
  DEBUG_WM(_date);
  DEBUG_WM(_time);

  // Serial.println(_date.substring(6,10));
  // Serial.println(_date.substring(3,5));
  // Serial.println(_date.substring(0,2));

  // Serial.println(_time.substring(0,2));
  // Serial.println(_time.substring(3,5));
  // Serial.println(_time.substring(6,8));

  // Serial.printf("Year = %ld", (_date.substring(6,10).toInt()));
  // Serial.println();
  // Serial.printf("Month = %ld", ((_date.substring(3,5).toInt())));
  // Serial.println();
  // Serial.printf("Day = %ld", ((_date.substring(0,2).toInt())));
  // Serial.println();
  // Serial.printf("Hour = %ld", ((_time.substring(0,2).toInt())));
  // Serial.println();
  // Serial.printf("Min = %ld", ((_time.substring(3,5).toInt())));
  // Serial.println();
  // Serial.printf("Sec = %ld", ((_time.substring(6,8).toInt())));
  // Serial.println();

  
  struct tm tm;
  tm.tm_year = ((_date.substring(6,10).toInt()) - 1900);// 2018 - 1900;
  tm.tm_mon =  ((_date.substring(3,5).toInt())-1); // dunno why this needs the -1. 
  tm.tm_mday = ((_date.substring(0,2).toInt()));
  tm.tm_hour = ((_time.substring(0,2).toInt()));
  tm.tm_min =  ((_time.substring(3,5).toInt()));
  tm.tm_sec =  ((_time.substring(6,8).toInt()));
  time_t t = mktime(&tm);
  printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
  
  // char buff[25];
  // time_t rightnow = time (0);
  // strftime (buff, 25, "%Y-%m-%d %H:%M:%S", localtime (&rightnow));
  // Serial.println(buff);
  return true;
}

String WiFiStuff::prepConfigPage()
{
  String pageHTML;
  SPIFFS.begin();
  File file = SPIFFS.open("/config.html");
  if (!file || file.isDirectory())
  {
    pageHTML = "- failed to open file for reading";
    wwwServer->send(500, "text/html", pageHTML.c_str());
    DEBUG_WM(pageHTML.c_str());
    return pageHTML;
  }
  while (file.available())
  {
    pageHTML = file.readString();
  }

  file.close();
  return pageHTML;
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
