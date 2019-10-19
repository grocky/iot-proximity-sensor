#include "ProjectConfiguration.h"

ProjectConfiguration::ProjectConfiguration(int baudRate, CallbackFn loggingFunction)
: bleeper(BleeperClass::shared()), baudRate(baudRate), loggingFunction(loggingFunction) {
    this->setup();
}

// private
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
