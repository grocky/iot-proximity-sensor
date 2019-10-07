//
// Created by Rocky Gray on 10/7/19.
//
#include <Config.h>

SettingsCallbackObserver::SettingsCallbackObserver(CallbackFn callback)
    : callback(callback) {
}

void SettingsCallbackObserver::onConfigurationChanged(const ConfigurationPropertyChange value) {
    if (this->callback) {
        this->callback(value);
    }
}
