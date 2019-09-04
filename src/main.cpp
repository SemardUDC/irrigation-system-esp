#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "PCF8574.h"
// Managers
#include "SensorManager.h"
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

// Creates a json message for publishing to a mqtt topic.
String createJsonMessage(uint8_t identification, char *description, char *key, unsigned int val)
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

const int ULTRASONICS_SIZE = 2;

// Array of callbacks for ultrasonic sensors.
void (*ultrasonic_callbacks[ULTRASONICS_SIZE])(unsigned int current_distance){
    [](unsigned int current_distance) {
        String message = createJsonMessage(1, "Ultrasonic 1", "distance", current_distance);
        mqtt_client.publish("irrigation-system/sensor/ultrasonic", message.c_str());
    },
    [](unsigned int current_distance) {
        String message = createJsonMessage(2, "Ultrasonic 2", "distance", current_distance);
        mqtt_client.publish("irrigation-system/sensor/ultrasonic", message.c_str());
    }};

// Array of ultrasonic sensors with PCF8574.
UltrasonicPCF8574 ultras[ULTRASONICS_SIZE] = {
    {0, 1, 600, ultrasonic_callbacks[0], pcf20},
    {2, 3, 600, ultrasonic_callbacks[1], pcf20}};

const int FLOW_METERS_SIZE = 2;

// Array of callbacks for flow meter sensors.
void (*flowm_callbacks[FLOW_METERS_SIZE])(unsigned int current_content) = {
    [](unsigned int current_content) {
        String message = createJsonMessage(1, "flow_1", "content", current_content);
        mqtt_client.publish("irrigation-system/sensor/water-flow", message.c_str());
    },
    [](unsigned int current_content) {
        String message = createJsonMessage(2, "flow_2", "content", current_content);
        mqtt_client.publish("irrigation-system/sensor/water-flow", message.c_str());
    }};

// Array of flow meter sensors.
FlowMeter flows[FLOW_METERS_SIZE] = {
    {D5, flowm_callbacks[0]},
    {D6, flowm_callbacks[1]}};

// SensorManager object that manages
// the array of FlowMeter and Ultrasonic
// sensors.
SensorManager sensors(flows, 2, ultras, 2);

void setup()
{
    Serial.begin(9600);
    pcf20.begin();
    // Performs initialization for the sensors that
    // need it.
    sensors.begin();

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
    // Invokes the handle method for each
    // one of the sensors managed by the
    // SensorManager object.
    sensors.handle();
}