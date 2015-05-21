#ifndef UWBSETTINGS_H
#define UWBSETTINGS_H

#include <stdlib.h>
#include <QColor>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QFile>

#include "stddefs.h"

/**
 * @file uwbsettings.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class which serves as a storage for all application settings during program execution
 *
 * @section DESCRIPTION
 *
 * This class is created automatically when the program starts, in mainWindow constructor.
 * It is a very simple class with all variables needed to store a basic settings required
 * by application to work properly. Because it is used cross the application by different
 * threads, the mutex are needed to protect the class from being accessed by more threads
 * at the same time. Class provides simple functions for settings modification and maintenance.
 */

class uwbSettings
{
public:
    /**
     * @brief Constructor initializes all values with default values.
     *
     * This contructor will initialize all inner variables with default values.
     */
    uwbSettings();

    /**
     * @brief Constructor initializes all values with values stored in configuration file.
     * @param[in] config Path to a file with all configuration values.
     *
     * This contructor will initialize all inner variables with values stored in configuration
     * file which path is passed as the parameter.
     */
    uwbSettings(char * config);

    /**
     * @brief Is method used by higher classes to find out, what method is currently used for data recieving.
     * @return Returns reciever method as the enumeration type.
     */
    reciever_method getRecieverMethod(void) { return recieverMethod; }

    /**
     * @brief Function sets new recieving method enumeration type.
     * @param[in] method Specifies the new method used for data obtaining.
     */
    void setRecieverMethod(reciever_method method) { recieverMethod = method; }

    /**
     * @brief Function sets new upper limit for maximum tolerable error count for reciever.
     * @param[in] error_count Specifies the new error count.
     */
    void setMaximumRecieverErrorCount(unsigned int error_count) { maximumRecieverErrorCount = error_count; }

    /**
     * @brief Sets the new comport ID which will be used for communication initialization. Note that in Linux and Windows comport indexing is different.
     * @param[in] com_port_id COM port ID (e. g. COM5 in windows has index 4)
     */
    void setComPortNumber(int com_port_id = -1) { comPort = com_port_id; }

    /**
     * @brief Sets the new baudrate for serial communication.
     * @param[in] baud_rate New baudrate to be set. Default baudrate is 9600.
     */
    void setComPortBaudRate(int baud_rate = 9600) { comPortBaudRate = baud_rate; }

    /**
     * @brief Sets the new mode for serial communication. This mode is expressed as 3 characters long string. Possible modes are availible at RS232-library (author's) documentation at: http://www.teuniz.net/RS-232/
     * @param[in] mode New string with required mode string pointer.
     */
    void setComPortMode(char * mode) { comPortMode[0] = mode[0]; comPortMode[1] = mode[1]; comPortMode[2] = mode[2]; }

    /**
     * @brief Returns currently used COM port index.
     * @return The return value is COM port number.
     */
    int getComPortNumber(void) { return comPort; }

    /**
     * @brief Is used when discovery of currently used baudrate is needed.
     * @return  The return value is baudrate expressed as integer number.
     */
    int getComPortBaudRate(void) { return comPortBaudRate; }

    /**
     * @brief Is used for obtaining currently set mode for serial communication. Note that the return value is the copy of string and needs to be released later.
     * @return The return value is pointer to the copy of string holding the information of currently used RS232 mode.
     */
    char * getComPortMode(void) { return strdup(comPortMode); }

    /**
     * @brief Is method used by higher classes to find out, what upper limit for maximum tolerable error count for reciever is used.
     * @return Returns the maximum tolerable error count value.
     */
    unsigned int getMaximumRecieverErrorCount(void) { return maximumRecieverErrorCount; }

    /**
     * @brief This method can be used to set new idle time value for reciever in miliseconds.
     * @param[in] idleTime Parameter carrying the idle time value.
     */
    void setRecieverIdleTime(unsigned int idleTime) { recieverIdleTime = idleTime; }

    /**
     * @brief This method is used to find out what idle time in miliseconds is currently used.
     * @return The return value is the idle time value for reciever in miliseconds.
     */
    unsigned int getRecieverIdleTime(void) { return recieverIdleTime; }

    /**
     * @brief This function is used to retrieve the stack control periodicity value from settings object.
     * @return The return value is the cycles count of stack reader thread.
     *
     * When obtaing data from uwb sensor network, sometimes can stack be filled faster than the program can
     * process the data from it. In that case, the stack is getting still more memory and may require the
     * user action. The speed of how stack is filled is periodically controlled after the 'stackControlPeriodicity'
     * cycles.
     */
    unsigned int getStackControlPeriodicity(void) { return stackControlPeriodicity; }

    /**
     * @brief This function is used to set the stack control periodicity value into settings object.
     * @param[in] The input value is the cycles count of stack reader thread.
     *
     * When obtaing data from uwb sensor network, sometimes can stack be filled faster than the program can
     * process the data from it. In that case, the stack is getting still more memory and may require the
     * user action. The speed of how stack is filled is periodically controlled after the 'stackControlPeriodicity'
     * cycles.
     */
    void setStackControlPeriodicity(unsigned int periodicity) { stackControlPeriodicity = periodicity; }

    /**
     * @brief This function sets the idle time of stack reader thread if stack is empty and saves CPU some of unnecessary processing
     * @param[in] The idle time in miliseconds.
     */
    void setStackIdleTime(unsigned int idleTime) { stackIdleTime = idleTime; }

    /**
     * @brief This function returns the idle time of stack reader thread if stack is empty and saves CPU some of unnecessary processing
     * @return The return value is the idle time in miliseconds.
     */
    unsigned int getStackIdleTime(void) { return stackIdleTime; }

    /**
     * @brief This function sets the new value for maximum stack warning counter.
     * @param[in] The input value is the value that is saved as the new 'maxStackWarningCount'
     */
    void setMaxStackWarningCount(unsigned int maxCount) { maxStackWarningCount = maxCount; }

    /**
     * @brief This function is used by stack control mechanism to check for maximum tolerable warning count.
     * @return The return value is integer value of maximu tolerable warning count for stack control mechanism.
     */
    unsigned int getMaxStackWarningCount(void) { return maxStackWarningCount; }

    /**
     * @brief This function sets the new boolean value for stack rescue algorithm switch.
     * @param[in] stackRescue The input value is new boolean value for stackRescueEnable.
     */
    void setStackRescueState(bool stackRescue) { stackRescueEnable = stackRescue; }

    /**
     * @brief This function will read the stack rescue state from settings object.
     * @return The return value is boolean value of stack rescue state.
     */
    bool getStackRescueState(void) { return stackRescueEnable; }

    /**
     * @brief Sets new interval for scene update sequence in miliseconds.
     * @param[in] interval Is the new value of interval in miliseconds.
     */
    void setVisualizationInterval(unsigned int interval) { visualizationInterval = interval; }

    /**
     * @brief This function is used for checking out how often should the scene update be done.
     * @return The return value is interval in miliseconds.
     */
    unsigned int getVisualizationInterval(void) { return visualizationInterval; }

    /**
     * @brief Sets the new visualization schema.
     * @param[in] schema Is the new schema set for visualization algorithms.
     */
    void setVisualizationSchema(visualization_schema schema) { visualizationSchema = schema; }

    /**
     * @brief Returns the visualization schema currently set and used for targets display.
     * @return The return value is the enum type of schema currently set.
     */
    visualization_schema getVisualizationSchema(void) { return visualizationSchema; }

    /**
     * @brief Returns the color value of the most basic/main grid.
     * @return The return value is the QColor pointer.
     */
    QColor * getGridOneColor(void) { return gridOneColor; }

    /**
     * @brief Returns the color value of the intermediate detailed grid.
     * @return The return value is the QColor pointer.
     */
    QColor * getGridTwoColor(void) { return gridTwoColor; }

    /**
     * @brief Returns the color value of the most smooth grid.
     * @return The return value is the QColor pointer.
     */
    QColor * getGridThreeColor(void) { return gridThreeColor; }

    /**
     * @brief Returns the color value of the scene background.
     * @return The return value is the QColor pointer.
     */
    QColor * getBackgroundColor(void) { return backgroundColor; }

    /**
     * @brief Sets the new color value of the most basic/main grid.
     * @param[in] color Is the QColor pointer to newly generated color.
     */
    void setGridOneColor(QColor * color) { gridOneColor = color; }

    /**
     * @brief Sets the new color value of the intermediate detailed grid.
     * @param[in] color Is the QColor pointer to newly generated color.
     */
    void setGridTwoColor(QColor * color) { gridTwoColor = color; }

    /**
     * @brief Sets the new color value of the most smooth grid.
     * @param[in] color Is the QColor pointer to newly generated color.
     */
    void setGridThreeColor(QColor * color) { gridThreeColor = color; }

    /**
     * @brief Sets the new color value for scene background.
     * @param[in] color Is the QColor pointer to newly generated color.
     */
    void setBackgroundColor(QColor * color) { backgroundColor = color; }

    /**
     * @brief Allows to check if drawing grid of level one is enabled;
     * @return The return value is boolean value equal to true if the setting is enabled.
     */
    bool gridOneIsEnabled(void)  { return gridOneEnabled; }

    /**
     * @brief Allows to check if drawing grid of level two is enabled;
     * @return The return value is boolean value equal to true if the setting is enabled.
     */
    bool gridTwoIsEnabled(void)  { return gridTwoEnabled; }

    /**
     * @brief Allows to check if drawing grid of level three is enabled;
     * @return The return value is boolean value equal to true if the setting is enabled.
     */
    bool gridThreeIsEnabled(void) { return gridThreeEnabled; }

    /**
     * @brief Allows to check if drawing background enabled;
     * @return The return value is boolean value equal to true if the setting is enabled.
     */
    bool backgroundIsEnabled(void) { return backgroundColorEnabled; }

    /**
     * @brief Allows to enable/disable first level of grid.
     * @param[in] enable The new enable/disable value for grid.
     */
    void setGridOneEnable(bool enable) { gridOneEnabled = enable; }

    /**
     * @brief Allows to enable/disable second level of grid.
     * @param[in] enable The new enable/disable value for grid.
     */
    void setGridTwoEnable(bool enable) { gridTwoEnabled = enable; }

    /**
     * @brief Allows to enable/disable third level of grid.
     * @param[in] enable The new enable/disable value for grid.
     */
    void setGridThreeEnable(bool enable) { gridThreeEnabled = enable; }

    /**
     * @brief Allows to enable/disable background of the scene.
     * @param[in] enable The new enable/disable value for background.
     */
    void setBackgroundColorEnable(bool enable) { backgroundColorEnabled = enable; }

    /**
     * @brief This function retrievs the setting of smooth transitions animation state.
     * @return Return value is boolean value representing state.
     */
    bool getSmoothTransitions(void) { return smoothTransitions; }

    /**
     * @brief The following function is able to set the new setting for smooth transition setting.
     * @param[in] enable Is parameter representing the new value for setting.
     */
    void setSmootheTransitions(bool enable) { smoothTransitions = enable; }

    /**
     * @brief Returns the tapping rendering method selected by user which allows to save some performance by not rendering everything in the scene.
     * @return The return value is enum type specifying the option.
     */
    visualization_tapping_options getTappingRenderMethod(void) { return tapping_opt; }

    /**
     * @brief Allows to change rendering method for background in the scene.
     * @param[in] method Is the enum type specifying the option.
     */
    void setTappingRenderMethod(visualization_tapping_options method) { tapping_opt = method; }

    /**
     * @brief If the applications allows to use more than one rendering engine, the currently using engine can be retrieved by this function.
     * @return The enum type specifying the engine used.
     */
    rendering_engine getRenderingEngine(void) { return engine; }

    /**
     * @brief Sets the new engine type. This function however is not responsible for switching the program/inner operations and settings.
     * @param[in] new_engine Is the enum type representing new engine that we wish to set up.
      */
    void setRenderingEngine(rendering_engine new_engine) { engine = new_engine; }

    /**
     * @brief Sets wether the path passed by target should be drawn.
     * @param[in] enable Is the new value for drawing history status.
     */

    /**
     * @brief Sets the new export path for images that are exported from instant conversion from view.
     * @param[in] path Is the new value for export path.
     */
    void setExportPath(QString path) { exportPath = path; }

    /**
     * @brief Function will return currently set export path for images.
     * @return The return value is QString containing path.
     */
    QString getExportPath(void) { return exportPath; }

    /**
     * @brief This function enables/disables periodical image export from view.
     * @param[in] enabled Is true if periodical backup is enabled.
     */
    void setPeriodicalImgBackup(bool enabled) { periodicalImgBackup = enabled; }

    /**
     * @brief Allows to check if periodical backup is enabled.
     * @return Is boolean value indicating the periodical backup status.
     */
    bool getPeriodicalImgBackup(void) { return periodicalImgBackup; }

    /**
     * @brief Sets up the destination of exported images if periodical backup is enabled.
     * @param[in] path The new path for exported images.
     */
    void setPeriodicalImgBackupPath(QString path) { periodicalImgBackupPath = path; }

    /**
     * @brief Returns the currently used destination for images generated from view by periodical backup sequence.
     * @return Is the string showing to directory used as the currently used destination.
     */
    QString getPeriodicalImgBackupPath(void) { return periodicalImgBackupPath; }

    void setHistoryPath(bool enable) { recordPathHistory = enable; }

    /**
     * @brief This function returns the currently used interval for periodical image generation timer.
     * @return The return value is time interval in miliseconds.
     */
    unsigned int getPeriodicalImgBackupInterval(void) { return periodicalImgBackupInterval; }

    /**
     * @brief Sets the new break time interval for timer causing the periodical image export from view in miliseconds.
     * @param[in] interval Is the new milisecond interval for break between two image generation sequence.
     */
    void setPeriodicalImgBackupInterval(unsigned int interval) { periodicalImgBackupInterval = interval; }

    /**
     * @brief Allows to check if history path is being currently drawn in the view.
     * @return Returns the 'showPathHistory' status.
     */
    bool getHistoryPath(void) { return recordPathHistory; }

    /**
     * @brief Function allows retrieve information about wether visualization is turned on/off.
     * @return The return value is boolean value representing the mentioned state.
     */
    bool getVisualizationEnabled(void) { return visualization_enabled; }

    /**
     * @brief Allows to set new state for visualization. If set to true, rendering will be started, else background processing will remain without rendering.
     * @param[in] enabled Is the new value for visualization enable state.
     */
    void setVisualizationEnabled(bool enabled) { visualization_enabled = enabled; }

    /**
     * @brief Returns the file path (without file name) where the backup text file is going to be placed.
     */
    QString getDiskBackupFilePath(void) { return diskBackupFilePath.path(); }

    /**
     * @brief Sets the backup file path by string passed to it.
     * @param[in] path String with new file path.
     */
    void setDiskBackupFilePath(QString path) { diskBackupFilePath.setPath(path); }

    /**
     * @brief Returns the copy of 'QDir' object with file path already set. May be usefull by some functions.
     * @return The return value is QDir object.
     */
    QDir getDiskBackupFilePathObject(void) { return diskBackupFilePath; }

    /**
     * @brief Returns the string with backup file name without the path prepended.
     * @return The return value is QString object.
     */
    QString getBackupFileName(void) { return backupFileName; }

    /**
     * @brief Sets the new file name of target's backup file without prepended path or appended file type.
     * @param[in] path String with new file name.
     */
    void setBackupFileName(QString filename) { backupFileName.clear(); backupFileName.append(filename); }

    /**
     * @brief Returns boolean indicator for checking if the backup was allowed or not.
     * @return Boolean value representing the backup sequence enabled/disabled status.
     */
    bool getDiskBackupEnabled(void) { return diskBackupEnabled; }

    /**
     * @brief Sets the new status for backup sequence.
     * @param[in] enabled Boolean status. If true, backup sequence is running.
     */
    void setDiskBackupEnabled(bool enabled) { diskBackupEnabled = enabled; }

    /**
     * @brief Returns the file object that was opened for the backup sequence.
     * @return Pointer to the opened file object.
     */
    QFile * getBackupMainFileHandler(void) { return backupMainFileHandler; }

    /**
     * @brief Sets the new file handler. This file handler/object must be opened in text mode and has the write in privileges.
     * @param[in] New 'QFile handler' opened in text mode and set up for writing.
     */
    void setBackupMainFileHandler(QFile * file) { backupMainFileHandler = file; }

    /**
     * @brief Returns the pointer to the 'QTextStream' object currently used for data writing.
     * @return The return value QTextStream pointer.
     */
    QTextStream * getBackupFileHandler(void) { return backupFileHandler; }

    /**
     * @brief Sets the new stream object pointer for data backup use.
     * @param[in] handler Is the pointer to the newly created stream used for data backup.
     */
    void setBackupFileHandler(QTextStream * handler) { backupFileHandler = handler; }

    /** THE FOLLOWING FUNCTIONS ARE ABLE TO RETRIEVE BASIC OPENGL SETTINGS **/

    rendering_engine_buffer_type oglGetBufferType(void) { return ogl_buffering_type; }
    bool oglGetDirectRendering(void) { return ogl_direct_rendering; }
    bool oglGetDepthBuffer(void) { return ogl_depth_buffer; }
    bool oglGetAccumulationBuffer(void) { return ogl_accumulation_buffer; }
    bool oglGetStencilBuffer(void) { return ogl_stencil_buffer; }
    bool oglGetMultisampleBuffer(void) { return ogl_multisample_buffer; }
    int oglGetSwapInterval(void) { return ogl_swap_interval; }
    int oglGetMultisampleBufferSize(void) { return ogl_multisample_buffer_size; }
    int oglGetStencilBufferSize(void) { return ogl_stencil_buffer_size; }
    int oglGetAccumulationBufferSize(void) { return ogl_accumulation_buffer_size; }
    int oglGetDepthBufferSize(void) { return ogl_depth_buffer_size; }
    int oglGetRedBufferSize(void) { return ogl_red_buffer_size; }
    int oglGetGreenBufferSize(void) { return ogl_green_buffer_size; }
    int oglGetBlueBufferSize(void) { return ogl_blue_buffer_size; }
    int oglGetAlphaBufferSize(void) { return ogl_alpha_buffer_size; }

    /** THE FOLLOWING FUNCTIONS ARE ABLE TO SET BASIC OPENGL SETTINGS **/

    void oglSetBufferType(rendering_engine_buffer_type type) { ogl_buffering_type = type; }
    void oglSetDirectRendering(bool enabled) { ogl_direct_rendering = enabled; }
    void oglSetDepthBuffer(bool enabled) { ogl_depth_buffer = enabled; }
    void oglSetAccumulationBuffer(bool enabled) { ogl_accumulation_buffer = enabled; }
    void oglSetStencilBuffer(bool enabled) { ogl_stencil_buffer = enabled; }
    void oglSetMultisampleBuffer(bool enabled) { ogl_multisample_buffer = enabled; }
    void oglSetSwapInterval(int value) { ogl_swap_interval = value; }
    void oglSetMultisampleBufferSize(int value) { ogl_multisample_buffer_size = value; }
    void oglSetStencilBufferSize(int value) { ogl_stencil_buffer_size = value; }
    void oglSetAccumulationBufferSize(int value) { ogl_accumulation_buffer_size = value; }
    void oglSetDepthBufferSize(int value) { ogl_depth_buffer_size = value; }
    void oglSetRedBufferSize(int value) { ogl_red_buffer_size = value; }
    void oglSetGreenBufferSize(int value) { ogl_green_buffer_size = value; }
    void oglSetBlueBufferSize(int value) { ogl_blue_buffer_size = value; }
    void oglSetAlphaBufferSize(int value) { ogl_alpha_buffer_size = value; }

    /**
     * @brief This function will set new approval state for single radar MTT application. If set to false, no MTT algorithm will be applied for individuall radar unit's new data.
     * @param[in] enable New MTT approval state.
     */
    void setSingleRadarMTT(bool enable) { enableSingleRadarMTT = enable; }

    /**
     * @brief This function will set new approval state for global MTT application. If set to true, MTT algorithm will be applied for all radar data instead of averaging.
     * @param enable
     */
    void setGlobalRadarMTT(bool enable) { enableGlobalRadarMTT = enable; }

    /**
     * @brief Retrieves currently used single radar MTT algorithm state.
     * @return Boolean value. If true, on each radar unit, after new data are recieved, MTT algorithm is applied.
     */
    bool getSingleRadarMTT(void) { return enableSingleRadarMTT; }

    /**
     * @brief Retrieves currently used global MTT algorithm state.
     * @return Boolean value. If true, before data are passed into visualizationData list, instead of averaging, MTT algorithm will be applied.
     */
    bool getGlobalRadarMTT(void) { return enableGlobalRadarMTT; }

private:

    reciever_method recieverMethod; ///< Method used for obtaining data from UWB network
    unsigned int maximumRecieverErrorCount; ///< Determines how much errors one after another are tolerable
    unsigned int recieverIdleTime; ///< Determines how long to wait in miliseconds if the reciever is forwarded to idle state
    unsigned int stackControlPeriodicity; ///< Determines the number of cycles of reading data from stack after the control of stack filling speed is checked
    unsigned int stackIdleTime; ///< Determines how long to wait in miliseconds in stack reader thread if the stack is empty
    unsigned int maxStackWarningCount; ///< Is the maximum tolarable warning count when doing the speed of stack filling revision

    int comPort; ///< Specifies the index of COM port in operating system, which is used for data recieving
    int comPortBaudRate; ///< Holds the information about speed used for serial link commuication
    char comPortMode[4]; ///< Used for serial link communication initialization with some options. See this site for more information: http://www.teuniz.net/RS-232/

    unsigned int visualizationInterval; ///< Sets how often should be the scene updated.
    visualization_schema visualizationSchema; ///< Holds the user choice of visual effects in scene.

    bool stackRescueEnable; ///< Switch to enable or disable stack rescue functionality

    QColor * gridOneColor; ///< Is the pointer to the color of the most basic/main grid.
    QColor * gridTwoColor; ///< Is the pointer to the color of the intermediate detailed grid.
    QColor * gridThreeColor; ///< Is the pointer to the color of the most smooth grid.
    QColor * backgroundColor; ///< Represents the background color of the scene.

    bool visualization_enabled; ///< If user wants to save some processing power, he may disable visualization while background processing will be done without rendering.

    bool gridOneEnabled; ///< Specify if drawing of grid level one is enabled.
    bool gridTwoEnabled; ///< Specify if drawing of grid level two is enabled.
    bool gridThreeEnabled; ///< Specify if drawing of grid level three is enabled.
    bool backgroundColorEnabled; ///< Specify if drawing of background is enabled.
    bool smoothTransitions; ///< Enables/Disables smooth transitions during zooming, changing angles or moving to [x, y] position etc.

    bool recordPathHistory; ///< If this option is enabled, then the path that target has passed since the settings was enabled is recorded.

    visualization_tapping_options tapping_opt; ///< Holds the information about what rendering method is choosed while tapping.

    rendering_engine engine; ///< Holds the information about what rendering engine is used by graphics view to visualize objects.

    rendering_engine_buffer_type ogl_buffering_type; ///< Specifies the buffer type if openGL rendering engine is on

    ///< The following variables are basic options for openGL rendering engine. See documentation for more information.
    bool ogl_direct_rendering;
    int ogl_red_buffer_size;
    int ogl_green_buffer_size;
    int ogl_blue_buffer_size;
    int ogl_alpha_buffer_size;

    bool ogl_depth_buffer;
    bool ogl_accumulation_buffer;
    bool ogl_stencil_buffer;
    bool ogl_multisample_buffer;
    int ogl_swap_interval;

    int ogl_depth_buffer_size;
    int ogl_accumulation_buffer_size;
    int ogl_stencil_buffer_size;
    int ogl_multisample_buffer_size;


    QString exportPath; ///< Allows to save image from the current view in directory specified in this path.
    bool periodicalImgBackup; ///< Indicates if periodical image export from the view is enabled.
    QString periodicalImgBackupPath; ///< Specifies the path for periodical export images.
    unsigned int periodicalImgBackupInterval; ///< Interval in miliseconds that is passed to the timer evoking the periodical img export slot.

    QDir diskBackupFilePath; ///< When saving data to disk is enabled, here the path to the file is stored.
    QString backupFileName; ///< Backup file name used.
    bool diskBackupEnabled; ///< Specifies if save to disk or not.
    QFile * backupMainFileHandler; ///< FILE object of opened backup file.
    QTextStream * backupFileHandler; ///< Stores the 'handler' where can new data be written and redirected to file.

    bool enableSingleRadarMTT; ///< Switches on/off single radar MTT. If turned on, every radar will apply MTT on newly recieved data.
    bool enableGlobalRadarMTT; ///< Switches on/off global MTT algorithm. If turned on, averaging data will be replaced with MTT algorithm.
};

#endif // UWBSETTINGS_H
