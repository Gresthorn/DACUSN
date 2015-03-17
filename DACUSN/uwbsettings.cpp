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

    visualizationSchema = COMMON_FLOW;

    gridOneColor = new QColor(111, 111, 111, 255);

    gridTwoColor = new QColor(111, 111, 111, 100);

    gridThreeColor = new QColor(0,0,0, 150);

    backgroundColor = new QColor(255, 255, 255, 255);

    gridOneEnabled = true;

    gridTwoEnabled = true;

    gridThreeEnabled = true;

    tapping_opt = RENDER_EVERYTHING;
}

uwbSettings::uwbSettings(char *config)
{
    // not defined yet
}
