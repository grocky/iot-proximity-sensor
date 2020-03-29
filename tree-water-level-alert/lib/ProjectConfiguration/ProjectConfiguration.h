#ifndef IOT_PROXIMITY_SENSOR_PROJECTCONFIGURATION_H
#define IOT_PROXIMITY_SENSOR_PROJECTCONFIGURATION_H

#include "SettingsCallbackObserver.h"

class MQTTConfig: public Configuration {
public:
    persistentStringVar(serverHost, "");
    persistentIntVar(serverPort, 1883);
    persistentIntVar(publishIntervalInSeconds, 1);
};

/**
 * mqtt:
 *      serverHost: foo.com
 */
class ProjectConfiguration: public RootConfiguration {
public:
    ProjectConfiguration(int baudRate): ProjectConfiguration(baudRate, [](const ConfigurationPropertyChange value) {
        Serial.printf("Configuration %s changed from %s to %s\r\n", value.key.c_str(), value.oldValue.c_str(), value.newValue.c_str());
    }) {};

    ProjectConfiguration(int baudRate, CallbackFn loggingFunction);

    subconfig(MQTTConfig, mqtt);

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
