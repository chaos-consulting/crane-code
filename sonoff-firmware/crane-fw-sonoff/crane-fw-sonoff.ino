#include <ESP8266WiFi.h>
#include <Espanol.h>

char *ssid     = "tollessid";
char *password = "tollespasswort";
char *broker   = "192.168.1.100";
char *myhostname = "crane-sonoff-01";
int   port       = 1883;

#define RELAY_PIN  12
#define LED_PIN 13

int relay = LOW;
String macaddress;

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}


void setup()
{
    uint8_t mac[6];
    Serial.begin(115200);
    Serial.flush();

    Serial.println("Crane sonoff v0.0.1");
    
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, relay);
    
    // Blink... we are alive...
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);

    WiFi.macAddress(mac);
    macaddress = macToStr(mac);
    
    Espanol.begin(ssid, password, myhostname, broker, port);

    //Espanol.setDebug(true);
    Espanol.subscribe("crane/sonoff/" + macaddress);
    
    Espanol.setCallback([](char *topic, byte *payload, unsigned int length) {
        payload[length] = '\0';
        String command = (char*) payload;
        String mytopic = topic;
                
        if (mytopic == "crane/sonoff/"+ macaddress) {
            if ( command == "on") {
              relay = HIGH;
            } else if (command == "off") {
              relay = LOW;
            }
            digitalWrite(RELAY_PIN, relay);
            digitalWrite(LED_PIN, relay == HIGH ? LOW : HIGH);
        }
    });
}

void loop()
{
    Espanol.loop();
}
