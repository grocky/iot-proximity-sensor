#include "ProjectConfiguration.h"

ProjectConfiguration::ProjectConfiguration(int baudRate)
: baudRate(baudRate) {
    this->loggingFunction = [](const ConfigurationPropertyChange value) {
        Serial.println("Configuration " + value.key + " changed from " + value.oldValue + " to " + value.newValue);
    };
}

ProjectConfiguration::ProjectConfiguration(int baudRate, CallbackFn loggingFunction)
: baudRate(baudRate), loggingFunction(loggingFunction) { }

void ProjectConfiguration::setup() {
    Bleeper
        .verbose(this->baudRate)
        .configuration
            .set(this)
            .addObserver(new SettingsCallbackObserver(this->loggingFunction), {
                    &this->sonar.triggerDistanceInches,
                    &this->sonar.triggerIntervals,
                    &this->sonar.intervalDelayMilliseconds,
                    &this->sonar.triggerTimeoutSeconds
            })
            .done()
        .configurationInterface
            .addDefaultWebServer()
            .done()
        .storage
            .set(new SPIFFSStorage()) // SPIFFS
            .done()
        .init();
}
