#include "radarsubwindow.h"
#include "ui_radarsubwindow.h"

radarSubWindow::radarSubWindow(int radar_Id, uwbSettings *setts, QMutex *settings_mutex, QVector<radar_handler *> *radar_List, QMutex *radar_List_Mutex, QList<QColor *> *visualization_Color, QMutex *visualization_Data_Mutex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::radarSubWindow)
{
    ui->setupUi(this);

    // find the pointer to the correct radar according radarId
    thisRadarUnit = NULL;
    radar_List_Mutex->lock();

        for(int i = 0; i<radar_List->count(); i++)
        {
            if(radar_List->at(i)->id==radar_Id)
            {
                thisRadarUnit = radar_List->at(i)->radar;
                break;
            }
        }

    radar_List_Mutex->unlock();

    // if such id was not found, closing this window, esle continue in establishment
    if(thisRadarUnit==NULL)
    {
        qDebug() << "Radar with such identifier was not found in the list. Closing radar subwindow.";
        this->close();
    }

    radarId = radar_Id;

    // save important pointers
    settings = setts;
    settingsMutex = settings_mutex;
    radarList = radar_List;
    radarListMutex = radar_List_Mutex;
    visualizationColor = visualization_Color;
    visualizationDataMutex = visualization_Data_Mutex;

    // create own scenes, view and manager
    thisVisualizationData = new QList<QPointF * >;

    thisVisualizationScene = new radarScene(settings, settingsMutex, this);
    thisVisualizationView = new radarView(thisVisualizationScene, settings, settingsMutex, this);
    thisVisualizationDataMutex = new QMutex;
    thisVisualizationColor = new QList<QColor * >;
    thisVisualizationManager = new animationManager(thisVisualizationScene, thisVisualizationView, thisVisualizationData, thisVisualizationColor, thisVisualizationDataMutex, settings, settingsMutex);

    ui->radarSubWindowViewLayout->addWidget(thisVisualizationView);

    // update radar markers in private visualization scene
    thisVisualizationManager->setActiveRadarId(radarId);
    thisVisualizationManager->updateRadarMarkerList(radarList, radarListMutex);

    this->setWindowTitle(tr("Radar %1 view").arg(radarId));
}

radarSubWindow::~radarSubWindow()
{
    delete ui;

    delete thisVisualizationScene;
    delete thisVisualizationView;
    delete thisVisualizationManager;

    // need to lock, since no mutex protection is used inside clear function
    thisVisualizationDataMutex->lock();

    clearRadarData();
    clearColorList();

    thisVisualizationDataMutex->unlock();
}

void radarSubWindow::addVisualizationData(float *data_array, int count)
{
    // locking local mutex
    thisVisualizationDataMutex->lock();

    // delete all current data in list
    clearRadarData();

    // check for colors in list, if not enough, update
    if(thisVisualizationColor->count()<count) updateColorList();

    // fill list with new values
    for(int j = 0; j<count; j++)
    {
        thisVisualizationData->append(new QPointF(data_array[j*2], data_array[j*2+1]));
    }

    // unlocking local mutex
    thisVisualizationDataMutex->unlock();
}

void radarSubWindow::addVisualizationData(double *data_array, int count)
{
    // locking local mutex
    thisVisualizationDataMutex->lock();

    // delete all current data in list
    clearRadarData();

    // check for colors in list, if not enough, update
    if(thisVisualizationColor->count()<count) updateColorList();

    // fill list with new values
    for(int j = 0; j<count; j++)
    {
        thisVisualizationData->append(new QPointF(data_array[j*2], data_array[j*2+1]));
    }

    // unlocking local mutex
    thisVisualizationDataMutex->unlock();
}

void radarSubWindow::addVisualizationData(QList<QPointF *> data_list)
{
    // locking local mutex
    thisVisualizationDataMutex->lock();

    // delete all current data in list
    clearRadarData();

    // check for colors in list, if not enough, update
    if(thisVisualizationColor->count()<data_list.count()) updateColorList();

    // fill list with new values
    for(int j = 0; j<data_list.count(); j++)
    {
        thisVisualizationData->append(new QPointF(*data_list.at(j)));
    }

    // unlocking local mutex
    thisVisualizationDataMutex->unlock();
}

void radarSubWindow::updateColorList()
{
    // IMPORTANT NOTE: ALTHOUGH WE SHOULD USE MUTEXES HERE TO PROTECT VECTORS/LISTS, IN THIS CASE IT IS NOT REQUIRED.
    // thisVisualizationDataMutex is not needed since update is always called inside addVisualizationData function and here already mutexes are used
    // visualizationDataMutex is not needed but only in this particular case. addVisualizationData function is called inside fusion or already protected area with visualizationDataMutexes.

    // remove old colors in list
    //thisVisualizationDataMutex->lock();

    clearColorList();

    // create new list with updated colors from original color list
    //visualizationDataMutex->lock();
    for(int i = 0; i<visualizationColor->count(); i++)
    {
        thisVisualizationColor->append(new QColor(*visualizationColor->at(i)));
    }


    //visualizationDataMutex->lock();

    //thisVisualizationDataMutex->lock();
}

void radarSubWindow::updateRadarMarkers()
{
    thisVisualizationManager->updateRadarMarkerList(radarList, radarListMutex);
}

void radarSubWindow::clearRadarData()
{
    // not using mutex, since another processing is usually done in higher function so mutex should be used there.

    while(!thisVisualizationData->isEmpty())
    {
        delete thisVisualizationData->first();
        thisVisualizationData->removeFirst();
    }
}

void radarSubWindow::clearColorList()
{
    // not using mutex, since another processing is usually done in higher function so mutex should be used there.
    while(!thisVisualizationColor->isEmpty())
    {
        delete thisVisualizationColor->first();
        thisVisualizationColor->removeFirst();
    }
}
