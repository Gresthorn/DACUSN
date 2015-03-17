#ifndef UWBSETTINGS_H
#define UWBSETTINGS_H

#include <stdlib.h>
#include <QColor>

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
     * @brief Returns the new color value of the most basic/main grid.
     * @param[in] color Is the QColor pointer to newly generated color.
     */
    void setGridOneColor(QColor * color) { gridOneColor = color; }

    /**
     * @brief Returns the new color value of the intermediate detailed grid.
     * @param[in] color Is the QColor pointer to newly generated color.
     */
    void setGridTwoColor(QColor * color) { gridTwoColor = color; }

    /**
     * @brief Returns the new color value of the most smooth grid.
     * @param[in] color Is the QColor pointer to newly generated color.
     */
    void setGridThreeColor(QColor * color) { gridThreeColor = color; }

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
     * @brief Returns the tapping rendering method selected by user which allows to save some performance by not rendering everything in the scene.
     * @return The return value is enum type specifying the option.
     */
    visualization_tapping_options getTappingRenderMethod(void) { return tapping_opt; }

    /**
     * @brief Allows to change rendering method for background in the scene.
     * @param[in] method Is the enum type specifying the option.
     */
    void setTappingRenderMethod(visualization_tapping_options method) { tapping_opt = method; }

private:

    reciever_method recieverMethod; ///< Method used for obtaining data from UWB network
    unsigned int maximumRecieverErrorCount; ///< Determines how much errors one after another are tolerable
    unsigned int recieverIdleTime; ///< Determines how long to wait in miliseconds if the reciever is forwarded to idle state
    unsigned int stackControlPeriodicity; ///< Determines the number of cycles of reading data from stack after the control of stack filling speed is checked
    unsigned int stackIdleTime; ///< Determines how long to wait in miliseconds in stack reader thread if the stack is empty
    unsigned int maxStackWarningCount; ///< Is the maximum tolarable warning count when doing the speed of stack filling revision

    unsigned int visualizationInterval; ///< Sets how often should be the scene updated.
    visualization_schema visualizationSchema; ///< Holds the user choice of visual effects in scene.

    bool stackRescueEnable; ///< Switch to enable or disable stack rescue functionality

    QColor * gridOneColor; ///< Is the pointer to the color of the most basic/main grid.
    QColor * gridTwoColor; ///< Is the pointer to the color of the intermediate detailed grid.
    QColor * gridThreeColor; ///< Is the pointer to the color of the most smooth grid.
    QColor * backgroundColor; ///< Represents the background color of the scene.

    bool gridOneEnabled; ///< Specify if drawing of grid level one is enabled
    bool gridTwoEnabled; ///< Specify if drawing of grid level two is enabled
    bool gridThreeEnabled; ///< Specify if drawing of grid level three is enabled

    visualization_tapping_options tapping_opt; ///< Holds the information about what rendering method is choosed while tapping.
};

#endif // UWBSETTINGS_H
