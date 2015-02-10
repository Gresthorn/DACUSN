#include "uwbsettings.h"

uwbSettings::uwbSettings()
{
    recieverMethod = SYNTHETIC;

    maximumRecieverErrorCount = 13;
}

uwbSettings::uwbSettings(char *config)
{
    // not defined yet
}
