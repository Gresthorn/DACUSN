#include "stackmanager.h"

stackManager::stackManager(QVector<rawData *> *raw_data_stack, QMutex *raw_data_stack_mutex, QVector<radar_handler * > * radar_list, QMutex * radar_list_mutex,
                           QList<QPointF * > * visualization_data, QList<QColor * > * visualization_color, QList<radarSubWindow * >  * radar_Sub_Window_List, QMutex * radar_Sub_Window_List_Mutex, QMutex * visualization_data_mutex,
                           uwbSettings *setts, QMutex *settings_mutex)
{
    rawDataStack = raw_data_stack;
    rawDataStackMutex = raw_data_stack_mutex;
    settings = setts;
    settingsMutex = settings_mutex;
    visualizationData = visualization_data;
    radarSubWindowList = radar_Sub_Window_List;
    radarSubWindowListMutex = radar_Sub_Window_List_Mutex;
    visualizationColor = visualization_color;
    visualizationDataMutex = visualization_data_mutex;

    idleTime = 200;
    stackControlPeriodicity = 50;
    maxStackWarningCount = 10;
    lastStackCount = 0;

    processingIterator = currentProcessingSpeed = averageProcessingSpeed = 0;

    rescueEnabled = true;

    pauseState = false;
    pauseMutex = new QMutex;
    pause = new QWaitCondition;

    stopped = false;
    stoppedMutex = new QMutex;

    stoppedCheckMutex = new QMutex;
    stoppedCheck = false;

    radarList = radar_list;
    radarListMutex = radar_list_mutex;

    active_radar_ID = 0;
    active_radar_ID_index = -1;
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

void stackManager::changeActiveRadarId(int id)
{
    if(id<=0)
    {
        // if less or equall to zero, we will consider this as operator radar
        active_radar_ID_index = -1;
        active_radar_ID = 0;
        return;
    }
    else active_radar_ID = id;

    active_radar_ID_index = -1;
    // find index of desired radar in radarList
    radarListMutex->lock();
    for(int i=0; i<radarList->count(); i++)
    {
        if(active_radar_ID==radarList->at(i)->id) active_radar_ID_index = i;
    }
    radarListMutex->unlock();

    qDebug() << "New radar id : " << active_radar_ID << " index : " << active_radar_ID_index;
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

qint64 stackManager::getAverageProcessingSpeed()
{
    // WE CAN USE RADARLIST MUTEX SINCE TIME IS CALCULATED INSIDE ITS PROTECTED AREA
    qint64 val;
    radarListMutex->lock();
    val = averageProcessingSpeed;
    radarListMutex->unlock();
    return val;
}

qint64 stackManager::getCurrentProcessingSpeed()
{
    // WE CAN USE RADARLIST MUTEX SINCE TIME IS CALCULATED INSIDE ITS PROTECTED AREA
    qint64 val;
    radarListMutex->lock();
    val = currentProcessingSpeed;
    radarListMutex->unlock();
    return val;
}

void stackManager::dataProcessing(rawData *data)
{
    if(data==NULL) return;

    reciever_method method = data->getRecieverMethod();
    int radar_id = 0;
    // obtain radar id
    if(method==SYNTHETIC) radar_id = data->getSyntheticRadarId();

    radarListMutex->lock();

    // the timer is used for calculating the average or current processing speed
    QElapsedTimer timer;
    timer.start();

    // find the correct radarUnit
    int i = -1;
    if(!radarList->isEmpty())
    {
        for(i = 0; i<radarList->count(); i++)
        {
            if(radarList->at(i)->id==(unsigned int)(radar_id)) break;
        }
    }

    if(i<0 || i>=radarList->count())
    {
        // need to register new radar
        radar_handler * rd = new radar_handler;
        rd->id = radar_id;
        rd->updated = false;
        rd->radar = new radarUnit(radar_id);
        radarList->append(rd);
        i = radarList->count()-1;
    }

    // in 'i' the correct index should be stored now, we can run
    if(radarList->at(i)->radar->processNewData(data)) radarList->at(i)->updated = true;
    else radarList->at(i)->updated = false;

    // Here another data processing should take place if needed
    // It is supposed that some data fusion from all radarUnits will be placed here

    // Update subWindow vectors if user wants to see specific radar view
    updateRadarSubWindowList();

    if(checkRadarDataUpdateStatus())
    {
        // if all radars are updated its time for fusion and visualization list update
        applyFusion();
    }

    // update time information
    currentProcessingSpeed = timer.nsecsElapsed();
    averageProcessingSpeed = (averageProcessingSpeed*processingIterator + currentProcessingSpeed)/(++processingIterator);

    radarListMutex->unlock();
}

bool stackManager::checkRadarDataUpdateStatus()
{
   // no need to lock mutex because this function is already called when the mutex is locked
   int i;

   for(i=0; i<radarList->count(); i++) if(!radarList->at(i)->updated) return false;

   // now all radars are updated
   return true;
}

void stackManager::applyFusion()
{
    // apply the fusion algorithm (so far only simple averaging the values)
    int i, j;

    // specifies the maximum target count from all radar units
    int maximum_targets_visible = 0;

    // temporary pointer handlers
    QVector<float * > arrays;
    QVector<int> targets_count;

    // for simplicity we suppose that all targets have same indexing in all radarUnits and coordinates are non-zero
    for(i=0; i<radarList->count(); i++)
    {
        arrays.append(radarList->at(i)->radar->getCoordinatesLast());
        targets_count.append(radarList->at(i)->radar->getNumberOfTargetsLast());

        // update maximum targets visible
        if(targets_count.last()>maximum_targets_visible) maximum_targets_visible = targets_count.last();

        // restore updated status back to false so fusion can run again only after all radars has updated data
        radarList->at(i)->updated = false;
    }

    clearVisualizationData();

    // if specific radar is used to be displayed in main/central view, we will push only its values into visualizationData

    if(active_radar_ID_index>=0 && active_radar_ID>0)
    {
        float * radar_coords = radarList->at(active_radar_ID_index)->radar->getCoordinatesLast();
        int targets = radarList->at(active_radar_ID_index)->radar->getNumberOfTargetsLast();
        visualizationDataMutex->lock();
        for(int i=0; i<targets; i++)
        {
            QPointF * temp_point = new QPointF(radar_coords[i*2], radar_coords[i*2+1]);

            visualizationData->append(temp_point);
        }
        visualizationDataMutex->unlock();
    }

    float x_average; // the averaged value from all radars
    float y_average; // the averaged value from all radars
    int counter; // used for averaging;
    if(!arrays.isEmpty() && !targets_count.isEmpty())
    {

        // if data backup is enabled we need to start new record
        settingsMutex->lock();
            if(settings->getDiskBackupEnabled())
            {
                makeDataBackup(QDateTime::currentMSecsSinceEpoch(), true, true);
                makeDataBackup((float)(maximum_targets_visible));
            }
        settingsMutex->unlock();

        // so far only averaging of coordinates is used: no fusion algorithm availible
        // it is also supposed that indexes of coordinates are the same for specific target

        for(j=0; j<maximum_targets_visible; j++)
        {
            x_average = 0.0;
            y_average = 0.0;
            counter = 0;

            // iterating through all possible targets
            for(i=0; i<arrays.count(); i++)
            {    
                //qDebug() << "-------------------------------------------------------------------------------------";
                // if radar was disabled by user, do not use it in fusion algorithm
                if(!radarList->at(i)->radar->isEnabled()) continue;

                // if is less the radar unit surely has information about target's coordinates
                if(j<targets_count.at(i))
                {
                    // apply transformation to operator coordinate system

                    radarList->at(i)->radar->doTransformation(arrays.at(i)[j*2], arrays.at(i)[j*2+1]);

                    // Usually if MTT produces invalid value, the "nan" or "inf/-inf" states were catched. Therefore it is much better to not consider such values
                    float x = radarList->at(i)->radar->getTransformatedX();
                    float y = radarList->at(i)->radar->getTransformatedY();

                    if(x!=x || y!=y) continue;
                    else if(x>std::numeric_limits<float>::max() || x<(-std::numeric_limits<float>::max())
                            || y>std::numeric_limits<float>::max() || y<(-std::numeric_limits<float>::max())) continue;
                    else if(qFuzzyCompare(0.0, x) || qFuzzyCompare(0.0, y)) continue;

                    x_average += x;
                    y_average += y;
                    //qDebug() << radarList->at(i)->radar->getTransformatedX() << " " << radarList->at(i)->radar->getTransformatedY();

                    ++counter;
                }
            }

            // calculate average value
            x_average /= (float)(counter);
            y_average /= (float)(counter);

            settingsMutex->lock();
                if(settings->getDiskBackupEnabled())
                {
                    makeDataBackup(x_average);
                    makeDataBackup(y_average);
                }
            settingsMutex->unlock();

            // append to visualization vector if counter is more than one (at least one radar had value)
            // if active_radar_ID is not less or equal to zero, another data, from another radar are desired to be seen
            if(active_radar_ID_index<0 || active_radar_ID<=0) if(counter>=1)
            {
                QPointF * temp_point = new QPointF(x_average, y_average);

                visualizationDataMutex->lock();
                visualizationData->append(temp_point);
                visualizationDataMutex->unlock();
            }
        }

        // if data backup is enabled we need end record by adding end-of-line sign
        settingsMutex->lock();
            if(settings->getDiskBackupEnabled()) makeDataBackup((float)(0.0), false, false, true);
        settingsMutex->unlock();
    }

    visualizationDataMutex->lock();
    qDebug() << visualizationData->count();
    visualizationDataMutex->unlock();
}

void stackManager::clearVisualizationData()
{
    visualizationDataMutex->lock();
    while(!visualizationData->isEmpty())
    {
        delete visualizationData->first();
        visualizationData->removeFirst();
    }
    visualizationDataMutex->unlock();
}

void stackManager::updateRadarSubWindowList()
{
    // NOTE THAT WE DO NOT NEED TO LOCK RADARLIST MUTEX SINCE THIS FUNCTION IS CALLED IN FUSION FUNCTION OR IN AREA ALREADY PROTECTED BY RADARLIST MUTEXES

    // iterate over all subwindows
    int id, targets_number;
    float * coordinates;
    radarSubWindowListMutex->lock();

    if(radarSubWindowList->isEmpty()) { radarSubWindowListMutex->unlock(); return; }

    for(int a = 0; a<radarSubWindowList->count(); a++)
    {
        id = radarSubWindowList->at(a)->getRadarId();
        // find radar unit
        for(int b = 0; b<radarList->count(); b++)
        {

            if(id==radarList->at(b)->id)
            {
                // update points vector with new values
                coordinates = radarList->at(b)->radar->getCoordinatesLast();
                targets_number = radarList->at(b)->radar->getNumberOfTargetsLast();

                // if corrupted, or unavailible data
                if(targets_number<0 || coordinates==NULL) break;

                radarSubWindowList->at(a)->addVisualizationData(coordinates, targets_number);
            }
        }
    }
    radarSubWindowListMutex->unlock();
}

void stackManager::makeDataBackup(float val, bool write_val, bool newline, bool endline)
{
    // MUTEX IS NOT CALLED HERE SINCE IT IS SUPPOSED THAT MUTEX WAS ALREADY CALLED IN PARENT FUNCTION
    // if data backup is enabled we need to write all recieved coordinates

    if(!newline) (*settings->getBackupFileHandler()) << "%";

    if(write_val) (*settings->getBackupFileHandler()) << val;

    if(endline) (*settings->getBackupFileHandler()) << "\n";
}

void stackManager::makeDataBackup(QString val, bool write_val, bool newline, bool endline)
{
    // MUTEX IS NOT CALLED HERE SINCE IT IS SUPPOSED THAT MUTEX WAS ALREADY CALLED IN PARENT FUNCTION
    // if data backup is enabled we need to write all recieved coordinates

    if(!newline) (*settings->getBackupFileHandler()) << "%";

    if(write_val) (*settings->getBackupFileHandler()) << val;

    if(endline) (*settings->getBackupFileHandler()) << "\n";
}

void stackManager::makeDataBackup(qint64 val, bool write_val, bool newline, bool endline)
{
    // MUTEX IS NOT CALLED HERE SINCE IT IS SUPPOSED THAT MUTEX WAS ALREADY CALLED IN PARENT FUNCTION
    // if data backup is enabled we need to write all recieved coordinates

    if(!newline) (*settings->getBackupFileHandler()) << "%";

    if(write_val) (*settings->getBackupFileHandler()) << val;

    if(endline) (*settings->getBackupFileHandler()) << "\n";
}

