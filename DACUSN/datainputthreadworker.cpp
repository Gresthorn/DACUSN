/**
 * @file datainputthread.cpp
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Definitions of dataInputThread class methods.
 *
 * @section DESCRIPTION
 *
 * The dataInputThread class inherits a QObject class so it is able to run as a 'worker' object
 * in a separate thread which purpose is to obtain new data from UWB radar network
 * and push the data on the stack and wait for another data. This thread has the
 * highest priority because we require no data loss. It uses the 'reciever' class
 * for obtaining data by the correct way.
 *
 */

#include "datainputthreadworker.h"

dataInputThreadWorker::dataInputThreadWorker(QVector<rawData *> *raw_data_stack, QMutex *raw_data_stack_mutex, uwbSettings *setts, QMutex *settings_mutex)
    : QObject()
{
    errorCounterGlobal = errorCounter = 0;

    rawDataStack = raw_data_stack;
    rawDataStackMutex = raw_data_stack_mutex;

    settings = setts;
    settingsMutex = settings_mutex;

    pause = new QWaitCondition;
    pauseMutex = new QMutex;
    pauseState = false;

    stopped = false;
    stoppedCheck = false;
    stoppedMutex = new QMutex;
    stoppedCheckMutex = new QMutex;


    // create approprate reciever -> need to choose appropriate constructor
    settingsMutex->lock();
    if(settings->getRecieverMethod()==RS232) recieverHandler = new reciever(settings->getRecieverMethod(), settings->getComPortName(), settings->getComPortBaudRate(), settings->getComPortMode());
    #if defined (__WIN32__)
    else if(settings->getRecieverMethod()==SYNTHETIC) recieverHandler = new reciever(settings->getRecieverMethod());
    #endif
    else recieverHandler = new reciever(UNDEFINED);
    settingsMutex->unlock();

    // checking if the selected method was successfully established
    if(recieverHandler->calibration_status()) qDebug() << "The reciever was configured";
    else qDebug() << recieverHandler->check_status_message();
}

dataInputThreadWorker::~dataInputThreadWorker()
{
    delete pauseMutex;
    delete pause;

    delete stoppedMutex;

    qDebug() << "Deleting recieve handler from worker object";
    delete recieverHandler;
}

void dataInputThreadWorker::runWorker()
{
    // starting infinite loop for data recieving
    rawData * dataTempPointer;
    unsigned int idle;
    unsigned int maxErrorCount;

    forever
    {
        stoppedMutex->lock();
        if(stopped) { stoppedMutex->unlock(); break; }
        stoppedMutex->unlock();

        pauseMutex->lock();
        if(pauseState)
        {
            // pausing thread
            pause->wait(pauseMutex);
            stoppedMutex->lock();
            if(stopped)
            {
                stoppedMutex->unlock();
                // we have to unlock pause as well because if from some reason the thread will not be stopped
                // correctly by higher classes, the user will have no chance to run back into this thread
                pauseMutex->unlock();
                break;
            }
            stoppedMutex->unlock();
        }
        pauseMutex->unlock();

        dataTempPointer = recieverHandler->listen();
        if(dataTempPointer == NULL)
        {
            // something is wrong
            if(recieverHandler->curr_method_code()==UNDEFINED)
            {
                // no method is specified, making an idle state for 2 or more seconds (settings specified)
                qDebug() << "Reciever responds from idle state";
                settingsMutex->lock();
                idle = settings->getRecieverIdleTime();
                settingsMutex->unlock();
                #if defined (__WIN32__)
                Sleep(idle);
                #endif
                #if defined(__linux__) || defined(__FreeBSD__)
                usleep(idle*1000);
                #endif
            }
            else
            {
                // if no UNDEFINED, the current method is probably corrupted
                qDebug() << recieverHandler->check_status_message();

                // if(recieverHandler->curr_method_code()==RS232) Sleep(500); // give user time to start transmitting - test purposes

                errorCounter++;
                errorCounterGlobal++;

                settingsMutex->lock();
                maxErrorCount = settings->getMaximumRecieverErrorCount();
                idle = settings->getRecieverIdleTime();
                settingsMutex->unlock();
                if(errorCounter>=((int)(maxErrorCount)))
                    #if defined (__WIN32__)
                    Sleep(idle);
                    #endif
                    #if defined(__linux__) || defined(__FreeBSD__)
                    usleep(idle*1000);
                    #endif
            }
        }
        else
        {
            // everything is OK, we can now get new data
            rawDataStackMutex->lock();
            qDebug() << "Appending";
            rawDataStack->append(dataTempPointer);
            rawDataStackMutex->unlock();
            errorCounter = 0;
        }
    }

    // inform higher classes that the loop is finished
    stoppedCheckMutex->lock();
    stoppedCheck = true;
    stoppedCheckMutex->unlock();

    emit finished();
}

void dataInputThreadWorker::switchPauseState()
{
    pauseMutex->lock();
    if(pauseState==false)
    {
        // pausing thread
        pauseState =  true;
    }
    else
    {
        pauseState = false;
        pause->wakeAll();
    }
    pauseMutex->unlock();
}

bool dataInputThreadWorker::checkStoppedStatus()
{
    bool temp;
    stoppedCheckMutex->lock();
    temp = stoppedCheck;
    stoppedCheckMutex->unlock();

    return temp;
}

void dataInputThreadWorker::stopWorker()
{
    stoppedMutex->lock();
    stopped = true;
    stoppedMutex->unlock();
}

void dataInputThreadWorker::releaseIfInPauseState()
{
    pauseMutex->lock();
    if(pauseState==true)
    {
        // change the pause and wake up the thread
        pauseState = false;
        pause->wakeAll();
    }
    pauseMutex->unlock();
}

