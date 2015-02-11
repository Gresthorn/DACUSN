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

private:

    reciever_method recieverMethod; ///< Method used for obtaining data from UWB network
    unsigned int maximumRecieverErrorCount; ///< Determines how much errors one after another are tolerable
    unsigned int recieverIdleTime; ///< Determines how long to wait in miliseconds if the reciever is forwarded to idle state
};

#endif // UWBSETTINGS_H
