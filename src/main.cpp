#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <secrets.hpp>

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

Servo myservo;  // create servo object to control a servo

const int servoReadPin = A0;
const int servoWritePin = D4;
const int servoMinUs = 400;
const int servoMaxUs = 2500;
const bool invert = true;

unsigned int servoValue0Deg, servoValue180Deg;  // Vaiables to store min and max values of servo's pot
int pos = 0;                                    // variable to store the servo position

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

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

long get_servo_state() {
    long value = map(analogRead(servoReadPin), servoValue0Deg, servoValue180Deg, 0, 100);
    if (invert) {
        return 100 - value;
    } else {
        return value;
    }
}

void write_servo(int pos) {
    attachServo();

    if (invert) {
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

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("outTopic", "hello world");
            // ... and resubscribe
            client.subscribe("inTopic");
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
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    attachServo();
    Serial.begin(9600);
    calibration();
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 2000) {
        lastMsg = now;
        client.publish("outTopic", String(get_servo_state()).c_str());
    }
}
