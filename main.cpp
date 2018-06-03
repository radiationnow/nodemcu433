/*
  Modified from rc-switch simple receiving example (https://github.com/sui77/rc-switch/)
  
  Sends a simple 'received' message in a basetopic, with the subtopic set by the message.
  Does not parse protocol; if a device sends different codes for different states, this will use different subtopics.
  
  Requires RC-Switch and PubSubClient libraries.  Tested on a NodeMCU v2.
*/

#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>

const char* ssid = "SSID"; // your SSID
const char* password = "PWD"; // your wifi password
const char* mqtt_server = "192.168.2.2"; // your MQTT server IP
const char* mqtt_basetopic = "mqtt433/"; // the MQTT topic that messages should be based in
long lastMsg = 0;
void reconnect(void);

RCSwitch mySwitch = RCSwitch();
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(D1);  // Receiver on pin D1 on a NodeMCU)
  WiFi.mode(WIFI_STA);
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  client.setServer(mqtt_server, 1883);
  reconnect();
  Serial.print("init complete");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (mySwitch.available()) {
    char buffer [sizeof(int)*8+1] = "";
    char topic [sizeof(int)*8+12] = "";  // The 12 here is hardcoded to the appropriate size for the default base topic.
    int value = mySwitch.getReceivedValue();
    
    itoa(value, buffer, 10);
    strcat(topic,mqtt_basetopic);
    strcat(topic,buffer);

    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      Serial.print("Received ");
      Serial.print( value );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
    }
    if (!client.connected()) {
      reconnect();
      Serial.print("MQTT reconnected");
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 2000) {
        Serial.print("Publish message: ");
        Serial.println(buffer);
        Serial.print("Publish topic: ");
        Serial.println(topic);                

        client.publish(topic, "received");
        lastMsg = now;
    }
    mySwitch.resetAvailable();
  }
}
