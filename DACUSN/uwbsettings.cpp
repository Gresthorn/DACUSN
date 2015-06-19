/**
 * @file uwbsettings.cpp
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Definitions of uwbSettings class methods.
 *
 * @section DESCRIPTION
 *
 * This class is created automatically when the program starts, in mainWindow constructor.
 * It is a very simple class with all variables needed to store a basic settings required
 * by application to work properly. Because it is used cross the application by different
 * threads, the mutex are needed to protect the class from being accessed by more threads
 * at the same time. Class provides simple functions for settings modification and maintenance.
 *
 */

#include "uwbsettings.h"

uwbSettings::uwbSettings()
{
    /*
     * SEE UWBSETTINGS.H FOR MORE INFORMATION ABOUT VARIABLES AND THEIR DESCRIPTION.
     * CONSTRUCTOR ONLY SETS A DEFAULT VALUES FOR EACH VARIABLE.
     *
     * */

    recieverMethod = RS232;

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

    backgroundColorEnabled = true;

    smoothTransitions = true;

    tapping_opt = RENDER_EVERYTHING;

    engine = STANDARD_QT_PAINTER;

    ogl_buffering_type = DOUBLE_BUFFERING;

    visualization_enabled = true;

    ogl_direct_rendering = true;
    ogl_red_buffer_size = -1;
    ogl_green_buffer_size = -1;
    ogl_blue_buffer_size = -1;
    ogl_alpha_buffer_size = -1;

    ogl_depth_buffer = true;
    ogl_accumulation_buffer = false;
    ogl_stencil_buffer = false;
    ogl_multisample_buffer = true;
    ogl_swap_interval = -1;

    ogl_depth_buffer_size = -1;
    ogl_accumulation_buffer_size = -1;
    ogl_stencil_buffer_size = -1;
    ogl_multisample_buffer_size = -1;

    recordPathHistory = false;

    exportPath = QString("");

    periodicalImgBackup = false;
    periodicalImgBackupPath = QString("");
    periodicalImgBackupInterval = 5000;

    diskBackupEnabled = false;
    diskBackupFilePath.setPath(QDir::currentPath());
    backupFileName.append(QString("radar_backup_%1").arg(QString(QDateTime::currentDateTime().toString()).replace(QRegExp(" |:"), "_")));
    backupMainFileHandler = NULL;
    backupFileHandler = NULL;

    comPort = -1;
    comPortBaudRate = 9600;
    comPortMode[0] = '8';
    comPortMode[1] = 'N';
    comPortMode[2] = '1';
    comPortMode[3] = '\0';

    enableSingleRadarMTT = false;
    enableGlobalRadarMTT = false;
}

uwbSettings::uwbSettings(char *config)
{
    // not defined yet
}
