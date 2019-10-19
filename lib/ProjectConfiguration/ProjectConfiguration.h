
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

class ServoConfig: public Configuration {
public:
    persistentIntVar(intervalDelaySeconds, 10);
};

class LogConfig: public Configuration {
public:
    intVar(logLevel,4)
};

/**
 * sonar:
 *      triggerDistanceInches: 6
 *      intervalDelayMilliseconds: 100
 *      triggerIntervals: 5
 *      triggerTimeoutSeconds: 60
 * servo:
 *      intervalDelaySeconds: 5
 * log:
 *      logLevel: 4
 */
class ProjectConfiguration: public RootConfiguration {
public:
    ProjectConfiguration(int baudRate): ProjectConfiguration(baudRate, [](const ConfigurationPropertyChange value) {
        Serial.printf("Configuration %s changed from %s to %s\r\n", value.key.c_str(), value.oldValue.c_str(), value.newValue.c_str());
    }) {};

    ProjectConfiguration(int baudRate, CallbackFn loggingFunction);

    subconfig(SonarConfig, sonar);
    subconfig(ServoConfig, servo);
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
