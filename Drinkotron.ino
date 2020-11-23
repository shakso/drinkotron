#include "HX711.h"

#include <Wire.h>
#include "Adafruit_MCP23008.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <StringSplitter.h>

const char *ssid         = "xxx";
const char *password     = "xxx";

Adafruit_MCP23008 mcp;
HX711 scale;
ESP8266WebServer server(80);
int scaleFactor = 420;

uint8_t dataPin = 0;
uint8_t clockPin = 2;

static const char ICACHE_FLASH_ATTR homePage[] = "<!DOCTYPE html> <html> <head> <title>drinkOtron</title> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <link rel=\"stylesheet\" href=\"http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.css\"/><script src=\"http://code.jquery.com/jquery-1.9.1.min.js\"></script><script src=\"http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.js\"></script><style type=\"text/css\"></style></head> <body> <div data-role=\"page\" id=\"menu\"><div data-role=\"header\" data-position=\"fixed\"><h1>drinkOtron</h1><a href=\"#send\" id=\"send\" data-icon=\"check\" class=\"ui-state-disabled ui-btn-right\">Make</a></div><div data-role=\"content\" class=\"ui-content\"><ul data-role=\"listview\" data-inset=\"true\" id=\"cocktailList\"><li data-role=\"list-divider\">Long Drinks</li><li data-name=\"1-V|5-O\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ScrewDriver.png\"><h2>Screwdriver</h2><p>1 x Vodka<br>1 x Orange Juice</p></a></li><li data-name=\"2-V|5-O\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ScrewDriver.png\"><h2><b>Large</b> Screwdriver</h2><p>2 x Vodka<br>1 x Orange Juice</p></a></li><li data-name=\"1-G|5-O\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ScrewDriver.png\"><h2>Gin 'n' Juice</h2><p>1 x Gin<br>1 x Orange Juice</p></a></li><li data-name=\"2-G|5-O\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ScrewDriver.png\"><h2><b>Large</b> Gin 'n' Juice</h2><p>2 x Gin<br>1 x Orange Juice</p></a></li><li data-name=\"1-G|1-D|4-O\"><a href=\"#\"><img src=\"http://192.168.1.1/d/BronxCocktail.png\"><h2>Bronx Cocktail</h2><p>1 x Gin<br>1 x Dry Vermouth<br>1 x Orange Juice</p></a></li><li data-name=\"1-V|1-R|5-O\"><a href=\"#\"><img src=\"http://192.168.1.1/d/BrassMonkey.png\"><h2>Brass Monkey</h2><p>1 x Vodka<br>1 x Rum<br>1 x Orange Juice</p></a></li><li data-role=\"list-divider\">Short Drinks</li><li data-name=\"2-B|1-D|1-A\"><a href=\"#\"><img src=\"http://192.168.1.1/d/Manhatten.png\"><h2>Manhattan</h2><p>2 x Bourbon<br>1 x Dry Vermouth<br>1 x Angostura Bitters</p></a></li><li data-name=\"1-C|1-G|1-D\"><a href=\"#\"><img src=\"http://192.168.1.1/d/Negroni.png\"><h2>Negroni</h2><p>1 x Gin<br>1 x Dry Vermouth<br>1 x Campari</p></a></li><li data-name=\"1-G|1-D\"><a href=\"#\"><img src=\"http://192.168.1.1/d/DryMartini.png\"><h2>Dry Martini</h2><p>1 x Gin<br>1 x Dry Vermouth</p></a></li><li data-name=\"1-V|1-D\"><a href=\"#\"><img src=\"http://192.168.1.1/d/DryMartini.png\"><h2>Vodka Martini</h2><p>1 x Vodka<br>1 x Dry Vermouth</p></a></li><li data-name=\"1-C|1-B|1-D\"><a href=\"#\"><img src=\"http://192.168.1.1/d/Negroni.png\"><h2>Boulevardier</h2><p>1 x Bourbon<br>1 x Dry Vermouth<br>1 x Campari</p></a></li><li data-name=\"1-G|1-V|1-D\"><a href=\"#\"><img src=\"http://192.168.1.1/d/Vesper.png\"><h2>Vesper</h2><p>1 x Vodka<br>1 x Gin<br>1 x Dry Vermouth</p></a></li><li data-name=\"2-G|1-V|1-D\"><a href=\"#\"><img src=\"http://192.168.1.1/d/Vesper.png\"><h2><b>Super</b> Vesper</h2><p>1 x Vodka<br>2 x Gin<br>1 x Dry Vermouth</p></a></li><li data-name=\"1-G|1-A\"><a href=\"#\"><img src=\"http://192.168.1.1/d/pinkgin.png\"><h2>Pink Gin</h2><p>1 x Gin<br>1 x Angostura Bitters<br></p></a></li><li data-name=\"2-G|1-A\"><a href=\"#\"><img src=\"http://192.168.1.1/d/pinkgin.png\"><h2>Large Pink Gin</h2><p>2 x Gin<br>1 x Angostura Bitters<br></p></a></li><li data-role=\"list-divider\">Shots</li><li data-name=\"1-V\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ClearShot.png\"><h2>Shot of Vodka</h2><p>1 x Vodka</p></a></li><li data-name=\"1-G\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ClearShot.png\"><h2>Shot of Gin</h2><p>1 x Gin</p></a></li><li data-name=\"1-R\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ClearShot.png\"><h2>Shot of Rum</h2><p>1 x Rum</p></a></li><li data-name=\"1-B\"><a href=\"#\"><img src=\"http://192.168.1.1/d/BrownShot.png\"><h2>Shot of Bourbon</h2><p>1 x Bourbon</p></a></li><li data-name=\"2-V\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ClearShot.png\"><h2>Double Shot of Vodka</h2><p>2 x Vodka</p></a></li><li data-name=\"2-G\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ClearShot.png\"><h2>Double Shot of Gin</h2><p>2 x Gin</p></a></li><li data-name=\"2-R\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ClearShot.png\"><h2>Double Shot of Rum</h2><p>2 x Rum</p></a></li><li data-name=\"2-B\"><a href=\"#\"><img src=\"http://192.168.1.1/d/BrownShot.png\"><h2>Double Shot of Bourbon</h2><p>2 x Bourbon</p></a></li><li data-role=\"list-divider\">Soft Drinks</li><li data-name=\"5-O\"><a href=\"#\"><img src=\"http://192.168.1.1/d/ScrewDriver.png\"><h2>Orange</h2><p>1 x Orange Juice</p></a></li></ul></div><div data-role=\"footer\" data-position=\"fixed\"> <div data-role=\"navbar\" data-iconpos=\"bottom\"> <ul> <li><a href=\"#menu\" data-icon=\"grid\" data-role=\"tab\" class=\"ui-btn-active ui-state-persist\">Menu</a></li><li><a href=\"#mixit\" data-icon=\"gear\" data-role=\"tab\">Mix</a></li></ul> </div></div></div><div data-role=\"page\" id=\"mixit\"><div data-role=\"header\" data-position=\"fixed\"><h1>drinkOtron</h1><a href=\"#send2\" id=\"send2\" data-icon=\"check\" class=\"ui-btn-right\">Make</a></div><!--O=Orange JuiceG=GinV=VodkaB=BourbonA=Angastora BittersC=Cranberry JuiceR=RumD=Dry Vermouth--><div data-role=\"content\" class=\"ui-content\"><form><label for=\"slider-10\">Bourbon(shots)</label><input type=\"range\" name=\"slider-10\" id=\"B\" min=\"0\" max=\"4\" step=\"1\" value=\"0\"><label for=\"slider-11\">Gin(shots)</label><input type=\"range\" name=\"slider-11\" id=\"G\" min=\"0\" max=\"4\" step=\"1\" value=\"0\"><label for=\"slider-12\">Vodka(shots)</label><input type=\"range\" name=\"slider-12\" id=\"V\" min=\"0\" max=\"4\" step=\"1\" value=\"0\"><label for=\"slider-13\">Rum(shots)</label><input type=\"range\" name=\"slider-13\" id=\"R\" min=\"0\" max=\"4\" step=\"1\" value=\"0\"><label for=\"slider-14\">Dry Vermouth(shots)</label><input type=\"range\" name=\"slider-14\" id=\"D\" min=\"0\" max=\"4\" step=\"1\" value=\"0\"><label for=\"slider-17\">Campari(shots)</label><input type=\"range\" name=\"slider-17\" id=\"C\" min=\"0\" max=\"4\" step=\"1\" value=\"0\"><label for=\"slider-15\">Angastora Bitters(dashes)</label><input type=\"range\" name=\"slider-15\" id=\"A\" min=\"0\" max=\"4\" step=\"1\" value=\"0\"><label for=\"slider-16\">Orange Juice(measures) - 10 is &half; pint</label><input type=\"range\" name=\"slider-16\" id=\"O\" min=\"0\" max=\"10\" step=\"1\" value=\"0\"></form></div><div data-role=\"footer\" data-position=\"fixed\"> <div data-role=\"navbar\" data-iconpos=\"bottom\"> <ul> <li><a href=\"#menu\" data-icon=\"grid\" data-role=\"tab\">Menu</a></li><li><a href=\"#mixit\" data-icon=\"gear\" data-role=\"tab\" class=\"ui-btn-active ui-state-persist\">Mix</a></li></ul> </div></div></div><!--O=Orange JuiceG=GinV=VodkaB=BourbonA=Angastora BittersC=CampariR=RumD=Dry Vermouth--><script language=\"Javascript\">$(document).ready(function(){var selectedMix='';var drinkMappings={'O':0,'V':1,'G':2,'B':3,'R':4,'C':5,'A':6,'D':7};var shotMappings={'O':20,'V':10,'G':10,'B':10,'R':10,'C':10,'A':2,'D':10};$(\"a[data-role=tab]\").each(function (){var anchor=$(this); anchor.bind(\"click\", function (){$.mobile.changePage(anchor.attr(\"href\"),{transition: \"none\", changeHash: false}); return false;});});$('#cocktailList').children('li').bind('touchstart mousedown', function(e){$('#send').removeClass('ui-state-disabled'); selectedMix=$(this).attr('data-name');});$('#send').on('click', function (e){var translatedMix='';selectedMix.split('|').forEach(function(mix){translatedMix=translatedMix + drinkMappings[mix.split('-')[1]] + '-' + shotMappings[mix.split('-')[1]] * mix.split('-')[0] + '|';});$.get('/makeDrink?mix=' + translatedMix, function(){});}); $('#send2').on('click', function (e){var translatedMix='';$('input[data-type=range]').each(function(element){if ($(this)[0].value !=0){translatedMix=translatedMix + drinkMappings[$(this)[0].id] + '-' + shotMappings[$(this)[0].id] * $(this)[0].value + '|';}});$.get('/makeDrink?mix=' + translatedMix, function(){});});});</script></body></html>";

void writeStringToEEPROM(int addrOffset, const String &strToWrite) {
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++) {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset) {
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++) {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0';
  return String(data);
}

void handleRoot() {
  Serial.println("Handle root");
  server.send_P(200, "text/html", homePage);
  if (server.hasArg("pos")) {
 
  }  
}

void test() {
  if (server.hasArg("motor")) {
      int motor = server.arg("motor").toInt();  
      mcp.digitalWrite(motor, HIGH);
      delay(5000);
      mcp.digitalWrite(motor, LOW);
  }
}

void sendWeight() {
  server.send(200, "text/html", String(scale.get_units(10)));
}

void calibrate() {
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);
  digitalWrite(D7, HIGH);

  while (digitalRead(10) == 1) { delay(100); }
  digitalWrite(D7, LOW);

  scale.tare();
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));
  delay(500);

  while (digitalRead(10) == 1) { delay(100); }
  digitalWrite(D6, LOW);

  scale.callibrate_scale(500, 5);

  scaleFactor = scale.get_scale();
  server.send(200, "text/html", "OK");
}

void setLevels() {
  if (server.hasArg("levels")) {
    writeStringToEEPROM(0, server.arg("levels"));
  }
  server.send(200, "text/html", "OK");
}

void getLevels() {
  server.send(200, "text/html", readStringFromEEPROM(0));
}

void makeDrink() {
  float currentWeight = scale.get_units(10);

  if (server.hasArg("mix")) {

      digitalWrite(D5, LOW);
      digitalWrite(D6, HIGH);

      while (scale.get_units(2) < 50) { delay(10); }
      delay(1000);

      StringSplitter *drinks = new StringSplitter(server.arg("mix"), '|',8);

      for (int i=0; i < drinks->getItemCount()-1; i++)  {
        StringSplitter *currentDrink = new StringSplitter(drinks->getItemAtIndex(i), '-', 8);
      
        float currentWeight = scale.get_units(10);
      
        mcp.digitalWrite(currentDrink->getItemAtIndex(0).toInt(), HIGH);
  
        int counterFlash=0;
  
        while (scale.get_units(2) < currentWeight + currentDrink->getItemAtIndex(1).toInt()) { 
          delay(10); 
          counterFlash++;
          if (counterFlash == 3) {
            digitalWrite(D6, LOW);
          } else if (counterFlash == 6) {
            digitalWrite(D6, HIGH);
            counterFlash=0;
          }
        }
        mcp.digitalWrite(currentDrink->getItemAtIndex(0).toInt(), LOW);
        digitalWrite(D6, HIGH);
        delay(10000);
      }
      
      delay(1000);

      digitalWrite(D6, LOW);
      digitalWrite(D7, HIGH);      

      while (scale.get_units(2) > 50) { delay(10); }
      delay(1000);
      digitalWrite(D7, LOW);
      digitalWrite(D5, HIGH);      
      
  }

  server.send(200, "text/html", "POURED");
}

void setup() {
  WiFi.hostname("drinkotron");
  WiFi.mode(WIFI_STA);
  WiFi.begin ( ssid, password );

  if (MDNS.begin("drinkotron")) {
    Serial.println("MDNS responder started");
  }

  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 10 );
  }

  pinMode(10, INPUT_PULLUP);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);

  ArduinoOTA.begin();
  ArduinoOTA.onStart([]() {  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {  });
  ArduinoOTA.onEnd([]() { });
 
  Serial.begin(115200);

  mcp.begin();      // use default address 0
   
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  mcp.pinMode(4, OUTPUT);
  mcp.pinMode(5, OUTPUT);
  mcp.pinMode(6, OUTPUT);
  mcp.pinMode(7, OUTPUT);

  server.on("/", handleRoot);
  server.on("/getWeight", sendWeight);
  server.on("/calibrate", calibrate);
  server.on("/makeDrink", makeDrink);
  server.on("/test", test);
  server.on("/getLevels", getLevels);
  server.on("/setLevels", setLevels);
  
  
  server.begin();

  scale.begin(dataPin, clockPin);
  scale.tare();
  scale.set_scale(scaleFactor);

  digitalWrite(D5, HIGH);
  

}

void loop()
{
  ArduinoOTA.handle();
  server.handleClient();
}

// END OF FILE
