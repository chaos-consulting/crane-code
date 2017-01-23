#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Espanol.h>

/* Get WLAN information from external file (excluded via .gitignore):
 *
 * char const * ssid     = "MY_SSID";
 * char const * password = "MY_SECRET_WLAN_PASSWORD";
 *
 */
#include "wlan_credentials.h"

#define SERIAL_VERBOSE 1

char hostString[16] = {0};

#define MAIN_SWITCH_PIN  2

String macaddress;
int main_switch_position;
int tmp_position;

/*
   Convert 6 byte integer array to a string of colon separated hex values
*/
String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; i++) {
    // leading zero for one digit values
    if (mac[i] <= 0xf) {
      result += "0";
    }
    result += String(mac[i], 16);
    // separate values with colons
    if (i < 5) {
      result += ':';
    }
  }
  return result;
}

/*
 * setup things...
 */
void setup()
{
  uint8_t mac[6];
  int n;
  
  main_switch_position = -1;

  pinMode(MAIN_SWITCH_PIN, INPUT);
  pinMode(0, OUTPUT);
  digitalWrite(0, LOW);

  sprintf(hostString, "ESP_%06X", ESP.getChipId());

  Serial.begin(74880);
  delay(100);

  WiFi.macAddress(mac);
  macaddress = macToStr(mac);


#ifdef SERIAL_VERBOSE
  Serial.flush();
  Serial.println("Crane mainswitch v0.0.1");

  Serial.print("Hostname: ");
  Serial.println(hostString);

  Serial.print("MAC-Adress: ");
  Serial.println(macaddress);
#endif

  WiFi.hostname(hostString);
  WiFi.begin(ssid, password);
#ifdef SERIAL_VERBOSE
  Serial.print("Connecting to WLAN ");
#endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
#ifdef SERIAL_VERBOSE
    Serial.print(".");
#endif
  }
  Serial.println();

#ifdef SERIAL_VERBOSE
  Serial.println("Sending mDNS query");
#endif

  if (!MDNS.begin(hostString)) {
    Serial.println("Error setting up MDNS responder!");
  }

#ifdef SERIAL_VERBOSE
  Serial.println("mDNS responder started");
#endif

  n = MDNS.queryService("mqtts", "tcp");

#ifdef SERIAL_VERBOSE
  Serial.println("mDNS query done");
#endif

  if (n == 0) {
    Serial.println("no services found");
  }

#ifdef SERIAL_VERBOSE
  else {
    Serial.print(n);
    Serial.println(" service(s) found");
    for (int i = 0; i < n; i++) {
      Serial.print(i);
      Serial.print(": ");
      Serial.print(MDNS.hostname(i));
      Serial.print(" (");
      Serial.print(MDNS.IP(i));
      Serial.print(":");
      Serial.print(MDNS.port(i));
      Serial.println(")");
    }
  }
#endif
  // not ideal... but Espanol does not support mDNS
  WiFi.disconnect();
  delay(100);
  String ipStr = String(MDNS.IP(0)[0]) + '.' + String(MDNS.IP(0)[1]) + '.' + String(MDNS.IP(0)[2]) + '.' + String(MDNS.IP(0)[3]);
  //Espanol.begin((String)ssid, (String)password, (String)hostString, ipStr, (int)MDNS.port(0));
  
  Espanol.begin((String)ssid, (String)password, (String)hostString, ipStr, (int)MDNS.port(0), String("testeruser"), String("testeruser123"));

#ifdef SERIAL_VERBOSE
  Espanol.setDebug(true);
  Serial.println("setup ended");
#endif

}

void loop()
{
  // Wait for connections (wifi / mqtt) to be established...
  while (Espanol.connected() != true) {
    delay(100);
    Espanol.loop();
#ifdef SERIAL_VERBOSE
    if ( Espanol.connected() != true ) {Serial.print("."); }
#endif
  }

  Espanol.loop();

  tmp_position = digitalRead(MAIN_SWITCH_PIN);

  // on startup send info to mqtt broker
  if (main_switch_position == -1) {
    Espanol.publish("crane/mainswitch/info", hostString);
  }

  if (main_switch_position != tmp_position) {
    main_switch_position = tmp_position;
    if (main_switch_position == HIGH ) {
      Espanol.publish("crane/mainswitch/state", "off");

#ifdef SERIAL_VERBOSE
      Serial.println("Mainswitch changed to off");
#endif

    } else {
      Espanol.publish("crane/mainswitch/state", "on");

#ifdef SERIAL_VERBOSE
      Serial.println("Mainswitch changed to on");
#endif

    }
    // Try to debounce a little bit...
    delay(700);
  }
}

