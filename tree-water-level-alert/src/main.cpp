#include <Arduino.h>

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#include <ProjectConfiguration.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

ProjectConfiguration* config;

WiFiClient wiFiClient;
PubSubClient mqttClient(wiFiClient);

static const int BAUD_RATE = 9600;
static const uint8  PIN_SENSOR_1 = D5;
const static char* MQTT_TREE_TOPIC= "home/upstairs/christmas-tree";
const static char* MQTT_TREE_TRIGGER_TOPIC= "home/upstairs/christmas-tree/trigger";
const static char* MQTT_TREE_LEVEL_TOPIC= "home/upstairs/christmas-tree/level";
static const int NUM_SENSORS = 1;

const int ONE_SECOND_MS = 1000;

bool mqttEnabled = false;

struct WaterLevelSensor {
    int pin;
    int depth;
    bool currentState;
} sensor1 = { PIN_SENSOR_1, 100, false };

struct ApplicationState {
    unsigned long lastPublishTime = 0;
    WaterLevelSensor* levels[NUM_SENSORS];
} currentState;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    std::string topicString(topic);

    if (topicString == MQTT_TREE_TRIGGER_TOPIC) {
    }
}

bool mqttReconnect() {
    Serial.print("Attempting MQTT connection... ");

    mqttClient.setServer(config->mqtt.serverHost.c_str(), config->mqtt.serverPort);
    mqttClient.setCallback(mqttCallback);

    // Create a random client ID
    String clientId = "christmas-tree-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (!mqttClient.connect(clientId.c_str())) {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
        return false;
    }

    Serial.printf("mqtt client connected: %s\n", clientId.c_str());
    // Once connected, publish an announcement...
    mqttClient.publish(MQTT_TREE_TOPIC, "1");
    // ... and resubscribe
    mqttClient.subscribe(MQTT_TREE_TRIGGER_TOPIC);
    Serial.printf("Subscribed to %s\n", MQTT_TREE_LEVEL_TOPIC);

    return true;
}

void setupWifi() {
    Serial.println("Opening configuration portal");

    digitalWrite(LED_BUILTIN, LOW);

    WiFiManager wifiManager;

    if (!wifiManager.autoConnect()) {
        Serial.println("failed to connect and hit timeout");
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(1000);
    }

    Serial.println("WiFi connected: " + WiFi.localIP().toString());

    if (MDNS.begin("christmas-tree")) {
        MDNS.addService("http", "tcp", 80);
        Serial.println("mDNS name: christmas-tree.local");
    } else {
        Serial.println("Error setting up mDNS responder.");
    }

    Serial.println("WiFi connection completed");

    digitalWrite(LED_BUILTIN, HIGH);

    int connResult = WiFi.waitForConnectResult();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("failed to connect, finishing setup anyway");
    } else {
        Serial.printf("local ip: %s\r\n", WiFi.localIP().toString().c_str());
    }
}

void setupConfiguration() {
    config = new ProjectConfiguration(BAUD_RATE);
}

void setup() {
    pinMode(PIN_SENSOR_1, INPUT_PULLUP);
    Serial.begin(BAUD_RATE);

    delay(1000);
    Serial.println();
    Serial.println("Device starting.");

    setupWifi();
    setupConfiguration();

    currentState.levels[0] = &sensor1;
}

bool isWaterHigh(int sensorPin) {
    return digitalRead(sensorPin) == 1;
}

bool shouldPublishLevels(unsigned long lastPublishTime) {
    return millis() - lastPublishTime > config->mqtt.publishIntervalInSeconds * ONE_SECOND_MS;
}

long lastMqttConnectionAttempt = 0;

void loop() {
    MDNS.update();
    config->handle();

    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastMqttConnectionAttempt > 5 * ONE_SECOND_MS) {
            mqttEnabled = config->mqtt.serverHost != "";
            if (!mqttEnabled || !mqttReconnect()) {
                return;
            } else {
                lastMqttConnectionAttempt = 0;
            }
        }
    }

    mqttClient.loop();

    bool shouldPublish = false;

    for(WaterLevelSensor* s : currentState.levels) {
        bool currentValue = isWaterHigh(s->pin);
        if (currentValue != s->currentState) {
            Serial.printf("Sensor 1 state changed from %d to %d\n", s->currentState, currentValue);
            s->currentState = currentValue;
            shouldPublish = true;
        }
    }

    int waterLevel = 0;

    for(WaterLevelSensor* s : currentState.levels) {
        if (s->currentState) {
            waterLevel += s->depth;
        }
    }

    if (shouldPublish) {
        Serial.printf("Water level changed: %d\n", waterLevel);
        mqttClient.publish(MQTT_TREE_LEVEL_TOPIC, String(waterLevel).c_str());
    }

    if (shouldPublishLevels(currentState.lastPublishTime)) {
        currentState.lastPublishTime = millis();
        Serial.printf("%6dl - publishing water level: %d\n", currentState.lastPublishTime, waterLevel);
        mqttClient.publish(MQTT_TREE_LEVEL_TOPIC, String(waterLevel).c_str());
    }
}
