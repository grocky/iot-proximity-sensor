#include "ProjectConfiguration.h"

ProjectConfiguration::ProjectConfiguration(int baudRate, CallbackFn loggingFunction)
: bleeper(BleeperClass::shared()), baudRate(baudRate), loggingFunction(loggingFunction) {
    this->setup();
}

// private
void ProjectConfiguration::setup() {
    const std::vector<StringConvertibleVariable*>& variables = this->getVariables();
    std::set<VariableAddress> allVars(variables.begin(), variables.end());

    this->bleeper
        ->verbose(this->baudRate)
        .configuration
            .set(this)
            .addObserver(new SettingsCallbackObserver(this->loggingFunction), allVars)
            .done()
        .configurationInterface
            .addDefaultWebServer()
            .done()
        .storage
            .set(new SPIFFSStorage())
            .done()
        .init();
}
