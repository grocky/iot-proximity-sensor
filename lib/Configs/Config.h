//
// Created by Rocky Gray on 10/6/19.
//

#ifndef IOT_PROXIMITY_SENSOR_CONFIG_H
#define IOT_PROXIMITY_SENSOR_CONFIG_H

#include <Arduino.h>
#include <Bleeper.h>

class SonarConfig: public Configuration {
public:
   persistentIntVar(triggerDistance, 6);
};

/**
 * sonar:
 *      triggerDistance: 6
 */
class Config: public RootConfiguration {
public:
    subconfig(SonarConfig, sonar);
};

class SettingsCallbackObserver : public ConfigurationObserver {
public:
    using CallbackFn = std::function<void(const ConfigurationPropertyChange)>;

    SettingsCallbackObserver(CallbackFn callback);
    void onConfigurationChanged(const ConfigurationPropertyChange value);

private:
    CallbackFn callback;
};

#endif //IOT_PROXIMITY_SENSOR_CONFIG_H
