#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "PCF8574.h"
// Managers
#include <ActionManager.h>
// Sensors
#include <UltrasonicPCF8574.h>
#include <FlowMeter.h>
// Actuators
#include <Electrovalvula.h>

// WiFi and MQTT config
const char *ssid = "............";
const char *password = "............";
const char *mqtt_server = ".........";
const char *mqtt_username = "............";
const char *mqtt_password = ".............";

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// PCF8574 initialization
PCF8574 pcf20(0x20, D2, D1);

// Tests the PCF by blinking a LED
// every second.
int old_time_test_pcf = 0;
bool state_test_pcf = true;
const uint8_t pin_test_pcf = 3;

// Creates a json message for publishing to a solenoid-valve mqtt topic.
String createJsonMessage(uint8_t identification, char *description, char *key, bool val)
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["identification"] = identification;
    root["description"] = description;
    root[key] = val;
    String message;
    root.printTo(message);
    return message;
}

const int VALVES_SIZE = 5;

// Array of Electrovalvula objects.
Electrovalvula valves[VALVES_SIZE] = {
    {0, pcf20}, {1, pcf20}, {2, pcf20}, {3, pcf20}, {4, pcf20}};

// Array of callbacks.
void (*valves_callbacks[VALVES_SIZE])(uint8_t state) = {
    [](uint8_t state) {
        String message = createJsonMessage(0, "electrovalvula 1", "open", (state ? true : false));
        mqtt_client.publish("irrigation-system/actuator/solenoid-valve/get", message.c_str());
    },
    [](uint8_t state) {
        String message = createJsonMessage(1, "electrovalvula 2", "open", (state ? true : false));
        mqtt_client.publish("irrigation-system/actuator/solenoid-valve/get", message.c_str());
    },
    [](uint8_t state) {
        String message = createJsonMessage(2, "electrovalvula 3", "open", (state ? true : false));
        mqtt_client.publish("irrigation-system/actuator/solenoid-valve/get", message.c_str());
    },
    [](uint8_t state) {
        String message = createJsonMessage(3, "electrovalvula 4", "open", (state ? true : false));
        mqtt_client.publish("irrigation-system/actuator/solenoid-valve/get", message.c_str());
    },
    [](uint8_t state) {
        String message = createJsonMessage(4, "electrovalvula 5", "open", (state ? true : false));
        mqtt_client.publish("irrigation-system/actuator/solenoid-valve/get", message.c_str());
    }};

// Array of SolenoidValve.
SolenoidValve sol_valves[VALVES_SIZE] = {
    {valves[0], *valves_callbacks[0]},
    {valves[1], *valves_callbacks[1]},
    {valves[2], *valves_callbacks[2]},
    {valves[3], *valves_callbacks[3]},
    {valves[4], *valves_callbacks[4]}
};

// ActionManager object that manages 5 SolenoidValve
// variables.
ActionManager actuators(sol_valves, VALVES_SIZE);

void setup()
{
    Serial.begin(9600);
    pcf20.begin();

    delay(10);
    Serial.println();
    Serial.print("Connecting to");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    mqtt_client.setServer(mqtt_server, 19328);
}

void loop()
{
    if (!mqtt_client.connected())
    {
        // Loop until we're reconnected
        while (!mqtt_client.connected())
        {
            Serial.print("Attempting MQTT connection...");
            // Create a random client ID
            String clientId = "ESP8266Client";
            // Attempt to connect
            if (mqtt_client.connect(clientId.c_str(), mqtt_username, mqtt_password))
            {
                Serial.println("connected");
                // Once connected, publish an announcement...
                mqtt_client.publish("outTopic", "hello world");
                // ... and resubscribe
                mqtt_client.subscribe("inTopic");
                mqtt_client.subscribe("irrigation-system/actuator/solenoid-valve/set");
                // Callback for managing the receiving mqtt messages.
                mqtt_client.setCallback([](char *topic, uint8_t *payload, unsigned int length) {
                    String solenoid_topic = "irrigation-system/actuator/solenoid-valve/set";
                    Serial.println("Mensaje recibido para topic: ");
                    Serial.println(topic);
                    // Evaluates if the topic for the message received
                    // is from solenoid-valve/set
                    if (solenoid_topic.equals(topic))
                    {
                        actuators.handleValveMessage((char *)payload);
                    }
                    else
                    {
                        Serial.print("Topic: ");
                        Serial.print(topic);
                        Serial.println(" not contemplated.");
                    }
                });
            }
            else
            {
                Serial.print("failed, rc=");
                Serial.print(mqtt_client.state());
                Serial.println(" try again in 5 seconds");
                // Wait 5 seconds before retrying
                delay(5000);
            }
        }
    }
    mqtt_client.loop();
}