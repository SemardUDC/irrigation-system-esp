#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "PCF8574.h"
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

// Ultrasonic initialization
const uint8_t TRIGGER_PIN = 0;
const uint8_t ECHO_PIN = 1;
void ultr_sensor_callback(unsigned int current_distance)
{
    Serial.print("Distance: ");
    Serial.println(current_distance);
    //Creates a JSON to send to topic containing
    //ultr info.
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["identification"] = "666";
    root["description"] = "test from sketch";
    root["distance"] = current_distance;
    String message;
    root.printTo(message);
    Serial.print("JSON generated: ");
    Serial.println(message);
    mqtt_client.publish("irrigation-system/sensor/ultrasonic", message.c_str());
}
UltrasonicPCF8574 ultr(TRIGGER_PIN, ECHO_PIN, 600, ultr_sensor_callback, pcf20);

// Flow Meter initialization
const uint8_t flow_signal_pin = D5;
void flow_meter_callback(unsigned int current_content)
{
    static int oldTime = 0;
    if (millis() - oldTime > 10000)
    {
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        root["identification"] = 666;
        root["description"] = "tanque vaciado";
        root["content"] = current_content;
        oldTime = millis();
        String message;
        root.printTo(message);
        Serial.print("JSON generated: ");
        Serial.println(message);
        mqtt_client.publish("irrigation-system/sensor/water-flow", message.c_str());
    }
}

FlowMeter flowM1(flow_signal_pin, flow_meter_callback);

// Tests the PCF by blinking a LED
// every second.
int old_time_test_pcf = 0;
bool state_test_pcf = true;
const uint8_t pin_test_pcf = 3;

// Solenoid-valve initialization
void solenoid_callback(uint8_t state)
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["identification"] = 666;
    root["description"] = "electrovalvula 1";
    root["open"] = state ? true : false;
    String message;
    root.printTo(message);
    Serial.print("JSON generated: ");
    Serial.println(message);
    mqtt_client.publish("irrigation-system/actuator/solenoid-valve/get", message.c_str());
}
Electrovalvula solenoid1(3, pcf20);

void setup()
{
    Serial.begin(9600);
    pcf20.begin();
    flowM1.begin();

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
                // Callback for managing the activation/deactivation of a solenoid-valve.
                mqtt_client.setCallback([](char *topic, uint8_t *payload, unsigned int length) {
                    String solenoid_topic = "irrigation-system/actuator/solenoid-valve/set";
                    Serial.println("Mensaje recibido para topic: ");
                    Serial.println(solenoid_topic);
                    if (solenoid_topic.equals(topic))
                    {
                        StaticJsonBuffer<200> jsonBuffer;
                        JsonObject &message = jsonBuffer.parseObject(payload);
                        Serial.println("Mensaje recibido:");
                        String jsonmessage;

                        message.printTo(jsonmessage);
                        Serial.println(jsonmessage.c_str());

                        if (solenoid1.getPin() == message["identification"])
                        {
                            Serial.println("electrovalvula encontrada.");
                            if (message["state"] == "open")
                            {
                                Serial.println("Abriendo electrovalvula.");
                                solenoid1.abrir(solenoid_callback);
                            }
                            else if (message["state"] == "close")
                            {
                                Serial.println("Cerrando electrovalvula.");
                                solenoid1.cerrar(solenoid_callback);
                            }
                        }
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
    if (millis() - old_time_test_pcf > 10000)
    {
        old_time_test_pcf = millis();
        // As the ultrsonic sensor obtains the distance
        // parameter directly, is not necessary to handle
        // it every time.
        ultr.handle();
    }
    // The readings received from flowMeter are more
    // critical to capture, because it needs the pulses
    // to calculate the content passed, that's why on
    // every loop the handle is called.
    flowM1.handle();
}