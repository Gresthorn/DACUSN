#include "uwbsettings.h"

uwbSettings::uwbSettings()
{
    recieverMethod = SYNTHETIC;

    maximumRecieverErrorCount = 13;

    recieverIdleTime = 2000;
}

uwbSettings::uwbSettings(char *config)
{
    // not defined yet
}
