#include "uwbsettings.h"

uwbSettings::uwbSettings()
{
    recieverMethod = SYNTHETIC;

    maximumRecieverErrorCount = 13;

    recieverIdleTime = 2000;

    stackIdleTime = 100;

    stackControlPeriodicity = 10;

    maxStackWarningCount = 10;

    stackRescueEnable = true;

    visualizationInterval = 38;

    visualizationSchema = COMET_EFFECT;
}

uwbSettings::uwbSettings(char *config)
{
    // not defined yet
}
