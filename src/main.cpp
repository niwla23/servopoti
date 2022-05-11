#include <ArduinoJson.h>  //https://github.com/bblanchon/ArduinoJson
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>  //https://github.com/esp8266/Arduino
#include <PubSubClient.h>
#include <Servo.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager

#include "LittleFS.h"

// define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_state_topic[100] = "servopotis/1/state";
char mqtt_cmnd_topic[100] = "servopotis/1/cmnd";
char mqtt_availability_topic[100] = "servopotis/1/available";
char invert_servo[2] = "1";

const int servoReadPin = A0;
const int servoWritePin = D4;
const int servoMinUs = 400;
const int servoMaxUs = 2500;

unsigned int servoValue0Deg, servoValue180Deg;  // Vaiables to store min and max values of servo's pot
int pos = 0;                                    // variable to store the servo position

// flag for saving data
bool shouldSaveConfig = false;

// callback notifying us of the need to save config
void saveConfigCallback() {
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

Servo myservo;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

void attachServo() {
    myservo.attach(servoWritePin, servoMinUs, servoMaxUs);
}

// Calibrates the servo by setting the servo to 180 and 0 degrees respectively and reading the value of the pot
void calibration() {
    // set the servo to 180 degrees then read the value of the pot
    delay(500);
    myservo.write(180);
    delay(2000);
    servoValue180Deg = analogRead(servoReadPin);
    delay(100);

    // set the servo to 0 degrees then read the value of the pot
    myservo.write(0);
    delay(2000);
    servoValue0Deg = analogRead(servoReadPin);
    delay(1000);
    myservo.detach();
}

int get_servo_state() {
    int value = (int)map(analogRead(servoReadPin), servoValue0Deg, servoValue180Deg, 0, 100);
    if (strcmp(invert_servo, "1") == 0) {
        return 100 - value;
    } else {
        return value;
    }
}

void write_servo(int pos) {
    attachServo();

    if (strcmp(invert_servo, "1") == 0) {
        pos = 100 - pos;
    }

    long servo_value = (long)pos * 1.8;
    myservo.write(servo_value);
    delay(100);
    myservo.detach();
}

void callback(char* topic, byte* payload, unsigned int length) {
    char payload_char[length + 1];
    for (int i = 0; i < length; i++) {
        payload_char[i] = (char)payload[i];
    }
    payload_char[length] = '\0';
    write_servo(atoi(payload_char));
}

// (re)connects to MQTT server
void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "servopoti-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            client.publish(mqtt_availability_topic, "online");
            client.subscribe(mqtt_cmnd_topic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    // clean FS, for testing
    // LittleFS.format();

    // read configuration from FS json
    Serial.println("mounting FS...");

    if (LittleFS.begin()) {
        Serial.println("mounted file system");
        if (LittleFS.exists("/config.json")) {
            // file exists, reading and loading
            Serial.println("reading config file");
            File configFile = LittleFS.open("/config.json", "r");
            if (configFile) {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);

                DynamicJsonDocument json(1024);
                Serial.println(buf.get());
                auto deserializeError = deserializeJson(json, buf.get());
                serializeJson(json, Serial);
                if (!deserializeError) {
                    Serial.println("\nparsed json");
                    strcpy(mqtt_server, json["mqtt_server"]);
                    strcpy(mqtt_port, json["mqtt_port"]);
                    strcpy(mqtt_state_topic, json["mqtt_state_topic"]);
                    strcpy(mqtt_cmnd_topic, json["mqtt_cmnd_topic"]);
                    strcpy(mqtt_availability_topic, json["mqtt_availability_topic"]);
                    strcpy(invert_servo, json["invert_servo"]);

                } else {
                    Serial.println("failed to load json config");
                }
                configFile.close();
            }
        }
    } else {
        Serial.println("failed to mount FS");
    }
    // end read

    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
    WiFiManagerParameter custom_mqtt_state_topic("mqtt_state_topic", "mqtt state topic", mqtt_state_topic, 100);
    WiFiManagerParameter custom_mqtt_cmnd_topic("mqtt_cmnd_topic", "mqtt command topic", mqtt_cmnd_topic, 100);
    WiFiManagerParameter custom_mqtt_availability_topic("mqtt_availability_topic", "mqtt availability topic", mqtt_availability_topic, 100);
    WiFiManagerParameter custom_invert_servo("invert_servo", "invert servo", invert_servo, 2);

    // WiFiManager
    // Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    // set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    // set static ip
    // wifiManager.setSTAStaticIPConfig(IPAddress(10, 0, 1, 99), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));

    // add all your parameters here
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_state_topic);
    wifiManager.addParameter(&custom_mqtt_cmnd_topic);
    wifiManager.addParameter(&custom_mqtt_availability_topic);
    wifiManager.addParameter(&custom_invert_servo);

    // reset settings - for testing
    // wifiManager.resetSettings();

    if (!wifiManager.autoConnect("servopoti-config", "1234")) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        // reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(5000);
    }

    // if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    // read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_state_topic, custom_mqtt_state_topic.getValue());
    strcpy(mqtt_cmnd_topic, custom_mqtt_cmnd_topic.getValue());
    strcpy(mqtt_availability_topic, custom_mqtt_availability_topic.getValue());
    strcpy(invert_servo, custom_invert_servo.getValue());

    // save the custom parameters to FS
    if (shouldSaveConfig) {
        Serial.println("saving config");
        DynamicJsonDocument json(1024);

        json["mqtt_server"] = mqtt_server;
        json["mqtt_port"] = mqtt_port;
        json["mqtt_state_topic"] = mqtt_state_topic;
        json["mqtt_cmnd_topic"] = mqtt_cmnd_topic;
        json["mqtt_availability_topic"] = mqtt_availability_topic;
        json["invert_servo"] = invert_servo;

        File configFile = LittleFS.open("/config.json", "w");
        if (!configFile) {
            Serial.println("failed to open config file for writing");
        }
        serializeJson(json, Serial);
        serializeJson(json, configFile);
        configFile.close();
        // end save
    }

    Serial.println("local ip");
    Serial.println(WiFi.localIP());

    // Connect to MQTT and attach servo
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    attachServo();
    calibration();
    write_servo(0);
}

int lastValue = -20;
void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    int read = get_servo_state();
    if (read < lastValue - 2 || read > lastValue + 2) {
        lastMsg = now;
        lastValue = read;
        if (read < 6) {
            read = 0;
        } else if (read > 100) {
            read = 100;
        }
        client.publish(mqtt_state_topic, String(read).c_str());
    }
    delay(80);
}
