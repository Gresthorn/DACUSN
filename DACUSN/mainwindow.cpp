#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mainToolBar->setContextMenuPolicy(Qt::CustomContextMenu); // Custom context menu is not implemented so no menu will appear (unhidable toolbar)

    // create settings object filled with default values
    settings = new uwbSettings();
    // creating protector mutex for settings
    settingsMutex = new QMutex;

    dataStack = new QVector<rawData * >;
    dataStackMutex = new QMutex;

    radarList = new QVector<radar_handler * >;
    radarListMutex = new QMutex;

    visualizationData = new QList<QPointF * >;
    visualizationColor = new QList<QColor * >;
    // preparing some basic color set for targets
    visualizationColor->append(new QColor(Qt::red));
    visualizationColor->append(new QColor(Qt::blue));
    visualizationColor->append(new QColor(Qt::green));
    visualizationColor->append(new QColor(Qt::gray));
    visualizationColor->append(new QColor(Qt::yellow));
    visualizationColor->append(new QColor(Qt::black));
    visualizationColor->append(new QColor(Qt::cyan));
    visualizationColor->append(new QColor(Qt::magenta));
    visualizationColor->append(new QColor(Qt::darkRed));
    visualizationColor->append(new QColor(Qt::darkBlue));
    visualizationDataMutex = new QMutex;

    lastKnownSchema = settings->getVisualizationSchema();

    this->setWindowTitle(tr("Centrum asociácie dát v UWB sensorovej sieti"));

    /* ------------------------------------------------- THREADS ------------------------------------------------- */


    /* ------------------------------------------------- DATA RECIEVING ------------------------------------------ */
    dataInputThread = NULL;
    dataInputWorker = NULL;
    pauseBlinkEffect = NULL;
    blinker = true;

    connect(ui->actionPause, SIGNAL(triggered()), this, SLOT(pauseDataInputSlot()));
    connect(ui->actionRestart, SIGNAL(triggered()), this, SLOT(restartDataInputSlot()));
    connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(establishDataInputThreadSlot()));

    /* ------------------------------------------------- DATA RECIEVING ------------------------------------------ */

    /* ------------------------------------------------- STACK MANAGER ------------------------------------------- */

    stackManagerThread = NULL;
    stackManagerWorker = NULL;

    /* ------------------------------------------------- STACK MANAGER ------------------------------------------- */

    /* ------------------------------------------------- VISUALIZATION ------------------------------------------- */

    visualizationTimer = new QTimer(this);
    visualizationScene = new radarScene(settings, settingsMutex, this);

    /*QGraphicsTextItem * testText = new QGraphicsTextItem("Hello world");
    testText->setPos(50.0, 50.0);
    QTransform textTransform;
    textTransform.scale(1.0, -1.0);
    testText->setTransform(textTransform);
    visualizationScene->addItem(testText);*/

    visualizationView = new radarView(visualizationScene, settings, settingsMutex, this);

    visualizationView->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);

    ui->radarViewLayout->addWidget(visualizationView, 0, 0);

    visualizationManager = new animationManager(visualizationScene, visualizationView, visualizationData, visualizationColor, visualizationDataMutex, settings, settingsMutex);

    connect(visualizationTimer, SIGNAL(timeout()), this, SLOT(visualizationSlot()));
    visualizationTimer->setInterval(settings->getVisualizationInterval());

    /* ------------------------------------------------- VISUALIZATION ------------------------------------------- */


    /* ------------------------------------------------- SCENE CONTROLS ------------------------------------------ */

    connect(ui->rotationControlDial, SIGNAL(sliderMoved(int)), this, SLOT(sceneRotationChangedSlot(int)));
    connect(ui->sceneRotationSettingsButton, SIGNAL(clicked()), this, SLOT(sceneRotationManualChangeSlot()));
    connect(ui->moveToXYButton, SIGNAL(clicked()), this, SLOT(sceneMoveToXYSlot()));
    connect(ui->centerToZeroButton, SIGNAL(clicked()), this, SLOT(centerToZeroSlot()));
    connect(ui->pathHistoryCheckButton, SIGNAL(clicked()), this, SLOT(pathHistoryShow()));

    /* ------------------------------------------------- SCENE CONTROLS ------------------------------------------ */


    /* ------------------------------------------------- DIALOGS SLOTS ------------------------------------------- */

    connect(ui->actionData_Input_Dialog, SIGNAL(triggered()), this, SLOT(openDataInputDialog()));
    connect(ui->actionStack_Manager_Dialog, SIGNAL(triggered()), this, SLOT(openStackManagerDialog()));
    connect(ui->actionRadar_List_Dialog, SIGNAL(triggered()), this, SLOT(openRadarListDialog()));
    connect(ui->actionScene_renderer, SIGNAL(triggered()), this, SLOT(openSceneRendererDialog()));

    /* ------------------------------------------------- DIALOGS SLOTS ------------------------------------------- */
}

MainWindow::~MainWindow()
{
    // if some of data recieving is still runnning, stopping it
    this->destroyDataInputThreadSlot();

    delete ui;
}

void MainWindow::pauseDataInputSlot()
{
    // if the pointers are NULL, no thread or object was created what means either error or no running thread
    if(dataInputWorker==NULL || dataInputWorker==NULL)
    {
        qDebug() << "The data recieving thread seems not running. Nothing to pause.";
    }
    else
    {
        dataInputWorker->switchPauseState();
        stackManagerWorker->switchPauseState();

        if(pauseBlinkEffect==NULL)
        {
            // start blink effect
            pauseBlinkEffect = new QTimer(this);
            pauseBlinkEffect->setInterval(1000);
            connect(pauseBlinkEffect, SIGNAL(timeout()), this, SLOT(changeDataInputPauseButtonSlot()));
            pauseBlinkEffect->start();

            // pause visualization
            visualizationTimer->stop();
        }
        else
        {
            // stop blink effect
            pauseBlinkEffect->stop();
            delete pauseBlinkEffect;
            pauseBlinkEffect = NULL;
            blinker = true;
            ui->actionPause->setIcon(QIcon(":/mainToolbar/icons/pause.png"));

            // return visualization running state
            visualizationTimer->start();
        }
    }
}

void MainWindow::restartDataInputSlot()
{
    // kill old thread
    destroyDataInputThreadSlot();

    // start new thread
    establishDataInputThreadSlot();

    qDebug() << "Data recieving restarted...";
}

void MainWindow::establishDataInputThreadSlot()
{
    // if the pointer have no NULL value, probably the thread is still running
    if(dataInputThread!=NULL || dataInputWorker!=NULL)
    {
        qDebug() << "The data recieving thread seems still running. To start new thread, stop the previous thread.";
        return;
    }

    // change the action icon and connect to stop slot
    ui->actionStart->setIcon(QIcon(":/mainToolbar/icons/off.png"));
    connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(destroyDataInputThreadSlot()));
    disconnect(ui->actionStart, SIGNAL(triggered()), this, SLOT(establishDataInputThreadSlot()));
    // enabling another management buttons
    ui->actionPause->setEnabled(true);
    ui->actionRestart->setEnabled(true);

    // create wait for condition object so it will be able to pause thread if needed
    dataInputWorker = new dataInputThreadWorker(dataStack, dataStackMutex, settings, settingsMutex);
    // setting the maximum priority to ensure that data will not be lost because of processing the old one
    dataInputThread = new QThread(this);
    dataInputWorker->moveToThread(dataInputThread);
    dataInputThread->start(QThread::TimeCriticalPriority);
    QMetaObject::invokeMethod(dataInputWorker, "runWorker", Qt::QueuedConnection);

    qDebug() << "Data recieving thread started...";

    // starting stack manager thread
    establishStackManagementThread();

    // turn visualization on
    visualizationTimer->start();
}

void MainWindow::destroyDataInputThreadSlot()
{
    // If the both of the pointers are NULL it has not reason to continue
    // But if at least one of them is not NULL, it may mean the error or running thread.
    // In this case the thread must be stopped and if the error state is present, the
    // following algorithm will free the memory and set pointers back to NULL.
    if(dataInputWorker==NULL && dataInputThread==NULL)
    {
        qDebug() << "The thread does not exist. Nothing to stop.";
        return;
    }

    // change the action icon and connect to start slot
    ui->actionStart->setIcon(QIcon(":/mainToolbar/icons/on.png"));
    connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(establishDataInputThreadSlot()));
    disconnect(ui->actionStart, SIGNAL(triggered()), this, SLOT(destroyDataInputThreadSlot()));
    // enabling another management buttons
    ui->actionPause->setEnabled(false);
    ui->actionRestart->setEnabled(false);

    // if the method was initiated during pause, need to stop blinker
    if(pauseBlinkEffect!=NULL)
    {
        // stop blink effect
        pauseBlinkEffect->stop();
        delete pauseBlinkEffect;
        pauseBlinkEffect = NULL;
        blinker = true;
        ui->actionPause->setIcon(QIcon(":/mainToolbar/icons/pause.png"));
    }

    // NULL condition is used, just in case
    if(dataInputWorker!=NULL)
    {
        dataInputWorker->stopWorker(); // change the pauseState boolean value
        dataInputWorker->releaseIfInPauseState(); // if in pause, wake up: must be called after stopWorker method if to stop is desired

        // wait until worker is stopped properly
        while(!dataInputWorker->checkStoppedStatus())
        {
            Sleep(5);
            continue;
        }
    }

    // closing thread
    if(dataInputThread!=NULL) dataInputThread->terminate();

    // deleting objects

    if(dataInputWorker!=NULL) delete dataInputWorker;
    if(dataInputThread!=NULL) delete dataInputThread;

    dataInputWorker = NULL;
    dataInputThread = NULL;

    qDebug() << "Data recieving thread canceled...";

    // cancel the stack manager thread
    destroyStackManagementThread();

    // stop visualization
    visualizationTimer->stop();
}

void MainWindow::changeDataInputPauseButtonSlot()
{
    if(blinker) ui->actionPause->setIcon(QIcon(":/mainToolbar/icons/play.png"));
    else ui->actionPause->setIcon(QIcon(":/mainToolbar/icons/pause.png"));
    blinker = !blinker; // negate blinker
}

void MainWindow::establishStackManagementThread()
{
    qDebug() << "Starting stack management thread...";

    stackManagerThread = new QThread(this);
    stackManagerWorker = new stackManager(dataStack, dataStackMutex, radarList, radarListMutex, visualizationData, visualizationColor, visualizationDataMutex, settings, settingsMutex);
    stackManagerWorker->moveToThread(stackManagerThread);
    stackManagerThread->start(QThread::HighestPriority);
    QMetaObject::invokeMethod(stackManagerWorker, "runWorker", Qt::QueuedConnection);
}

void MainWindow::destroyStackManagementThread()
{
    // Stopping stack management thread is usually done as soon as the data reciever thread is stopped
    // so no processing is required in background in idle state.

    if(stackManagerThread==NULL && stackManagerWorker==NULL)
    {
        qDebug() << "Stack manager thread seems not runnning. Nothing to stop.";
        return;
    }


    // NULL condition is used, just in case
    if(stackManagerWorker!=NULL)
    {
        stackManagerWorker->stopWorker(); // change the pauseState boolean value
        stackManagerWorker->releaseIfInPauseState(); // if in pause, wake up: must be called after stopWorker method if to stop is desired

        // wait until worker is stopped properly
        while(!stackManagerWorker->checkStoppedStatus())
        {
            Sleep(5);
            continue;
        }
    }

    // closing thread
    if(stackManagerThread!=NULL) stackManagerThread->terminate();

    // deleting objects

    if(stackManagerWorker!=NULL) delete stackManagerWorker;
    if(stackManagerThread!=NULL) delete stackManagerThread;

    stackManagerWorker = NULL;
    stackManagerThread = NULL;

    qDebug() << "Stack manager thread canceled...";
}


/* ------------------------------------------------- VISUALIZATION SLOTS -------------------------------------------- */

void MainWindow::renderingEngineChangedSlot(rendering_engine rengine)
{
    if(rengine==OPEN_GL_ENGINE)
    {
        openGLWidget * t = new openGLWidget;
        setOglEngineFormatSlot(t);

        visualizationView->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);

        visualizationView->setViewport(t);

        visualizationView->viewport()->update();
        visualizationView->scene()->update();
    }
    else
    {
        visualizationView->setViewport(new QWidget);

        visualizationView->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);

        visualizationView->viewport()->update();
        visualizationView->scene()->update();
    }
}

void MainWindow::setOglEngineFormatSlot(openGLWidget * oglwidget)
{
    settingsMutex->lock();

    QGLFormat oglformat;
    if(settings->oglGetBufferType()==DOUBLE_BUFFERING)
    {
        oglformat.setDoubleBuffer(true);
    }
    else
    {
        oglformat.setDoubleBuffer(false);
    }
    oglformat.setDirectRendering(settings->oglGetDirectRendering());
    oglformat.setAccum(settings->oglGetAccumulationBuffer());
    oglformat.setStencil(settings->oglGetStencilBuffer());
    oglformat.setSampleBuffers(settings->oglGetMultisampleBuffer());

    oglformat.setRedBufferSize(settings->oglGetRedBufferSize());
    oglformat.setGreenBufferSize(settings->oglGetGreenBufferSize());
    oglformat.setBlueBufferSize(settings->oglGetBlueBufferSize());
    oglformat.setAlphaBufferSize(settings->oglGetAlphaBufferSize());
    oglformat.setDepthBufferSize(settings->oglGetDepthBufferSize());
    oglformat.setAccumBufferSize(settings->oglGetAccumulationBufferSize());
    oglformat.setStencilBufferSize(settings->oglGetStencilBufferSize());
    oglformat.setSamples(settings->oglGetMultisampleBufferSize());
    oglformat.setSwapInterval(settings->oglGetSwapInterval());

    oglwidget->setFormat(oglformat);

    settingsMutex->unlock();
}

void MainWindow::sceneRotationChangedSlot(int angle)
{

    visualizationView->setRotationAngle(angle);

    ui->sceneRotationLabel->setText(QString("Scene rotation: %1°").arg(angle));
}

void MainWindow::sceneRotationManualChangeSlot()
{
    int targetAngle = QInputDialog::getInt(this, tr("Enter the scene rotation angle."), tr("Scene rotation angle: "), ui->rotationControlDial->value(), -180, 180);

    bool smoothTransitionsEnabled;

    settingsMutex->lock();
        smoothTransitionsEnabled = settings->getSmoothTransitions();
    settingsMutex->unlock();

    if(smoothTransitionsEnabled)
    {
        // start animation
        visualizationView->setRotationAngleAnimation(targetAngle);
        ui->rotationControlDial->setValue(targetAngle);
        ui->sceneRotationLabel->setText(QString("Scene rotation: %1°").arg(targetAngle));
    }
    else
    {
        // just change angle
        visualizationView->setRotationAngle(targetAngle);
        ui->rotationControlDial->setValue(targetAngle);
        ui->sceneRotationLabel->setText(QString("Scene rotation: %1°").arg(targetAngle));
    }
}

void MainWindow::sceneMoveToXYSlot()
{
    // calculate the line between current center and target center
    QPointF currCenter = visualizationView->mapToScene(visualizationView->viewport()->rect().center());
    QPointF targetCenter;
    bool userDecision = false;

    // after user confirms dialog new center will be stored in targetCenter object
    coordinatesInputDialog dialog(currCenter, &targetCenter, &userDecision);
    dialog.exec();

    // if user cancelled dialog
    if(!userDecision) return;


    bool smoothTransitionEnabled;
    settingsMutex->lock();
        smoothTransitionEnabled = settings->getSmoothTransitions();
    settingsMutex->unlock();

    if(smoothTransitionEnabled)
    {
        visualizationView->moveToXYAnimation(targetCenter);
    }
    else
    {
        // simply center on target position
        visualizationView->centerOn(targetCenter);
    }
}

void MainWindow::centerToZeroSlot()
{
    bool smoothTransitionEnabled;
    settingsMutex->lock();
        smoothTransitionEnabled = settings->getSmoothTransitions();
    settingsMutex->unlock();

    if(smoothTransitionEnabled)
    {
        visualizationView->moveToXYAnimation(QPointF(0.0, 0.0));
        visualizationView->setRotationAngleAnimation(0);

        ui->rotationControlDial->setValue(0.0);
        ui->sceneRotationLabel->setText(QString("Scene rotation: %1°").arg(0));
    }
    else
    {
        visualizationView->centerOn(0.0, 0.0);
        visualizationView->setRotationAngle(0);

        ui->rotationControlDial->setValue(0.0);
        ui->sceneRotationLabel->setText(QString("Scene rotation: %1°").arg(0));
    }
}

void MainWindow::pathHistoryShow()
{
    if(ui->pathHistoryCheckButton->isChecked())
    {
        // start drawing paths

        bool pathHistorySaveEnabled;

        // First of all we need to switch to common Qt 2D painting engine since it supports local rendering.
        settingsMutex->lock();
        // Save the current visualization schema, so we can return to it after turning history drawing off.
        lastKnownSchema = settings->getVisualizationSchema();
        // Save the currently used engine.
        lastKnownEngine = settings->getRenderingEngine();
        // Check if so far history saving has been enabled.
        pathHistorySaveEnabled = settings->getHistoryPath();
        settingsMutex->unlock();

        // Now because of performance issues we need to switch to basic Qt 2D painting engine because of performance issues.
        if(lastKnownEngine!=STANDARD_QT_PAINTER)
        {
            delete visualizationView->viewport();
            visualizationView->setViewport(new QWidget);
        }

        // Taking absolute control of scene rendering.
        visualizationView->setMouseTracking(true);
        visualizationView->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);


        // Now if the history protocol is switched on, we can add all data from list to scene and do a global update.
        // If no history protocol was saved, if some data are availible in list, delete them.
        if(pathHistorySaveEnabled) visualizationManager->loadPathsList();
        else visualizationManager->clearPathsList();

        // Global update
        visualizationView->viewport()->update();

        // Because of performance it is better to absolutely disable rendering smoother level grids.
        settingsMutex->lock();
        lastKnownTappingOptions = settings->getTappingRenderMethod();
        // Save the smooth transitions enable/disable state.
        lastKnownSmoothTransitionsState = settings->getSmoothTransitions();

        settings->setSmootheTransitions(false);
        settings->setTappingRenderMethod(NO_3_2);

        // Finally start the path drawing schema
        settings->setVisualizationSchema(PATH_HISTORY);
        settingsMutex->unlock();

        // NOW WE HAVE SUCCESSFULY SWITCHED TO HISTORY DRAWING SCHEMA
        // NOTE: SINCE NOW EVERY ITEM ADDED OR SCROLLING/ZOOMING MUST BE UPDATED MANUALLY.
    }
    else
    {
        // stop drawing paths
        settingsMutex->lock();
        // setup back visualization schema, tapping options and engine to lastly used ones
        settings->setVisualizationSchema(lastKnownSchema);
        settings->setRenderingEngine(lastKnownEngine);
        settings->setTappingRenderMethod(lastKnownTappingOptions);
        settings->setSmootheTransitions(lastKnownSmoothTransitionsState);
        settingsMutex->unlock();

        // Now because of performance issues we need to switch to basic Qt 2D painting engine because of performance issues.
        if(lastKnownEngine==OPEN_GL_ENGINE)
        {
            delete visualizationView->viewport();
            visualizationView->setViewport(new QGLWidget);
        }

        // Restore the rendering methods
        visualizationView->setMouseTracking(false);
        visualizationView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

        // Remove all history objects/paths from scene.
        visualizationManager->removePathsFromScene();

        // Finally update scene without cross items
        visualizationScene->update();

        // NOW WE HAVE SUCCESSFULY SWITCHED BACK TO PREVIOUS REGIME
    }
}



/* ------------------------------------------------- DIALOGS SLOTS ------------------------------------------- */

void MainWindow::openDataInputDialog()
{
    dataInputDialog dialog(settings, settingsMutex, this);

    dialog.exec();
}

void MainWindow::openStackManagerDialog()
{
    stackManagerDialog dialog(settings, settingsMutex, this);

    dialog.exec();
}

void MainWindow::openRadarListDialog()
{
    radarListDialog dialog(radarList, radarListMutex, settings, settingsMutex, this);

    dialog.exec();
}

void MainWindow::openSceneRendererDialog()
{
    sceneRendererDialog dialog(settings, settingsMutex, visualizationScene, visualizationView, this);
    connect(&dialog, SIGNAL(renderingEngineChanged(rendering_engine)), this, SLOT(renderingEngineChangedSlot(rendering_engine)));

    dialog.exec();
}
