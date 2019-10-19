#include "ProjectConfiguration.h"

ProjectConfiguration::ProjectConfiguration(int baudRate)
: bleeper(BleeperClass::shared()), baudRate(baudRate)  {
    this->loggingFunction = [](const ConfigurationPropertyChange value) {
        Serial.println("Configuration " + value.key + " changed from " + value.oldValue + " to " + value.newValue);
    };
}

ProjectConfiguration::ProjectConfiguration(int baudRate, CallbackFn loggingFunction)
: bleeper(BleeperClass::shared()), baudRate(baudRate), loggingFunction(loggingFunction) { }

void ProjectConfiguration::setup() {
    this->bleeper
        ->verbose(this->baudRate)
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
            .set(new SPIFFSStorage())
            .done()
        .init();
}
