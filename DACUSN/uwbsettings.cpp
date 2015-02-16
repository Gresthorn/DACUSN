#include "uwbsettings.h"

uwbSettings::uwbSettings()
{
    recieverMethod = SYNTHETIC;

    maximumRecieverErrorCount = 13;

    recieverIdleTime = 2000;

    stackIdleTime = 5000;

    stackControlPeriodicity = 50;

    maxStackWarningCount = 25;
}

uwbSettings::uwbSettings(char *config)
{
    // not defined yet
}
