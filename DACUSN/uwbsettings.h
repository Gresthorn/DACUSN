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
    void setMaximumRecieverErrorCount(int error_count) { maximumRecieverErrorCount = error_count; }

    /**
     * @brief Is method used by higher classes to find out, what upper limit for maximum tolerable error count for reciever is used.
     * @return Returns the maximum tolerable error count value.
     */
    int getMaximumRecieverErrorCount(void) { return maximumRecieverErrorCount; }

private:

    reciever_method recieverMethod; ///< Method used for obtaining data from UWB network
    int maximumRecieverErrorCount; ///< Determines how much errors one after another are tolerable
};

#endif // UWBSETTINGS_H
