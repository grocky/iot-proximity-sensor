#ifndef IOT_PROXIMITY_SENSOR_SETTINGSCALLBACKOBSERVER_H
#define IOT_PROXIMITY_SENSOR_SETTINGSCALLBACKOBSERVER_H

#include <Bleeper.h>

typedef std::function<void(const ConfigurationPropertyChange)> CallbackFn;

class SettingsCallbackObserver : public ConfigurationObserver {
public:
    SettingsCallbackObserver(CallbackFn callback);

    void onConfigurationChanged(const ConfigurationPropertyChange value);
private:
    CallbackFn callback;
};

#endif //IOT_PROXIMITY_SENSOR_SETTINGSCALLBACKOBSERVER_H
