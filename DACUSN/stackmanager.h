#ifndef STACKMANAGER_H
#define STACKMANAGER_H

#include <QObject>
#include <QMutex>
#include <QVector>
#include <QThread>
#include <QWaitCondition>

#include <QDebug>

#include "rawdata.h"
#include "stddefs.h"
#include "uwbsettings.h"

/**
 * @file reciever.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class which provides interface for data stack management
 *
 * @section DESCRIPTION
 *
 * The 'stackManager' class provides a basic interface for manipulation with the stack.
 * After the data are recieved by 'reciever', they are pushed onto the stack which then
 * must be protected by mutex. This stack, however can obtain more data, than this class
 * may be able to process. Reasons may differ (too many input data, very complex processing
 * algorithms, time consuming rendering session etc.). In such cases some of special methods
 * are needed to solve this issue. The stack must free memory in time and 'rawData' objects
 * must be deleted after processing. The instance based on this class serves as a worker
 * object for stack management thread. This thread is never deleted and remains active
 * during the application run. Only actions applied on the reciever thread as pause, stop will
 * pause/unpause the thread.
 *
 */

class stackManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief The main constructor of 'stackManager' class will prepare the instance for first start.
     * @param[in] raw_data_stack Is the pointer to the stack of recieved data.
     * @param[in] raw_data_stack_mutex The mutex locking the stack during data processing.
     * @param[in] setts The pointer to basic settings object.
     * @param[in] settings_mutex The mutex locking the settings object during reading some settings value.
     *
     * This constructor will obtain all pointers and mutex pointers when creating the object from the main
     * application thread. These pointers are saved and a few important values are initialized to their
     * default values (later they are rewritten by settings values if they are specified).
     */
    stackManager(QVector<rawData * > * raw_data_stack, QMutex * raw_data_stack_mutex, uwbSettings * setts, QMutex * settings_mutex);
    ~stackManager();

    /**
      * @brief The main function of stack management thread, running in infinite loop.
      */
    Q_INVOKABLE void runWorker(void);

    /**
     * @brief Switches the pause state value.
     */
    void switchPauseState(void);

private:
    QVector<rawData * > * rawDataStack; ///< Pointer to the stack
    QMutex * rawDataStackMutex; ///< Pointer to the mutex locking the stack
    uwbSettings * settings; ///< Pointer to the basic application settings object
    QMutex * settingsMutex; ///< Pointer to the mutex locking the settings object

    unsigned int idleTime; ///< The idle time, during the thread is sleeping after it find out that the stack is empty
    unsigned int stackControlPeriodicity; ///< Number of cycles that must pass until the 'stackControl' function is run
    unsigned int lastStackCount; ///< The last known stack items count from the lastly done stack control
    unsigned int stackWarningCount; ///< The warning count is incremented when the stack control find some undesired issue
    unsigned int maxStackWarningCount; ///< The maximum warning count until the 'rescue' function is run

    bool rescueEnabled; ///< Determines wether run the rescue function or just inform the user about critical state

    QMutex * pauseMutex; ///< Mutex protecting the pause variable from being accessed by multiple threads at once
    bool pauseState; ///< Determines if pause state is on/off
    QWaitCondition * pause; ///< The wait condition that ensures the correct pause of worker cycle in stack management thread


    /**
     * @brief This function will control the stack situation and its behaviour in time (checks for speed of data input) and prevent the stack from being filled too fastly.
     * @param[in] stackControlCounter Is the adress of variable declared inside the thread and serves as a counter for 'stackControl' function.
     */
    void stackControl(unsigned int * stackControlCounter);

    /**
     * @brief The data processing function will obtain a pointer to 'rawData' object and process the obtained data and passes them to 'radarUnit' objects.
     * @param[in] data Is the pointer to the 'rawData' object lastly token from the stack.
     */
    void dataProcessing(rawData * data);

public slots:
    /**
     * @brief This function is called always when the processing is slower than the data obtaining and there is a danger that stack will be overflowed.
     */
    void rescue(void);
};

#endif // STACKMANAGER_H
