#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
// I2C components
#include "PCF8574.h"
#include "PCF8591.h"
// Managers
#include <ActionManager.h>
#include "SensorManager.h"
#include <ActionManager.h>
// Sensors
#include <UltrasonicPCF8574.h>
#include <FlowMeter.h>
#include <pH4502c.h>
#include <Press.h>
// Actuators
#include <Electrovalvula.h>
#include <PumpMotor.h>

// WiFi and MQTT config
const char *ssid = "..........";
const char *password = ".........";
const char *mqtt_server = "..........";
const char *mqtt_username = ".........";
const char *mqtt_password = "............";

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// PCF8574 initialization
PCF8574 pcfUltras(0x20, D6, D5, true);

PCF8574 pcfSolenoidValves(0x20, D2, D1, true);

PCF8574 pcfMotors(0x20, D2, D1, true);

// PCF8591 initialization
PCF8591 pcf48(0x48, D4, D3, true);


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
    Serial.println(message);
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
    {4, 5, 600, ultrasonic_callbacks[0], pcfUltras},
    {6, 7, 600, ultrasonic_callbacks[1], pcfUltras}};

const int FLOW_METERS_SIZE = 1;

// Array of callbacks for flow meter sensors.
void (*flowm_callbacks[FLOW_METERS_SIZE])(unsigned int current_content) = {
    [](unsigned int current_content) {
        String message = createJsonMessage(1, "flow_1", "content", current_content);
        mqtt_client.publish("irrigation-system/sensor/water-flow", message.c_str());
    }};

// Array of flow meter sensors.
FlowMeter flows[FLOW_METERS_SIZE] = {
    {D5, flowm_callbacks[0]}};

const int PH_METERS_SIZE = 1;

// Array of callbacks for ph-meter sensors
void (*phmeter_callbacks[PH_METERS_SIZE])(float_t ph_value) = {
    [](float_t ph_value) {
        String message = createJsonMessage(0, "Ph_1", "ph", ph_value);
        mqtt_client.publish("irrigation-system/sensor/ph-meter", message.c_str());
    }};

// Array of phmeter sensors
pH4502c phmeters[PH_METERS_SIZE] = {
    {pcf48, 0, phmeter_callbacks[0]}};

const int PRESS_SENSORS_SIZE = 2;

// Array of callbacks for pressure sensors
void (*presssensors_callbacks[PRESS_SENSORS_SIZE])(uint16_t kpa) = {
    [](uint16_t kpa) {
        String message = createJsonMessage(1, "Pressure_1", "kpa", kpa);
        mqtt_client.publish("irrigation-system/sensor/pressure", message.c_str());
    },
    [](uint16_t kpa) {
        String message = createJsonMessage(2, "Pressure_2", "kpa", kpa);
        mqtt_client.publish("irrigation-system/sensor/pressure", message.c_str());
    }};

// Array of pressure sensors
Press press_sensors[PRESS_SENSORS_SIZE] = {
    {1, pcf48, presssensors_callbacks[0]},
    {2, pcf48, presssensors_callbacks[1]}};

// SensorManager object that manages
// the array of FlowMeter and Ultrasonic
// sensors.
SensorManager sensors(flows, FLOW_METERS_SIZE, ultras, ULTRASONICS_SIZE,
                      phmeters, PH_METERS_SIZE, press_sensors, PRESS_SENSORS_SIZE);

// Creates a json message for publishing to an actuator mqtt topic.
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
    {0, pcfSolenoidValves},
    {1, pcfSolenoidValves},
    {2, pcfSolenoidValves},
    {3, pcfSolenoidValves},
    {4, pcfSolenoidValves}};

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
    [](uint8_t state){
        String message = createJsonMessage(4, "electrovalvula 5", "open", (state ? true : false));
        mqtt_client.publish("irrigation-system/actuator/solenoid-valve/get", message.c_str());
    }};

// Array of SolenoidValve.
SolenoidValve sol_valves[VALVES_SIZE] = {
    {valves[0], *valves_callbacks[0]},
    {valves[1], *valves_callbacks[1]},
    {valves[2], *valves_callbacks[2]},
    {valves[3], *valves_callbacks[3]},
    {valves[4], *valves_callbacks[4]}};

// PumpMotor

const int MOTORS_SIZE = 2;

void (*motors_callback[MOTORS_SIZE])(uint8_t state) = {
    [](uint8_t state) {
        String message = createJsonMessage(1, "Motobomba 1", "state", (state ? true : false));
        mqtt_client.publish("irrigation-system/actuator/pump-motor/get", message.c_str());
    },
    [](uint8_t state) {
        String message = createJsonMessage(2, "Motobomba 2", "state", (state ? true : false));
        mqtt_client.publish("irrigation-system/actuator/pump-motor/get", message.c_str());
    }
};

PumpMotor motors[MOTORS_SIZE] = {
    {5, motors_callback[0], pcfMotors},
    {6, motors_callback[1], pcfMotors}
};

// ActionManager object that manages 5 SolenoidValve
// variables.
ActionManager actuators(sol_valves, VALVES_SIZE, motors, MOTORS_SIZE);

void setup()
{
    Serial.begin(9600);
    pcfSolenoidValves.begin();
    pcfUltras.begin();
    pcfMotors.begin();
    pcf48.iniciar();
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
                mqtt_client.subscribe("irrigation-system/actuator/solenoid-valve/set");
                mqtt_client.subscribe("irrigation-system/actuator/pump-motor/set");
                // Callback for managing the receiving mqtt messages.
                mqtt_client.setCallback([](char *topic, uint8_t *payload, unsigned int length) {
                    String solenoid_topic = "irrigation-system/actuator/solenoid-valve/set";
                    String pump_topic = "irrigation-system/actuator/pump-motor/set";

                    Serial.println("Mensaje recibido para topic: ");
                    Serial.println(topic);
                    // Evaluates if the topic for the message received
                    // is from solenoid-valve/set
                    if (solenoid_topic.equals(topic))                
                        actuators.handleValveMessage((char *)payload);                    
                    else if (pump_topic.equals(topic)) 
                        actuators.handlePumpMotorMessage((char *) payload);                    

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
    // Invokes the handle method for each
    // one of the sensors managed by the
    // SensorManager object.
    sensors.handle();
}