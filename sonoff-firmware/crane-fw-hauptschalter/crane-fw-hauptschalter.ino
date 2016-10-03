#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Espanol.h>

char * ssid     = "SkyNet";
char * password = "DbmiWsMD6Ko4Pc";

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
   setup things...
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
  Serial.flush();
  Serial.println("Crane mainswitch v0.0.1");

#ifdef SERIAL_VERBOSE
  Serial.print("Hostname: ");
  Serial.println(hostString);
#endif

  WiFi.hostname(hostString);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);

#ifdef SERIAL_VERBOSE
    Serial.print(".");
#endif

  }

#ifdef SERIAL_VERBOSE
  Serial.println();
#endif

  WiFi.macAddress(mac);
  macaddress = macToStr(mac);

#ifdef SERIAL_VERBOSE
  Serial.print("MAC-Adress: ");
  Serial.println(macaddress);
  Serial.println("Sending mDNS query");
#endif

  if (!MDNS.begin(hostString)) {
    Serial.println("Error setting up MDNS responder!");
  }

#ifdef SERIAL_VERBOSE
  Serial.println("mDNS responder started");
#endif

  n = MDNS.queryService("mqtt", "tcp");

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
  Espanol.begin((String)ssid, (String)password, (String)hostString, MDNS.hostname(0), (int)MDNS.port(0));

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

#ifdef SERIAL_VERBOSE
    Serial.print(".");
#endif

    Espanol.loop();
  }

  Espanol.loop();

  tmp_position = digitalRead(MAIN_SWITCH_PIN);

  // on startup send info to mqtt broker
  if (main_switch_position == -1) {
    Espanol.publish("crane/mainswitch/info", "started");
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
