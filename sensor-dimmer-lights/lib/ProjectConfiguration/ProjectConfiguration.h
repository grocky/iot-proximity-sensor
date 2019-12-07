
#ifndef IOT_PROXIMITY_SENSOR_PROJECTCONFIGURATION_H
#define IOT_PROXIMITY_SENSOR_PROJECTCONFIGURATION_H

#include "SettingsCallbackObserver.h"

class LightsConfig: public Configuration {
public:
    persistentIntVar(durationSeconds, 300);
};

class MQTTConfig: public Configuration {
public:
    persistentStringVar(serverHost, "");
    persistentIntVar(serverPort, 1883);
};

class LogConfig: public Configuration {
public:
    intVar(logLevel,5)
};

/**
 * lights:
 *      durationSeconds: 300
 * mqtt:
 *      serverHost: foo.com
 * log:
 *      logLevel: 4
 */
class ProjectConfiguration: public RootConfiguration {
public:
    ProjectConfiguration(int baudRate): ProjectConfiguration(baudRate, [](const ConfigurationPropertyChange value) {
        Serial.printf("Configuration %s changed from %s to %s\r\n", value.key.c_str(), value.oldValue.c_str(), value.newValue.c_str());
    }) {};

    ProjectConfiguration(int baudRate, CallbackFn loggingFunction);

    subconfig(LightsConfig, lights);
    subconfig(MQTTConfig, mqtt);
    subconfig(LogConfig, log);

    void handle() {
        this->bleeper->handle();
    }
private:
    void setup();
    BleeperClass* bleeper;
    int baudRate;
    CallbackFn loggingFunction;
};

#endif //IOT_PROXIMITY_SENSOR_PROJECTCONFIGURATION_H
