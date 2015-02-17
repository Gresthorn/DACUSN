#include "uwbsettings.h"

uwbSettings::uwbSettings()
{
    recieverMethod = SYNTHETIC;

    maximumRecieverErrorCount = 13;

    recieverIdleTime = 2000;

    stackIdleTime = 100;

    stackControlPeriodicity = 50;

    maxStackWarningCount = 25;

    stackRescueEnable = true;
}

uwbSettings::uwbSettings(char *config)
{
    // not defined yet
}
