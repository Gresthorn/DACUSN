#ifndef DATAINPUTTHREAD_H
#define DATAINPUTTHREAD_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <QWaitCondition>

#include <QDebug>

#include "stddefs.h"
#include "reciever.h"
#include "rawdata.h"
#include "uwbsettings.h"

/**
 * @file datainputthread.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class which represents the thread running in infinity loop for obtaining new data
 *
 * @section DESCRIPTION
 *
 * The dataInputThread class inherits a QObject class so it is able to run as a 'worker' object
 * in a separate thread which purpose is to obtain new data from UWB radar network
 * and push the data on the stack and wait for another data. This thread has the
 * highest priority because we require no data loss. It uses the 'reciever' class
 * for obtaining data by the correct way.
 */

class dataInputThreadWorker : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief Constructor ensures correct initial setup for worker object and save the pointers of mutex and stack
     * @param[in] raw_data_stack Is pointer to stack where all objects containing data are stored
     * @param[in] raw_data_stack_mutex Is pointer to mutex protecting the stack from being accessed by multiple threads at one time
     * @param[in] setts Is pointer to basic object with all software presets, defaults and current settings
     * @param[in] setting_mutex Is the pointer to mutex protecting the settings from being accessed by multiple threads at one time
     *
     * Before the thread may start, it is important to set up several parameters from predefined profile or
     * saved profile. For both purposes the 'uwbSettings' class is needed. It initiates the method for data
     * recieving, prepares 'reciever' object etc. After the constructor is finished, the 'run' method can be
     * started.
     */
    dataInputThreadWorker(QVector<rawData * > * raw_data_stack, QMutex * raw_data_stack_mutex, uwbSettings * setts, QMutex * settings_mutex);

    ~dataInputThreadWorker();

    /**
     * @brief Main function of worker class, which will start the basic inifite cycle for data returning
     */
    Q_INVOKABLE void runWorker(void);

    /**
     * @brief This function will change the current pause state from one to another
     */
    void switchPauseState(void);

    /**
     * @brief Checks if the infinite loop was cancelled
     * @return The return value is boolean value, true if infinite loop was cancelled, false in another case.
     */
    bool checkStoppedStatus(void);

    /**
     * @brief This function will only set the boolean 'stopped' value to true, what leads in break of infinite loop
     */
    void stopWorker(void);

    /**
     * @brief This function will check for 'pauseState' mutex lock and if true, it will unlock it to continue the thread
     *
     * This function is merely used for stopping the thread if it is in pause state. This will automatically continue the
     * thread running and check for 'stopped' boolean value.
     */
    void releaseIfInPauseState(void);

private:

    QVector<rawData * > * rawDataStack; ///< Pointer to stack with all objects containing recieved data

    QMutex * rawDataStackMutex; ///< Pointer to stack mutex required to avoid conflicts with another threads

    uwbSettings * settings; ///< Pointer to settings object in parent window

    QMutex * settingsMutex; ///< Pointer to mutex required to avoid conflicts with another threads

    reciever * recieverHandler; ///< The main reciever object responsible for correct data obtaining

    QWaitCondition * pause; ///< Condition object for thread pausing
    QMutex * pauseMutex; ///< Lock mutex for pause condition object
    bool pauseState; ///< Boolean value determinig wether switch into the pause mode [true] or not [false]

    int errorCounter; ///< Error counter is incremented every time if error during recieving data occures and is set to zero if data are recieved successfully.
    int errorCounterGlobal; ///< ErrorCounterGlobal is incremented every time when error occures during data recieving.

    bool stopped; ///< If set to true, the infinite cycle is broken and connection is closed
    bool stoppedCheck; ///< In contructor set to false. Set to true after the main function really stops.
    QMutex * stoppedMutex; ///< Lock mutex for stopped condition.
    QMutex * stoppedCheckMutex; ///< Lock mutex for check boolean value.

signals:
    /**
     * @brief This signal is emitted when object leaves its infinite loop in runWorker() method
     */
    void finished(void);
};

#endif // DATAINPUTTHREAD_H
