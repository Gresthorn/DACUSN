#ifndef DATAINPUTTHREAD_H
#define DATAINPUTTHREAD_H

#include <QThread>
#include <QList>
#include <QMutex>

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
 * The dataInputThread class inherits a QThread class so it is able to run as a
 * separate thread which purpose is to obtain new data from UWB radar network
 * and push the data on the stack and wait for another data. This thread has the
 * highest priority because we require no data loss. It uses the 'reciever' class
 * for obtaining data by the correct way. Any interaction from the user side as
 * changing settings for example are managed by slots. Signals from the class are
 * then routed to slots of mainWindow object. Messages are sent by signal-slot
 * mechanism to mainWindow object.
 */

class dataInputThread : public QThread
{
    Q_OBJECT

public:

    /**
     * @brief Constructor ensures correct initial setup for thread and save the pointers of mutex and stack
     * @param[in] parent Pointer to parent object
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
    dataInputThread(QObject * parent, QList<rawData * > * raw_data_stack, QMutex * raw_data_stack_mutex, uwbSettings * setts, QMutex * settings_mutex);

    ~dataInputThread();

private:

    QList<rawData * > * rawDataStack; ///< Pointer to stack with all objects containing recieved data

    QMutex * rawDataStackMutex; ///< Pointer to stack mutex required to avoid conflicts with another threads

    uwbSettings * settings; ///< Pointer to settings object in parent window

    QMutex * settingsMutex; ///< Pointer to mutex required to avoid conflicts with another threads

    reciever * recieverHandler; ///< The main reciever object responsible for correct data obtaining

    int errorCounter; ///< Error counter is incremented every time if error during recieving data occures and is set to zero if data are recieved successfully.
    int errorCounterGlobal; ///< ErrorCounterGlobal is incremented every time when error occures during data recieving.

protected:
    void run();

};

#endif // DATAINPUTTHREAD_H
