#include "stackmanager.h"

stackManager::stackManager(QVector<rawData *> *raw_data_stack, QMutex *raw_data_stack_mutex, uwbSettings *setts, QMutex *settings_mutex)
{
    rawDataStack = raw_data_stack;
    rawDataStackMutex = raw_data_stack_mutex;
    settings = setts;
    settingsMutex = settings_mutex;

    idleTime = 200;
    stackControlPeriodicity = 50;
    maxStackWarningCount = 10;
    lastStackCount = 0;

    rescueEnabled = true;

    pauseState = false;
    pauseMutex = new QMutex;
    pause = new QWaitCondition;

    stopped = false;
    stoppedMutex = new QMutex;

    stoppedCheckMutex = new QMutex;
    stoppedCheck = false;
}

stackManager::~stackManager()
{
    delete pause;
    delete pauseMutex;

    delete stoppedMutex;
    delete stoppedCheckMutex;
}

void stackManager::runWorker()
{
    rawData * tempRawData;

    unsigned int stackControlCounter = 0;

    forever {
        rawDataStackMutex->lock();

            if(!rawDataStack->empty())
            {
                tempRawData = rawDataStack->first();
                rawDataStack->pop_front();
            }
            else tempRawData = NULL;

        rawDataStackMutex->unlock();

        if(tempRawData!=NULL)
        {
            // here the data processing is placed
            dataProcessing(tempRawData);

            // stack control functionality
            stackControl(&stackControlCounter);
        }
        else
        {
            pauseMutex->lock();
            if(pauseState)
            {
                pause->wait(pauseMutex);
            }
            pauseMutex->unlock();

            stackControlCounter = 0;
            lastStackCount = 0;

            qDebug() << "No data to read. Stack is empty.";

            settingsMutex->lock();
            idleTime = settings->getStackIdleTime();
            settingsMutex->unlock();

            QThread::msleep(idleTime);
        }

        stoppedMutex->lock();
        if(stopped)
        {
            stoppedMutex->unlock();
            qDebug() << "Processing remaining data in stack...";
            rescue(); // process all data in stack if are availible
            break;
        }
        stoppedMutex->unlock();
    }

    qDebug() << "Leaving stack manager worker cycle.";

    // inform higher classes that the loop is finished
    stoppedCheckMutex->lock();
    stoppedCheck = true;
    stoppedCheckMutex->unlock();

}

void stackManager::stackControl(unsigned int * stackControlCounter)
{
    ++(*stackControlCounter);
    settingsMutex->lock();
    stackControlPeriodicity = settings->getStackControlPeriodicity();
    settingsMutex->unlock();
    if(*stackControlCounter>=stackControlPeriodicity)
    {
        *stackControlCounter = 0;
        // check of stack
        int stackCount = rawDataStack->count();
        // compare to last known value
        if(stackCount>((int)(lastStackCount)))
        {
            // rising tendency - stack is being filled too fast
            ++stackWarningCount;
            settingsMutex->lock();
            maxStackWarningCount = settings->getMaxStackWarningCount();
            settingsMutex->unlock();
            if(stackWarningCount>=maxStackWarningCount)
            {
                qDebug() << "The stack is being filled too fast.";

                // if rescue functionality is enabled, reading all data from stack until it us empty again
                settingsMutex->lock();
                rescueEnabled = settings->getStackRescueState();
                settingsMutex->unlock();
                if(rescueEnabled) rescue();
            }
            else
            {
                qDebug() << "Stack control responds with OBSERVING status.";
            }

            lastStackCount = stackCount;
        }
        else
        {
            // falling tendency
            lastStackCount = stackCount;
            if(stackWarningCount>0) --stackWarningCount;
            qDebug() << "Stack control responds with OK status.";
        }
    }
}

void stackManager::rescue(void)
{
    rawDataStackMutex->lock();

        // iterate over the stack until all data are processed
        qDebug() << "Starting the stack rescue function...";
        while(!rawDataStack->empty())
        {
            dataProcessing(rawDataStack->first());
            rawDataStack->pop_front();
        }
        qDebug() << "The stack rescue function is finished...";

    rawDataStackMutex->unlock();

    // return warning count to zero again... Stack is safe for now
    stackWarningCount = 0;
}

void stackManager::switchPauseState()
{
    pauseMutex->lock();
    if(!pauseState)
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

void stackManager::stopWorker()
{
    stoppedMutex->lock();
    stopped = true;
    stoppedMutex->unlock();
}

bool stackManager::checkStoppedStatus()
{
    bool temp;
    stoppedCheckMutex->lock();
    temp = stoppedCheck;
    stoppedCheckMutex->unlock();

    return temp;
}

void stackManager::releaseIfInPauseState()
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

void stackManager::dataProcessing(rawData *data)
{
    qDebug() << "Radar id: " << data->getSyntheticRadarId() << " Targets count: " << data->getSyntheticTargetsCount() << " Time: " << data->getSyntheticTime();
}

