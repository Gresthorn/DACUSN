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

bool stackManager::coordinatesAreValid(float x, float y)
{
    if(x!=x || y!=y) return false;
    else if(x>std::numeric_limits<float>::max() || x<(-std::numeric_limits<float>::max())
            || y>std::numeric_limits<float>::max() || y<(-std::numeric_limits<float>::max())) return false;
    else if(qFuzzyCompare(0.0, y)) return false; // y cannot be zero

    return true;
}

void stackManager::dataProcessing(rawData *data)
{
    if(data==NULL) return;

    reciever_method method = data->getRecieverMethod();
    int radar_id = 0; // NEVER USE ID 0 SINCE IT IS RESERVED FOR OPERATOR
    // obtain radar id
    if(method==SYNTHETIC) radar_id = data->getSyntheticRadarId();
    else if(method==RS232) radar_id = data->getUwbPacketRadarId();

    if(radar_id<1)
    {
        // ID 0 is reserved for operator. Is such packet occures, it is not valid and is dropped.
        delete data;
        return;
    }

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
    bool localMTTEnabled = false;
    settingsMutex->lock();
    localMTTEnabled = settings->getSingleRadarMTT();
    settingsMutex->unlock();
    if(radarList->at(i)->radar->processNewData(data, localMTTEnabled)) radarList->at(i)->updated = true;
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
    // apply the fusion algorithm (so far simple averaging the values or global MTT)
    int i, j;

    // specifies the maximum target count from all radar units
    int maximum_targets_visible = 0;
    // if global MTT is used, we need to calculate array global array size (all coordinates in one array)
    int global_mtt_array_size = 0;

    // temporary pointer handlers
    QVector<float * > arrays;
    QVector<int> targets_count;

    bool enableGlobalMTT = false;
    settingsMutex->lock();
    enableGlobalMTT = settings->getGlobalRadarMTT();
    settingsMutex->unlock();

    // for simplicity we suppose that all targets have same indexing in all radarUnits and coordinates are non-zero
    for(i=0; i<radarList->count(); i++)
    {
        arrays.append(radarList->at(i)->radar->getCoordinatesLast());
        targets_count.append(radarList->at(i)->radar->getNumberOfTargetsLast());

        // if global MTT is enabled and radar unit is enabled as well, we can add to array size the number of coordinates.
        // In result, global_mtt_array_size will contain the size of array only for coordinates of active radar units.
        if(enableGlobalMTT && radarList->at(i)->radar->isEnabled()) global_mtt_array_size+=(radarList->at(i)->radar->getNumberOfTargetsLast()*2);

        // update maximum targets visible, this is usable only if averaging is selected instead of global MTT. If global MTT is set, no need of storing this information.
        if(targets_count.last()>maximum_targets_visible && !enableGlobalMTT) maximum_targets_visible = targets_count.last();

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


        if(enableGlobalMTT)
        {
            // If global MTT is enabled, create one array of all coordinates from all radars and pass newly created array to MTT
            #if defined MTT_ARRAY_FIT && MTT_ARRAY_FIT==1
                float * global_mtt_array = new float[MAX_N*2];
                for(int i=0; i<(MAX_N*2); i++) global_mtt_array[i] = 0.0; // zero all positions since not all can be used for data
            #else
                float * global_mtt_array = new float[global_mtt_array_size];
            #endif
            int global_mtt_array_pointer = 0; // still points to new empty array space

            //qDebug() << "STARTING FILLING ARRAY " << global_mtt_array_size;

            // fill array with coordinates
            //qDebug() << "Total radars : " << arrays.count();
            for(j=0; j<arrays.count(); j++)
            {
                if(!radarList->at(j)->radar->isEnabled()) continue; // do not copy coordinates from radar unit that is not allowed by user

                //qDebug() << "Running new filling of radar id : " << j << " while targets count is " << targets_count.at(j);

                for(i=0; i<targets_count.at(j); i++)
                {
                    //qDebug() << "FILLING RADAR " << j << " ITERATION " << i;
                    // apply transformation to operator coordinate system

                    radarList->at(j)->radar->doTransformation(arrays.at(j)[i*2], arrays.at(j)[i*2+1]);

                    // Usually if MTT produces invalid value, the "nan" or "inf/-inf" states were catched. Therefore it is much better to not consider such values
                    float x = radarList->at(j)->radar->getTransformatedX();
                    float y = radarList->at(j)->radar->getTransformatedY();

                    qDebug() << "X: " << x << " Y: " << y;

                    global_mtt_array[global_mtt_array_pointer++] = x;
                    global_mtt_array[global_mtt_array_pointer++] = y;
                }
            }

            //qDebug() << "FILLING FINISHED";

            // create MTT constants
            float r[] = {0.1, 0.01};
            float q[] = {0.0, 0.01, 0.0, 0.0001};
            float diff_d = 1.0;
            float diff_fi = 0.6;
            int min_NT = 10;
            int min_OLGI = 10;

            //this->MTT(global_mtt_array, r, q, diff_d, diff_fi, min_OLGI, min_NT);

            float * global_mtt_array_result = global_mtt_array; // Result array of target coordinates from global MTT algorithm.

            int global_mtt_array_result_pointer = global_mtt_array_pointer; // Holds the information about, how many numbers there are in array.

            // Backup data on disk, if backup is enabled
            bool backupEnabled = false;
            settingsMutex->lock();
                backupEnabled = settings->getDiskBackupEnabled();
            settingsMutex->unlock();
            if(backupEnabled)
            {
                for(j=0; j<(global_mtt_array_result_pointer/2); j++)
                {
                    // check if values are not NaN or -+ infinite. Also if y-coordinate is zero, coordinates are not valid
                    // Therefore also filtration of positions zeroed by MTT is done.
                    if(coordinatesAreValid(global_mtt_array_result[j*2], global_mtt_array_result[j*2+1]))
                    {
                        makeDataBackup(global_mtt_array_result[j*2]);
                        makeDataBackup(global_mtt_array_result[j*2+1]);
                    }
                }
            }

            // Append data to visualizationData list.
            // if active_radar_ID is not less or equal to zero, another data, from another radar are desired to be seen
            if(active_radar_ID_index<0 || active_radar_ID<=0)
            {
                for(j=0; j<(global_mtt_array_result_pointer/2); j++)
                {

                    // check if values are not NaN or -+ infinite. Also if y-coordinate is zero, coordinates are not valid
                    // Therefore also filtration of positions zeroed by MTT is done.
                    if(coordinatesAreValid(global_mtt_array_result[j*2], global_mtt_array_result[j*2+1]))
                    {
                        QPointF * temp_point = new QPointF(global_mtt_array_result[j*2], global_mtt_array_result[j*2+1]);

                        visualizationDataMutex->lock();
                        visualizationData->append(temp_point);
                        visualizationDataMutex->unlock();
                    }
                }
            }


            // Do not forget to delete common array
            delete[] global_mtt_array;

            qDebug() << "SEQUENCE FINISHED";
        }
        else
        {

            // The following cycle will do averaging of coordinates: no fusion algorithm availible
            // it is also supposed that indexes of coordinates are the same for specific target

            for(j=0; j<maximum_targets_visible; j++)
            {
                x_average = 0.0;
                y_average = 0.0;
                counter = 0;

                // iterating through all possible targets
                for(i=0; i<arrays.count(); i++)
                {
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

                        if(!coordinatesAreValid(x, y)) continue;

                        x_average += x;
                        y_average += y;

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
        }

        // if data backup is enabled we need end record by adding end-of-line sign
        settingsMutex->lock();
            if(settings->getDiskBackupEnabled()) makeDataBackup((float)(0.0), false, false, true);
        settingsMutex->unlock();
    }

    visualizationDataMutex->lock();
    qDebug() << "DATA COUNT " << visualizationData->count();
    visualizationDataMutex->unlock();
}

void stackManager::clearVisualizationData()
{
    visualizationDataMutex->lock();
    while(!visualizationData->isEmpty())
    {
        delete[] visualizationData->first();
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


/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/**************************************** DATA PROCESSING FUNCTIONS *************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/


/************ Supporting "matrixr" & Matlab  functions *********************/
void stackManager::matrix_mul_4x2( real c[][2], real a[][2], real b[][2]  ) {
word k,j;

     for( k=0; k<4; k++ ) {
        for( j=0; j<2; j++ ) {
             c[k][j] = (real) a[k][0]*b[0][j] + a[k][1]*b[1][j];
        }
     }
}

void stackManager::matrix_mul_4x4( real c[][4], real a[][4], real b[][4]  ) {
word k,j;

     for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
                       c[k][j] = a[k][0]*b[0][j] + a[k][1]*b[1][j] + a[k][2]*b[2][j] + a[k][3]*b[3][j];
        }
     }
}

void stackManager::matrix_transpose_4x4( real out[][4], real in[][4]  ) {
word k,j;

     for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
             out[j][k] = in[k][j];
        }
     }

}

void stackManager::matrix_add_4x4( real c[][4], real a[][4], real b[][4] ) {
word k,j;

     for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
             c[k][j] = a[k][j] + b[k][j];
        }
     }
}

/* 4x4 matrices (P_e_p[][4], real P_p[][4]) are reffered as 16x1 vectors as 3D arrays are used in MTT function */

/* code can be simplified ... */

void stackManager::prediction ( real Y_e_p[], real *P_e_p, real Q[][4], real Y_p[], real *P_p ) {
real f, T;
real A[4][4];
real TMP[4][4], A_TRAN[4][4];

word k, j;
real _P_e_p[4][4], _P_p[4][4];

/* copy row vectors (parts of 3D arrays) to the 4x4 matrices */

    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            _P_e_p[k][j] = P_e_p[4*k+j];
            _P_p[k][j] = P_p[4*k+j];
        }
    }

    f = (real) FREQUENCY;
    T = SCANLENGHT/f;
    T *= uS*HA*SA;


/*
A = [1 T 0 0;
     0 1 0 0;
     0 0 1 T;
     0 0 0 1]; % state transition matrix

Y_p = A*Y_e_p;
*/
     Y_p[0] = Y_e_p[0] + T* Y_e_p[1];
     Y_p[1] = Y_e_p[1];
     Y_p[2] = Y_e_p[2] + T* Y_e_p[3];
     Y_p[3] = Y_e_p[3];
/*
P_p = A*P_e_p*A' + Q;
*/
     A[0][2] = A[0][3] = 0.0;
     A[1][0] = A[1][2] = A[1][3] = 0.0;
     A[2][0] = A[2][1] = 0.0;
     A[3][0] = A[3][1] = A[3][2] = 0.0;
     A[0][0] = A[1][1] = A[2][2] = A[3][3] = 1.0;
     A[0][1] = A[2][3] = T;

     matrix_mul_4x4( TMP, A, _P_e_p );
     matrix_transpose_4x4( A_TRAN, A );
     matrix_mul_4x4( _P_p, TMP, A_TRAN );
     matrix_add_4x4( _P_p, _P_p, Q );

/* copy results back to the row vector (part of 3D array) */
    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            P_p[4*k+j] = _P_p[k][j];
        }
    }

}

/* P_e is stored as [4x4] matrix!!! */
void stackManager::init_estimation (real r, real fi, real Y_e_2_init, real Y_e_4_init, real P_init[][4], real Y_e[], real *P_e ) {
word k, j;

    Y_e[0] = r;
    Y_e[1] =  Y_e_2_init;
    Y_e[2] = fi;
    Y_e[3] = Y_e_4_init;

    for( k=0; k<4; k++ )
        for( j=0; j<4; j++ )
                P_e[4*k+j] = P_init[k][j];

}


void stackManager::cartesian2polar( real x, real y, real *r, real *fi) {
   *r = 0;
   *fi = 0;

   *r = (real) sqrt(x*x + y*y);
   if (*r == 0.0)
        return;

   if (*r == 0.0) {
        return;
   }
   else {
        if( y > 0)
           *fi = (real) acos(x / *r);
        else
           *fi = (real) -1.0 * (real) acos(x / *r);
   }
}

/* 4x4 matrix (P_p[][4]) are reffered as 16x1 vectors as 3D arrays are used in MTT function */

void stackManager::gate_checker ( real r, real fi, real Y_p[], real R[][2], real *P_p, word *m, real *c) {
    word  k, j;
    real  c0, c1, res_d, res_fi, temp;
    real sigma_d, sigma_fi;
    real _P_p[4][4];

    /* copy row vector (part of 3D arrays) to the 4x4 matrix */

    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            _P_p[k][j] = P_p[4*k+j];
        }
    }

    /*
    k = 3; % from "3 sigma rule"
    m = 0;
    c = 10^2; % prednastavena lubovolne velka hodnota
    */

     k = 3;
     *m = 0;
     *c = LARGE_NUMBER;

    /*
    res_d = abs(Z(1) - Y_p(1,1));
    res_fi = abs(Z(2) - Y_p(3,1));
    */
     res_d = (real) fabs( r - Y_p[0] );
     res_fi = (real) fabs( fi - Y_p[2] );

    /*
    sigma_d = sqrt(R(1,1) + P_p(2,2));
    sigma_fi = sqrt(R(2,2) + P_p(4,4));
    */
     sigma_d = (real) sqrt( R[0][0] + _P_p[1][1] );
     sigma_fi = (real) sqrt( R[1][1] + _P_p[3][3] );


    /*
    if res_d <= k*sigma_d && res_fi <= k*sigma_fi
        m = 1;
        c = ([res_d res_fi]*[P_p(3,3)+R(2,2) -P_p(1,3); -P_p(3,1) P_p(1,1)+R(1,1)]*[res_d res_fi]')...
            /((P_p(1,1)+R(1,1))*(P_p(3,3)+R(2,2))-P_p(1,1)*P_p(3,3));
    end
    */
     if( (res_d <= k*sigma_d) && (res_fi <= k*sigma_fi) ) {
          *m = 1;
          temp = (_P_p[0][0]+R[0][0])*(_P_p[2][2]+R[1][1])-_P_p[0][0]*_P_p[2][2];
                  if ( fabs(temp) > FLT_MIN ) {                             // MD addiional protection
              c0 = res_d*( _P_p[2][2]+R[1][1]) - res_fi*_P_p[2][0];
              c1 = res_fi*( _P_p[0][0]+R[0][0]) - res_d* _P_p[0][2];
              *c = (c0*res_d + c1*res_fi)/temp;
          }
     }
}

/* 4x4 matrices (P_e_last[][4], P_e[][4] ) are reffered as 16x1 vectors as 3D arrays are used in MTT function */
void stackManager::obs_less_gate_ident (word *OLGI_last,word track, word nn_track, real Y_e_last[], real *P_e_last,
                          real Y_e[], real *P_e, word *OLGI) {

real _P_e_last[4][4], _P_e[4][4];
    word k, j;

    /* copy row vector (part of 3D arrays) to the 4x4 matrix */
   for( k=0; k<4; k++ ) {
       for( j=0; j<4; j++ ) {
             _P_e_last[k][j] = P_e_last[4*k+j];
       }
   }

   for (k=0; k<4; k++ ) {
        Y_e[k] = 0.0;
        for ( j=0; j<4; j++ ) {
            _P_e[k][j] = 0.0;
        }
   }

   *OLGI = 0;

   if (track != nn_track ) {
        for (k=0; k<4; k++ ) {
            Y_e[k] = Y_e_last[k];
            for ( j=0; j<4; j++ ) {
                _P_e[k][j] = _P_e_last[k][j];
            }
        }
        *OLGI = *OLGI_last;
   }

   for (k=0; k<4; k++ ) {
        Y_e_last[k] = 0.0;
        for ( j=0; j<4; j++ ) {
            _P_e_last[k][j] = 0.0;
        }
   }
   *OLGI_last = 0;

    /* copy results back to the row vector (part of 3D array) */
    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            P_e[4*k+j] = _P_e[k][j];
            P_e_last[4*k+j] = _P_e_last[k][j];
        }
    }
}

void stackManager::zeros( real in[][MAX_N], word m, word n ) {
    word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                in[i][j] = 0.0;

}

void stackManager::log_negate( real in[][MAX_N], word m, word n, word out[][MAX_N]  ) {
    word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                if( in[i][j] == 0)
                      out[i][j] = TRUE;
                else
                      out[i][j] = FALSE;
}

void stackManager::falses( word in[][MAX_N], word m, word n ) {
    word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                in[i][j] = FALSE;

}

void stackManager::falses_vector( word in[], word n ) {
    word i;
    for( i=0; i<n; i++ )
                in[i] = FALSE;
}


word stackManager::any( word in[][MAX_N], word m, word n ) {
    word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                     if (in[i][j])
                             return TRUE;
    return FALSE;
}

word stackManager::any_vector( word in[], word n ) {
    word i;
    for( i=0; i<n; i++ )
        if (in[i])
                return TRUE;
    return FALSE;
}

word stackManager::any_not_selected ( word in[][MAX_N], word row_sel[], word col_sel[], word m, word n ) {
    word i,j;
    for( i=0; i<m; i++ )
        if( !row_sel[i] ) {
                for( j=0; j<n; j++ ) {
                        if( !col_sel[j] )
                                if (in[i][j])
                                        return TRUE;
                }
        }
    return FALSE;
}

word stackManager::any_vector_not_selected ( word in[], word m ) {
    word i;
    for( i=0; i<m; i++ ) {
        if( !in[i] )
            return TRUE;
    }
    return FALSE;
}

void stackManager::any_col( word in[][MAX_N], word m, word n, word out[]  ) {
    word i,j;
    for( j=0; j<n; j++ ) {
        out[j] = FALSE;
        for( i=0; i<m; i++ )
                if (in[i][j])
                        out[j] = TRUE;
    }
}

void stackManager::find( word in[][MAX_N], word m, word n, word *r, word *s ) {
    word i,j;
    *r = -1;
    *s = -1;
    for( j=0; j<n; j++ )        // checks complete columns
                for( i=0; i<m; i++ )
                      if (in[i][j]) {
                           *r = i;
                           *s = j;
                            return;
                      }
    return;
}


word stackManager::find_vec( word in[], word n ) {
    word i;
    for( i=0; i<n; i++ )        // checks complete vector
         if (in[i])
              return i;
    return -1;
}

void stackManager::copy_row ( word in[][MAX_N], word m, word n, word out[] ) {
    word i;
    for (i=0; i<n; i++) {
        out[i] = in[m][i];
    }
}

void stackManager::copy_col ( word in[][MAX_N], word m, word n, word out[] ) {
    word i;
    for (i=0; i<m; i++) {
        out[i] = in[i][n];
    }
}

real stackManager::find_min ( real in[][MAX_N], word m, word n ) {
    word i, j;
    real val = FLT_MAX;
    for (i=0; i<m; i++) {
        for(j=0; j<n; j++ )
             if( in[i][j] < val )
                  val =  in[i][j];
    }

    return val;
}

word stackManager::find_max ( word in[][MAX_N], word row ) {
    word k;
    word val = INT_MIN;
    for (k=0; k<MAX_N; k++) {
         if( in[row][k] > val )
                  val =  in[row][k];
    }
    return val;
}

void stackManager::extract_valid( real in[][MAX_N], word row_sel[], word col_sel[], real out[][MAX_N], word m, word n) {
    word i,j, i_out, j_out;
    i_out = 0;
    for(i=0; i<m; i++ ) {
       if( row_sel[i] ) {                // process only valid rows
            j_out = 0;
            for(j=0; j<n; j++) {         // check all cols
                if( col_sel[j] ) {       // only valid cols
                    out[i_out][j_out] = in[i][j];
                }
                j_out++;
            }
            i_out++;
       }
    }
}

/*
M=dMat(~coverRow,~coverColumn)
*/
void stackManager::extract_not_valid( real in[][MAX_N], word row_sel[], word col_sel[], word m, word n, real out[][MAX_N], word *r, word *c) {
    word i,j, i_out, j_out;

    j_out = 0;
    for(j=0; j<n; j++ ) {
        if( !col_sel[j] ) {              // process only non valid cols
            i_out = 0;
            for(i=0; i<n; i++) {         // check all rows
                if( !row_sel[i] ) {      // only non valid rows
                    out[i_out][j_out] = in[i][j];
                    i_out++;
                }
            }
            j_out++;
       }
    }
    *r = i_out;
    *c = j_out;
}

/*
        dMat(~coverRow,~coverColumn)=M-minval;
*/
void stackManager::insert_to_not_valid( real in[][MAX_N], word m, word n, real out[][MAX_N], word row_sel[], word col_sel[] ) {
    word i,j, i_in, j_in;
    i_in = 0; i=0;
        while( i_in <m ) {
            if( !row_sel[i] ) {        // process only non valid rows
                 j=0; j_in = 0;
                 while( j_in <n ) {
                         if( !col_sel[j] ) {
                                out[i][j] = in[i_in][j_in];
                                j_in++;
                         }
                         j++;
                 }
                 i_in++;
            }
            i++;
        }
}




/*
        assignment(validRow,validCol) = starZ(1:nRows,1:nCols);
*/
void stackManager::insert_to_valid( word in[][MAX_N], word m, word n, word out[][MAX_N], word row_sel[], word col_sel[] ) {
    word i,j, i_in, j_in;
    i_in = 0; i = 0;
        while( i_in <m ) {
            if( row_sel[i] ) {
                  j=0; j_in = 0;
                  while( j_in <n ) {
                          if( col_sel[j] ) {
                             out[i][j] = in[i_in][j_in];
                             j_in++;
                          }
                          j++;
                  }
                  i_in++;
            }
            i++;
        }
}




void stackManager::munkres( real costMat[][MAX_N], word rows, word cols, real *cost, word assign[][MAX_N] ) {

    word validCol[ MAX_N ];
    word validRow[ MAX_N ];
    real dMat[ MAX_N ][ MAX_N ], minval;
    real M[ MAX_N ][ MAX_N ];
    word M_row, M_col;
    real min_row;

    word zP[ MAX_N ][ MAX_N ];
    word starZ[ MAX_N ][ MAX_N ];
    word primeZ[ MAX_N ][ MAX_N ];
    word coverColumn[ MAX_N ];
    word coverRow[ MAX_N ];
    word stz[ MAX_N ];
    word rowZ1[ MAX_N ];

    word nRows, nCols, j, k, n, r, c, uZr, uZc, Step;

    /*
    nRows = sum(validRow);
    nCols = sum(validCol);
    n = max(nRows,nCols);
    if ~n
        return
    end
    */
   nRows = rows;
   nCols = cols;

   n = MAX( nRows, nCols);
   if (n==0)
        return;

    /*
    validCol = any(validMat);
    */
   for( k=0; k<nRows; k++ ) {
        validRow[k] = FALSE;
        for( j=0; j<nCols; j++ ) {
             if( costMat[k][j] != 0 ) {
                  validRow[k] = TRUE;       // can braek here
             }
        }
   }
    /*
    validRow = any(validMat,2);
    */
   for( j=0; j<nCols; j++ ) {
        validCol[j] = FALSE;
        for( k=0; k<nRows; k++ ) {
             if( costMat[k][j] != 0 ) {
                  validCol[j] = TRUE;       // can braek here
             }
        }
   }

    /*
    assignment = false(size(costMat));
    cost = 0;
    */
   for( j=0; j<nRows; j++ ) {
        for( k=0; k<nCols; k++ ) {
            assign[j][k] = FALSE;
        }
   }
   *cost = 0;


    /*
    dMat = zeros(n);
    */
   zeros( dMat, n, n );

    /*
    dMat(1:nRows,1:nCols) = costMat(validRow,validCol);
    */

   extract_valid( costMat, validRow, validCol, dMat, nRows, nCols);

    #if(0)
    %*************************************************
    % Munkres' Assignment Algorithm starts here
    %*************************************************

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %   STEP 1: Subtract the row minimum from each row.
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
     dMat = bsxfun(@minus, dMat, min(dMat,[],2));
    #endif

   for( j=0; j<n; j++ ) {
        min_row = FLT_MAX;
        for( k=0; k<n; k++ ) {
            if (min_row > dMat[j][k])
                min_row = dMat[j][k];
        }
        for( k=0; k<n; k++ ) {
            dMat[j][k] -= min_row;
        }

   }

    /*
    %**************************************************************************
    %   STEP 2: Find a zero of dMat. If there are no starred zeros in its
    %           column or row start the zero. Repeat for each zero
    %**************************************************************************
    zP = ~dMat;
    starZ = false(n);
    while any(zP(:))
        [r,c]=find(zP,1);
        starZ(r,c)=true;
        zP(r,:)=false;
        zP(:,c)=false;
    end
    */
    log_negate( dMat, n, n, zP );
    falses( starZ, n, n );
    while( any( zP, n, n) ) {
        find( zP, n, n, &r, &c );
        starZ[r][c] = TRUE;
        for(j=0; j<n; j++)
                zP[r][j] = FALSE;
        for(j=0; j<n; j++)
                zP[j][c] = FALSE;
    }

    /*
    while 1
    %**************************************************************************
    %   STEP 3: Cover each column with a starred zero. If all the columns are
    %           covered then the matching is maximum
    %**************************************************************************
        primeZ = false(n);
        coverColumn = any(starZ);
        if ~any(~coverColumn)
            break
        end
        coverRow = false(n,1);
    */

    while( 1 ) {
    falses( primeZ, n, n );
    any_col( starZ, n, n, coverColumn );
    if( !any_vector_not_selected ( coverColumn, n ) )
        break;
    falses_vector( coverRow, n );

    /*
        while 1
            %**************************************************************************
            %   STEP 4: Find a noncovered zero and prime it.  If there is no starred
            %           zero in the row containing this primed zero, Go to Step 5.
            %           Otherwise, cover this row and uncover the column containing
            %           the starred zero. Continue in this manner until there are no
            %           uncovered zeros left. Save the smallest uncovered value and
            %           Go to Step 6.
            %**************************************************************************
            zP(:) = false;
            zP(~coverRow,~coverColumn) = ~dMat(~coverRow,~coverColumn);
            Step = 6;
    */
    while( 1 ) {
        falses( zP, n, n);
        for( j=0; j<n; j++ ) {
           if ( !coverRow[j] ) {
                for( k=0; k<n; k++ ) {
                   if( !coverColumn[k] )
                        if (dMat[j][k] == 0)
                        zP[j][k] = TRUE;
                }
           }
        }
        Step = 6;
    /*
        while any(any(zP(~coverRow,~coverColumn)))
            [uZr,uZc] = find(zP,1);
            primeZ(uZr,uZc) = true;
            stz = starZ(uZr,:);
            if ~any(stz)
                Step = 5;
                break;
            end
            coverRow(uZr) = true;
            coverColumn(stz) = false;
            zP(uZr,:) = false;
            zP(~coverRow,stz) = ~dMat(~coverRow,stz);
        end
    */

        while( any_not_selected ( zP, coverRow, coverColumn, n, n ) ) {
            find( zP, n, n, &uZr, &uZc );
            primeZ[uZr][uZc] = TRUE;
            copy_row( starZ, uZr, n, stz );
            if (!any_vector( stz,n ) ) {
                Step = 5;
                break;
            }
            coverRow[uZr] = TRUE;

            for( j=0; j<n; j++ )
                if( stz[j] )
                    coverColumn[ j ] = FALSE;

            for( j=0; j<n; j++ )
                 zP[uZr][j] = FALSE;

            for( j=0; j<n; j++ ) {
                if ( !coverRow[j] ) {
                   for( k=0; k<n; k++ ) {
                       if( stz[k] )
                           zP[j][k] = !dMat[j][k];
                   }
                }
            }

        }

    /*
        if Step == 6
            % *************************************************************************
            % STEP 6: Add the minimum uncovered value to every element of each covered
            %         row, and subtract it from every element of each uncovered column.
            %         Return to Step 4 without altering any stars, primes, or covered lines.
            %**************************************************************************
            M=dMat(~coverRow,~coverColumn);
            minval=min(min(M));
            if minval==inf
                return
            end
            dMat(coverRow,coverColumn)=dMat(coverRow,coverColumn)+minval;
            dMat(~coverRow,~coverColumn)=M-minval;
        else
            break
        end
    end
    */
        if (Step == 6) {

             extract_not_valid( dMat, coverRow, coverColumn, n, n, M, &M_row, &M_col );
             minval = find_min( M, M_row, M_col);

             for( j=0; j<n; j++ ) {
                if ( coverRow[j] ) {
                       for( k=0; k<n; k++ ) {
                          if( coverColumn[k] )
                                  dMat[j][k] += minval;
                          }
                }
             }

             for( j=0; j<M_row; j++ ) {
                       for( k=0; k<M_col; k++ ) {
                                   M[j][k] -= minval;
                       }
             }

             insert_to_not_valid( M, M_row, M_col, dMat, coverRow, coverColumn );
        }
        else
            break;
    }
    /*
    %**************************************************************************
    % STEP 5:
    %  Construct a series of alternating primed and starred zeros as
    %  follows:
    %  Let Z0 represent the uncovered primed zero found in Step 4.
    %  Let Z1 denote the starred zero in the column of Z0 (if any).
    %  Let Z2 denote the primed zero in the row of Z1 (there will always
    %  be one).  Continue until the series terminates at a primed zero
    %  that has no starred zero in its column.  Unstar each starred
    %  zero of the series, star each primed zero of the series, erase
    %  all primes and uncover every line in the matrix.  Return to Step 3.
    %**************************************************************************
    rowZ1 = starZ(:,uZc);
    starZ(uZr,uZc)=true;
    while any(rowZ1)
        starZ(rowZ1,uZc)=false;
        uZc = primeZ(rowZ1,:);
        uZr = rowZ1;
        rowZ1 = starZ(:,uZc);
        starZ(uZr,uZc)=true;
    end

    }
    */

     copy_col ( starZ, n, uZc, rowZ1 );
     starZ[uZr][uZc] = TRUE;

     while( any_vector( rowZ1, n) ) {
                 for( j=0; j<n; j++ ) {
                         if ( rowZ1[j] ) {
                             starZ[j][uZc] = FALSE;
                         }
                 }
                 for( j=0; j<n; j++ ) {
                        if ( rowZ1[j] ) {
                           for( k=0; k<n; k++ ) {
                                if(primeZ[j][k])
                                uZc = k;
                           }
                        }
                 }

                 uZr = find_vec( rowZ1, n );
                 copy_col ( starZ, n, uZc, rowZ1 );
                 starZ[uZr][uZc] = TRUE;
     }
    /*
    end
    */
    }

    /*
    % Cost of assignment
    assignment(validRow,validCol) = starZ(1:nRows,1:nCols);
    cost = sum(costMat(assignment));
    */
     insert_to_valid( starZ, nRows, nCols, assign, validRow, validCol );
     *cost = 0;

     for( j=0; j<n; j++ ) {
               for( k=0; k<n; k++ ) {
                      if( assign[j][k] ) {
                              *cost += costMat[j][k];
                      }
               }
     }

}

/* 4x4 matrices (P_e[][4], real P_p[][4]) are reffered as 16x1 vectors as 3D arrays are used in MTT function */
void stackManager::correction ( real Y_e[], real *P_e, real Y_p[], real *P_p, real r, real fi, real R[][2]) {

    word k,j;
    real _P_e[4][4], _P_p[4][4];

    word t;

    real K[4][2];   // Kalman gain matrix
    real A[2][2];
    real A_inv[2][2];
    real P_p_H[4][2];
    real H_Y_p[2];
    real EYE[4][4];
    real tmp;

    /* copy row vectors (parts of 3D arrays) to the 4x4 matrices */
     for( k=0; k<4; k++ ) {
         for( j=0; j<4; j++ ) {
            _P_e[k][j] = P_e[4*k+j];
            _P_p[k][j] = P_p[4*k+j];
         }
     }

    /*
    Y_e = zeros(4,1);
    P_e = zeros(4,4);
    */
     for( t=0; t<4; t++ ) {
         Y_e[t] = 0.0;
         for( j=0; j<4; j++ ) {
            _P_e[t][j] = 0.0;
            EYE[t][j] = 0.0;
         }
     }
     for( t=0; t<4; t++ )
        EYE[t][t] = 1.0;

    /*
    K = zeros(4,2); % Kalman gain matrix

    */
     for( t=0; t<4; t++ ) {
         Y_e[t] = 0.0;
         for( j=0; j<2; j++ ) {
            K[t][j] = 0.0;
         }
     }

    /*
    A = H*P_p*H' + R; % matica 2x2
    */
     A[0][0] = _P_p[0][0] + R[0][0];
     A[0][1] = _P_p[0][2] + R[0][1];
     A[1][0] = _P_p[2][0] + R[1][0];
     A[1][1] = _P_p[2][2] + R[1][1];

    /*
    A_inv = [A(2,2) -A(1,2);-A(2,1) A(1,1)]/(A(1,1)*A(2,2)- A(1,2)*A(2,1));
    */
     tmp = A[0][0]*A[1][1] - A[0][1]*A[1][0];
     A_inv[0][0] = A[1][1]/tmp;
     A_inv[0][1] = -A[0][1]/tmp;
     A_inv[1][0] = -A[1][0]/tmp;
     A_inv[1][1] = A[0][0]/tmp;

    /*
    K = P_p*H'*A_inv;
    */
     P_p_H[0][0] = _P_p[0][0];
     P_p_H[1][0] = _P_p[1][0];
     P_p_H[2][0] = _P_p[2][0];
     P_p_H[3][0] = _P_p[3][0];
     P_p_H[0][1] = _P_p[0][2];
     P_p_H[1][1] = _P_p[1][2];
     P_p_H[2][1] = _P_p[2][2];
     P_p_H[3][1] = _P_p[3][2];

     matrix_mul_4x2(K,  P_p_H, A_inv);
    /*
    Y_e = Y_p + K*(Z - H*Y_p);
    */
     H_Y_p[0] = Y_p[0];
     H_Y_p[1] = Y_p[2];
     H_Y_p[0] = r - H_Y_p[0];
     H_Y_p[1] = fi - H_Y_p[1];

     Y_e[0] = Y_p[0] + K[0][0]*H_Y_p[0] + K[0][1]*H_Y_p[1];
     Y_e[1] = Y_p[1] + K[1][0]*H_Y_p[0] + K[1][1]*H_Y_p[1];
     Y_e[2] = Y_p[2] + K[2][0]*H_Y_p[0] + K[2][1]*H_Y_p[1];
     Y_e[3] = Y_p[3] + K[3][0]*H_Y_p[0] + K[3][1]*H_Y_p[1];

    /*
    P_e = (eye(4)-K*H)*P_p;
    */
     EYE[0][0] -= K[0][0];
     EYE[1][0] -= K[1][0];
     EYE[2][0] -= K[2][0];
     EYE[3][0] -= K[3][0];

     EYE[0][2] -= K[0][1];
     EYE[1][2] -= K[1][1];
     EYE[2][2] -= K[2][1];
     EYE[3][2] -= K[3][1];

     matrix_mul_4x4( _P_e, EYE, _P_p);

    /* copy results back to the row vector (part of 3D array) */
    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            P_p[4*k+j] = _P_p[k][j];
            P_e[4*k+j] = _P_e[k][j];
        }
    }

}

void stackManager::new_tg_ident ( word nn_NTI, word min_NTI, word NTI[], real r_last_obs[], real fi_last_obs[],
                    real r, real fi, real dif_d, real dif_fi, word nn_track, word cl[], word max_nn_tr,
                    word *new_track, word clearing[]
                  ) {
    word check, empty_nti;
    word k, nti_;
    real dif1, dif2;

    check = 0;
    empty_nti = -1;
    for( k=0; k<nn_NTI; k++ )
        clearing[k] = cl[k];
    *new_track = 0;

    nti_ = 0;


    while ( (nti_ < nn_NTI)  && (check == 0) ) {

        if ( NTI[nti_] > 0 ) {
            dif1 = (real) fabs( r_last_obs[nti_] - r );
            dif2 = (real) fabs( fi_last_obs[nti_] - fi );
            if( (dif1 <= dif_d) && (dif2 <= dif_fi) ) {
                NTI[nti_] = NTI[nti_] + 1;
                check = 1;
                if (NTI[nti_] >= min_NTI) {

                    if ( (nn_track+1) <= max_nn_tr )
                        *new_track = nn_track+1;
                    else
                        *new_track = max_nn_tr;

                    clearing[nti_] = 1;
                }
                else
                    clearing[nti_] = 0;
            }
        }
        else
            empty_nti = nti_;

        nti_ = nti_ + 1;
    }
    /*
    if check == 0 && empty_nti > 0
        NTI(empty_nti) = 1;
        last_obs(:,empty_nti) = Z;
        clearing(empty_nti) = 0;
    end
    */
    if ( (check == 0) && (empty_nti >= 0) ) {  // indexing from 0 in C
        NTI[empty_nti] = 1;
        r_last_obs[empty_nti] = r;
        fi_last_obs[empty_nti] = fi;
        clearing[empty_nti] = 0;
    }
}

void stackManager::polar2cartesian ( real Y_e[], real *x, real *y) {
   *x = *y = 0.0;

   *x = Y_e[0]* (real)cos(Y_e[2]);
   *y = Y_e[0]* (real)sin(Y_e[2]);
}

void stackManager::MTT2 (real P[][MAX_N], real r[], real q[], real dif_d, real dif_fi, word min_OLGI, word min_NTI, real T[][MAX_N], word *start ) {

    word k, j, t, track;
    real cost;
    word A[MAX_N][MAX_N];
    word max_MA, tmp;
    word Clearing[3];
    word new_track;


    word nn_NTI;

    /* nn_NTI = 3; % pocet identifikatorov novych cielov */
   nn_NTI = 3;

    switch( *start ) {
        case START_MTT_INIT:

    /********** START_MTT_INIT ****************/
    /*
    [nn_co, max_nn_tr, nn_im] = size(P);
    T = zeros(nn_co, max_nn_tr, nn_im);

    % nastavitelne parametre
    p_init = [0.01 0.01 0.01 0.01]; % prvky na diagonale kovariancnej matice
    P_init = diag(p_init); % kovariancna matica
    Y_e_2_init = 0.1; % pociatocny odhad rychlosti
    Y_e_4_init = 0.1; % pociatocny odhad uhlovej rychlosti

    */
    for( k=0; k<MAX_N; k++ ) {
         T[0][k] = 0;
         T[1][k] = 0;
    }

    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            P_init[k][j] = 0;
        }
    }
    P_init[0][0] = P_init[1][1] = P_init[2][2] = P_init[3][3] = (real) 0.01;

    Y_e_2_init = (real) 0.1;
    Y_e_4_init = (real) 0.1;

    /*
    R = diag(r);
    Q = diag(q);
    */

   R[0][1] = R[1][0] = 0.0;
   R[0][0] = r[0];
   R[1][1] = r[1];

   for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            Q[k][j] = 0;
        }
    }
    Q[0][0] = q[0];
    Q[1][1] = q[1];
    Q[2][2] = q[2];
    Q[3][3] = q[3];

    /*
    % inicializacia premennych
    Z = zeros (nn_co,max_nn_tr,nn_im); % observations
    Y_p = zeros(4,1,max_nn_tr); % state prediction vectors
    P_p = zeros(4,4,max_nn_tr); % prediction error covariance matrix
    K = zeros(4,2,max_nn_tr); % Kalman gain matrix
    Y_e = zeros(4,1,max_nn_tr); % state estimation vectors
    P_e = zeros(4,4,max_nn_tr); % estimation covariance matrix
    */
    for( k=0; k<MAX_N; k++ ) {
        Z[k][0] = 0;
        Z[k][1] = 0;
    }

    for( k=0; k<4; k++ ) {
        for( j=0; j<MAX_N; j++ ) {
            Y_p[j][k] = 0;
            Y_e[j][k] = 0;
            K[j][k][0] = 0;
            K[j][k][1] = 0;
        }
    }

    for( k=0; k<4; k++ ) {
        for( j=0; j<MAX_N; j++ ) {
                for( t=0; t<4; t++ ) {
                    P_p[j][t][k] = 0;
                    P_e[j][t][k] = 0;
                }
        }
    }
    /*
    NTI = zeros(1,nn_NTI); % new target identification
    last_obs = zeros(2,nn_NTI); % posledne pozorovanie
    OLGI = zeros(1,max_nn_tr); % observation-less gate identification
    */
    NTI[0] = NTI[1] = NTI[2] = 0;
    last_obs[0][0] = last_obs[0][1] = last_obs[0][2] = 0;
    last_obs[1][0] = last_obs[1][1] = last_obs[1][2] = 0;
    for( k=0; k< MAX_N; k++ ) {
        OLGI[k] = 0;
    }


             *start = WAIT_MTT;
             start_im = SKIP_IR;
    /********** END of START_MTT_INIT ****************/

        case WAIT_MTT:
        /********** START_WAIT_MTT ****************/
        /*
        % nastartovanie prveho tracku na prvom pozorovani
        nn_obs = 0;
        start_im = START_TRACK_IM;
        */
             if( start_im-- )
                   return;
             *start = START_MTT_TRACK;
        /********** END of START_WAIT_MTT ****************/

        case START_MTT_TRACK:

        /********** START_MTT_TRACK ****************/

        /*
        % nastartovanie prveho tracku na prvom pozorovani

        while nn_obs == 0
           obs = 1;
                if P(1,obs,start_im) ~= 0 || P(2,obs,start_im) ~= 0
                    nn_obs = nn_obs + 1;
                    Z(:,nn_obs,start_im) = cartesian2polar (P(:,obs,start_im));
                    % prepocet kartezianskych suradnic na polarne
                    [Y_e(:,:,nn_obs),P_e(:,:,nn_obs)] = init_estimation ...
                               (Z(:,nn_obs,start_im),Y_e_2_init,Y_e_4_init,P_init);
                    % inicializacia Y_e, P_e
                end
           start_im = start_im + 1;
        end

        nn_track = nn_obs; % jedno pozorovanie, jeden track
        */

            nn_obs = 0;
            if ( nn_obs == 0 ) {
               obs = 0;
               if( (P[0][obs] != 0) || (P[1][obs] != 0) ) {
                   cartesian2polar( P[0][obs], P[1][obs], &Z[nn_obs][0], &Z[nn_obs][1] );
                   init_estimation (Z[nn_obs][0], Z[nn_obs][1], Y_e_2_init, Y_e_4_init, P_init, &Y_e[nn_obs][0], &P_e[nn_obs][0][0]);
                   nn_obs++;
                   nn_track = nn_obs;
                   *start = MTT_TRACK;
                   break;

               }
            }
            break;

        /********** END of START_MTT_TRACK ****************/
        case MTT_TRACK:
        default:
        /*
            nn_obs = 0;
            for obs = 1 : max_nn_tr
                if P(1,obs,im) ~= 0 || P(2,obs,im) ~= 0
                    nn_obs = nn_obs + 1;
                    Z(:,nn_obs,im) = cartesian2polar (P(:,obs,im));
                    % prepocitavanie kartezianskych suradnic na polarne
                end
            end
        */
            for( k=0; k<MAX_N; k++ ) {      // Matlab code uses 3-d dimmension (im)
               Z[k][0] = 0;
               Z[k][1] = 0;
            }

            nn_obs = 0;
            for (obs=0; obs < MAX_N; obs++) {
                if ( (P[0][obs] != 0) || (P[1][obs] != 0) ) {
                    cartesian2polar( P[0][obs], P[1][obs], &Z[nn_obs][0], &Z[nn_obs][1] );
                    nn_obs = nn_obs + 1;
                }
            }

        /*
            if nn_obs > 0 && nn_track == 0 % pripad, ked su vymazane vsetky tracky
                nn_track = 1;
                [Y_e(:,:,nn_track),P_e(:,:,nn_track)] = init_estimation ...
                                   (Z(:,nn_track,im),Y_e_2_init,Y_e_4_init,P_init);
                % novy track nastartujem prvym dostupnym pozorovanim
            end
        */

            if ( (nn_obs > 0) && (nn_track == 0) ) {
                 init_estimation (Z[nn_track][0], Z[nn_track][1], Y_e_2_init, Y_e_4_init, P_init, &Y_e[nn_track][0], &P_e[nn_track][0][0]);
                 nn_track = 1;
            }

        /*
            M = zeros(nn_obs,nn_track); % matica "Gate Mask"
            C = 10^2*ones(nn_obs,nn_track); % nakladova matica
        */
            for( k=0; k<MAX_N; k++ ) {
                 for( j=0; j<MAX_N; j++ ) {
                     M[k][j] = 0;
                     C[k][j] = 0;
                 }
            }

            for( k=0; k<nn_obs; k++ )
                for( j=0; j<nn_track; j++ )
                        C[k][j] = LARGE_NUMBER;



        /*
            for track = 1 : nn_track
                [Y_p(:,:,track), P_p(:,:,track)] = prediction ...
                                    (Y_e(:,:,track),P_e(:,:,track),Q); % predikcia
                for obs = 1 : nn_obs
                    [M(obs,track),C(obs,track)] = gate_checker ...
                        (Z(:,obs,im),Y_p(:,:,track),R,P_p(:,:,track)); % kontrola
                    % "bran", t.j. ktore pozorovanie je v blizkosti ktoreho tracku
                end
            end
        */
             for( track=0; track<nn_track; track++ ) {
                  prediction ( &Y_e[track][0], &P_e[track][0][0], Q, &Y_p[track][0], &P_p[track][0][0] );
                  for( obs=0; obs<nn_obs; obs++ ) {
                      gate_checker ( Z[obs][0], Z[obs][1], &Y_p[track][0], R, &P_p[track][0][0], &M[obs][track],  &C[obs][track]);
                  }
             }


    if( nn_obs == 0) {
        /*
           if nn_obs == 0 % pripad, ked nie je dostupne ziadne pozorovanie
                for track = 1 : nn_track
                    OLGI(track) = OLGI(track) + 1; % zvysi sa "observation-less
                    % gate identificator" pre kazdy track
                    if OLGI(track) >= min_OLGI % podmienka pre vylucenie tracku
                        [Y_e(:,:,track),P_e(:,:,track),Y_e(:,:,nn_track),...
                            P_e(:,:,nn_track),OLGI(track),OLGI(nn_track)] = ...
                                obs_less_gate_ident ...
                                    (OLGI(nn_track),track,nn_track,...
                                          Y_e(:,:,nn_track),P_e(:,:,nn_track));
                        nn_track = nn_track - 1;
                    end
                end
        */
                for( track=0; track<nn_track; track++ ) {
            OLGI[track]++;
            if (OLGI[track] >= min_OLGI) {
                obs_less_gate_ident (&OLGI[nn_track], track, nn_track, &Y_e[nn_track][0], &P_e[nn_track][0][0],
                          &Y_e[track][0], &P_e[track][0][0], &OLGI[track]);
                nn_track--;
            }
        }
    }
    else {
        /*
            else % k dispozicii su aj pozorovania, aj tracky
                [A,cost] = munkres(C); % vyriesenie priradovacieho problemu
                MA = M.*A; % matica MA dava vierohodnejsie vysledky ako matica A,
                % ktora najde priradenie aj v pripade nuloveho riadku alebo stlpca
                % matice M (co je nespravny postup)
                if max(MA) == 0
                    MA = M;
                end
        */
        munkres( C, nn_obs, nn_track, &cost, A );
        max_MA = INT_MIN;
        for( k=0; k<nn_obs; k++ ) {
            for( j=0; j<nn_track; j++ ) {
                 tmp =  M[k][j]*A[k][j];
                 MA[k][j] = tmp;
                 if ( tmp > max_MA)
                        max_MA = tmp;
            }
        }

                if (max_MA == 0 ) {
            for( k=0; k<nn_obs; k++ ) {
                for( j=0; j<nn_track; j++ ) {
                    MA[k][j] = M[k][j];
                                }
                        }
                }
        /*
                for track = 1 : nn_track
                    obs = find(MA(:,track)==1,1);
                    if isempty(obs) == 0 % existuje priradenie
                        [Y_e(:,:,track), P_e(:,:,track)] = ...
                        correction(Y_p(:,:,track),P_p(:,:,track),Z(:,obs,im),R);
                        % korekcia predikcie podla priradeneho pozorovania
                        OLGI(track) = 0; % vynulovanie identifikatora
                    else
                        % Observation-less Gate Identification
                        OLGI(track) = OLGI(track) + 1;
                        if OLGI(track) >= min_OLGI % podmienka pre vylucenie tracku
                            [Y_e(:,:,track),P_e(:,:,track),Y_e(:,:,nn_track),...
                                P_e(:,:,nn_track),OLGI(track),OLGI(nn_track)] = ...
                                obs_less_gate_ident(OLGI(nn_track),track,...
                                nn_track,Y_e(:,:,nn_track),P_e(:,:,nn_track));
                                nn_track = nn_track - 1;
                        end
                    end
                 end % koniec cyklu cez vsetky tracky
            end
        */
         for( track=0; track<nn_track; track++ ) {
            obs = EMPTY;
            for( k=0; k< nn_obs; k++ ) {
                if ( obs == EMPTY) {
                    if( MA[k][track] == 1 )
                        obs = k;
                }
            }
            if ( obs != EMPTY) {
                correction ( &Y_e[track][0], &P_e[track][0][0], &Y_p[track][0], &P_p[track][0][0], Z[obs][0], Z[obs][1], R );
                OLGI[track] = 0;
            }
            else {
                OLGI[track]++;
                if (OLGI[track] >= min_OLGI) {
                     obs_less_gate_ident (&OLGI[nn_track-1], track, nn_track-1, &Y_e[nn_track-1][0], &P_e[nn_track-1][0][0],
                          &Y_e[track][0], &P_e[track][0][0], &OLGI[track]);
                     nn_track--;
                }
            }
         }
    }


        /*
            % New Target Identification
            clearing = ones(1,nn_NTI); % premenna na nulovanie NTI
            for obs = 1 : nn_obs
                if max(M(obs,:)) == 0 % podmienka pre mozny vznik noveho tracku
                    [NTI,new_track,clearing,last_obs] = ...
                         new_tg_ident (nn_NTI,min_NTI,NTI,last_obs,Z(:,obs,im),...
                            dif_d,dif_fi,nn_track,clearing,max_nn_tr); % overovanie
                            %dostatocnej blizkosti a poctu nepriradenych pozorovani
                    if new_track > 0
                        % inicializacia noveho tracku
                        [Y_e(:,:,new_track),P_e(:,:,new_track)] = init_estimation ...
                                     (Z(:,nn_obs,im),Y_e_2_init,Y_e_4_init,P_init);
                        nn_track = new_track;
                    end
                end
            end
        */
    Clearing[0] = Clearing[1] = Clearing[2] = 1;
    for( obs=0; obs<nn_obs; obs++ ) {
        if( find_max ( M, obs) == 0 ) {
            new_tg_ident ( nn_NTI, min_NTI, NTI, &last_obs[0][0], &last_obs[1][0],
                    Z[obs][0], Z[obs][1], dif_d, dif_fi, nn_track, Clearing, MAX_N,
                    &new_track, Clearing );
            if( new_track > 0 ) {
               init_estimation (Z[nn_obs-1][0],Z[nn_obs-1][1] , Y_e_2_init, Y_e_4_init, P_init, &Y_e[new_track-1][0], &P_e[new_track-1][0][0]);
               nn_track = new_track;
            }
        }
    }
    /*
        for nti = 1 : nn_NTI % nulovanie identifikatora a posledneho pozorovania
            if clearing(nti) == 1;
                NTI(nti) = 0;
                last_obs(:,nti) = [0;0];
            end
        end
    */
    for( k=0; k< nn_NTI; k++ ) {
        if (Clearing[k] == 1) {
            NTI[k] = 0;
            last_obs[0][k] = 0;
            last_obs[1][k] = 0;
        }
    }

    /*
        for track = 1 : nn_track % prepocitavanie polarnych suradnic na kartez.
            T(:,track,im) = polar2cartesian (Y_e(:,1,track));
        end
    */

    for( k=0; k< MAX_N; k++ ) {
                T[0][k] = T[1][k] = 0.0;         /* clear previous results */
    }

    for( track=0; track<nn_track; track++ ) {
        polar2cartesian ( &Y_e[track][0], &T[0][track], &T[1][track] );
    }




        break;

    }
}

float* stackManager::MTT(float* P_mem,float r[], float q[],float dif_d,float dif_fi, int min_OLGI,int min_NTI){
    int k;

    /*
     * Jednorozmerny vstup na dvojrozmerne pole
     */
    for(k=0;k<MAX_N;k++){
        P[0][k]=P_mem[2*k];
        P[1][k]=P_mem[2*k+1];
    }
    //Funkciu som zabalil aby sa zbavil parametra &start
    MTT2(P,r,q,dif_d,dif_fi,min_OLGI,min_NTI,T,&start);
    /*
     *Transformacia spat
     */
    for(k=0;k<MAX_N;k++){
        P_mem[2*k]=T[0][k];
        P_mem[2*k+1]=T[1][k];
    }
    return P_mem;
}


