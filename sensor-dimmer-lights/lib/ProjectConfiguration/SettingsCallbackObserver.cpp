#include "SettingsCallbackObserver.h"

SettingsCallbackObserver::SettingsCallbackObserver(CallbackFn callback)
        : callback(callback) { }

void SettingsCallbackObserver::onConfigurationChanged(const ConfigurationPropertyChange value) {
    if (this->callback) {
        this->callback(value);
    }
}
