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
}

stackManager::~stackManager()
{
    // destructor, no actions needed so far
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

            ++stackControlCounter;
            settingsMutex->lock();
            stackControlPeriodicity = settings->getStackControlPeriodicity();
            settingsMutex->unlock();
            if(stackControlCounter>=stackControlPeriodicity)
            {
                stackControlCounter = 0;
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
                        // if(rescueEnabled) stackRescue();
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
        else
        {
            stackControlCounter = 0;
            lastStackCount = 0;

            qDebug() << "No data to read. Stack is empty.";

            settingsMutex->lock();
            idleTime = settings->getStackIdleTime();
            settingsMutex->unlock();

            QThread::msleep(idleTime);
        }
    }
}

void stackManager::stackControl()
{

}

void stackManager::dataProcessing(rawData *data)
{
    qDebug() << "Radar id: " << data->getSyntheticRadarId() << " Targets count: " << data->getSyntheticTargetsCount() << " Time: " << data->getSyntheticTime();
}

