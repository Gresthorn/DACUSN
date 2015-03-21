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

    gridOneColor = new QColor(0, 0, 0, 255);

    gridTwoColor = new QColor(111, 111, 111, 255);

    gridThreeColor = new QColor(200,200,200, 255);

    backgroundColor = new QColor(255, 255, 255, 255);

    gridOneEnabled = true;

    gridTwoEnabled = true;

    gridThreeEnabled = true;

    tapping_opt = RENDER_EVERYTHING;

    engine = STANDARD_QT_PAINTER;
}

uwbSettings::uwbSettings(char *config)
{
    // not defined yet
}
