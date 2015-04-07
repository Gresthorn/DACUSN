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
    thisVisualizationManager = new animationManager(thisVisualizationScene, thisVisualizationView, thisVisualizationData, visualizationColor, visualizationDataMutex, settings, settingsMutex);

    ui->radarSubWindowViewLayout->addWidget(thisVisualizationView);

    this->setWindowTitle(tr("Radar %1 view").arg(radarId));
}

radarSubWindow::~radarSubWindow()
{
    delete ui;

    delete thisVisualizationScene;
    delete thisVisualizationView;
    delete thisVisualizationManager;
}
