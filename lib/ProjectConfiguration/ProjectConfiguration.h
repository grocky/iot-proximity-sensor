
#ifndef IOT_PROXIMITY_SENSOR_PROJECTCONFIGURATION_H
#define IOT_PROXIMITY_SENSOR_PROJECTCONFIGURATION_H

#include "SettingsCallbackObserver.h"

class SonarConfig: public Configuration {
public:
    persistentIntVar(triggerDistanceInches, 6);
    persistentIntVar(intervalDelayMilliseconds, 100);
    persistentIntVar(triggerIntervals, 5);
    persistentIntVar(triggerTimeoutSeconds, 60);
};

/**
 * sonar:
 *      triggerDistanceInches: 6
 *      intervalDelayMilliseconds: 100
 *      triggerIntervals: 5
 *      triggerTimeoutSeconds: 60
 */
class ProjectConfiguration: public RootConfiguration {
public:
    ProjectConfiguration(int baudRate): ProjectConfiguration(baudRate, [](const ConfigurationPropertyChange value) {
        Serial.println("Configuration " + value.key + " changed from " + value.oldValue + " to " + value.newValue);
    }) {};
    ProjectConfiguration(int baudRate, CallbackFn loggingFunction);

    subconfig(SonarConfig, sonar);

    void handle() {
        this->bleeper->handle();
    }
private:
    void setup();
    BleeperClass* bleeper;
    CallbackFn loggingFunction;
    int baudRate;
};

#endif //IOT_PROXIMITY_SENSOR_PROJECTCONFIGURATION_H
