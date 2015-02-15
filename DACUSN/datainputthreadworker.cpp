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

    settingsMutex->lock();
    recieverHandler = new reciever(settings->getRecieverMethod());
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

    delete recieverHandler;
}

void dataInputThreadWorker::runWorker()
{
    // starting infinite loop for data recieving
    rawData * dataTempPointer;
    unsigned int idle;
    unsigned int maxErrorCount;
    while(1)
    {
        stoppedMutex->lock();
        if(stopped) { stoppedMutex->unlock(); break; }
        stoppedMutex->unlock();

        pauseMutex->lock();
        if(pauseState==true)
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
                // no method is specified, making an idle state for 2 seconds
                qDebug() << "Reciever responds from idle state";
                settingsMutex->lock();
                idle = settings->getRecieverIdleTime();
                settingsMutex->unlock();
                Sleep(idle);
            }
            else
            {
                // if no UNDEFINED, the current method is probably corrupted
                qDebug() << recieverHandler->check_status_message();

                errorCounter++;
                errorCounterGlobal++;

                settingsMutex->lock();
                maxErrorCount = settings->getMaximumRecieverErrorCount();
                idle = settings->getRecieverIdleTime();
                settingsMutex->unlock();
                if(errorCounter>=maxErrorCount) Sleep(idle);

            }
        }
        else
        {
            // everything is OK, we can now get new data
            rawDataStackMutex->lock();
            rawDataStack->append(dataTempPointer);
            rawDataStackMutex->unlock();
            errorCounter = 0;
        }
    }

    // inform higher classes that the loop is finished
    stoppedCheckMutex->lock();
    stoppedCheck = true;
    stoppedCheckMutex->unlock();
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

