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
int led = HIGH;

void setup()
{
    Serial.begin(115200);
    Serial.flush();
    Serial.println("Crane sonoff v0.0.1");
    Serial.println("Subscribing to foo/bar/#");
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, relay);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, led);
    
    Espanol.begin(ssid, password, myhostname, broker, port);

    //Serial.println("Setup... 2");

    //Espanol.setDebug(true);
    Espanol.subscribe("foo/bar/#");
    
    //Serial.println("Setup... 3");

    Espanol.setCallback([](char *topic, byte *payload, unsigned int length) {
        payload[length] = '\0';
        String command = (char*) payload;
        String mytopic = topic;
        
        String msg = mytopic;
        msg += " - ";
        msg += command;
        Serial.println(msg);
        
        if (mytopic == "foo/bar/relay") {
            //Serial.println("topic entered...");
            if ( command == "on") {
              //Serial.println("on");
              relay = HIGH;
              led = LOW; 
            } else if (command == "off") {
              //Serial.println("off");
              relay = LOW;
              led = HIGH;
            }
            digitalWrite(RELAY_PIN, relay);
            digitalWrite(LED_PIN, led);
        }
    });
    
    //Serial.println("Setup... 4");

}

void loop()
{
    Espanol.loop();
}
