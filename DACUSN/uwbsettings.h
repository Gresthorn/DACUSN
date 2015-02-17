#ifndef UWBSETTINGS_H
#define UWBSETTINGS_H

#include <stdlib.h>

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
private:

    reciever_method recieverMethod; ///< Method used for obtaining data from UWB network
    unsigned int maximumRecieverErrorCount; ///< Determines how much errors one after another are tolerable
    unsigned int recieverIdleTime; ///< Determines how long to wait in miliseconds if the reciever is forwarded to idle state
    unsigned int stackControlPeriodicity; ///< Determines the number of cycles of reading data from stack after the control of stack filling speed is checked
    unsigned int stackIdleTime; ///< Determines how long to wait in miliseconds in stack reader thread if the stack is empty
    unsigned int maxStackWarningCount; ///< Is the maximum tolarable warning count when doing the speed of stack filling revision

    bool stackRescueEnable; ///< Switch to enable or disable stack rescue functionality
};

#endif // UWBSETTINGS_H
