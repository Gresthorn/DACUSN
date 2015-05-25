#include "mainwindow.h"
#include "ui_mainwindow.h"

double METER_TO_PIXEL_RATIO = 100.0;

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

    // FOR TEST PURPOSES ONLY - THIS SECTION IS USED FOR PACKET RECIEVING TEST - TEST IS DONE WITHOUT RUNNING THE REST OF APPLICATION

    /*char mode[] = "8N1";
    reciever * recieverHandler = new reciever(settings->getRecieverMethod(), 3, 9600, mode);

    qDebug() << settings->getRecieverMethod();

    // checking if the selected method was successfully established
    if(recieverHandler->calibration_status()) qDebug() << "The reciever was configured";
    else qDebug() << recieverHandler->check_status_message();

    while(1)
    {
        rawData * test = recieverHandler->listen();
        if(test==NULL)
        {
            //qDebug() << recieverHandler->check_status_message();
        }
        else
        {
            qDebug() << "Packet number: " << test->getUwbPacketPacketNumber();
            int targetsNumber = test->getUwbPacketTargetsCount();
            float * targetsCoordinates = test->getUwbPacketCoordinates();
            qDebug() << "Targets count: " << targetsNumber;
            for(int i = 0; i<targetsNumber; i++)
            {
                qDebug() << targetsCoordinates[i*2] << " " << targetsCoordinates[i*2+1];
            }

            delete test;
        }
    }
    return;*/

    // FOR TEST PUSRPOSES ONLY

    dataStack = new QVector<rawData * >;
    dataStackMutex = new QMutex;

    radarList = new QVector<radar_handler * >;
    radarSubWindowList = new QList<radarSubWindow * >;
    radarSubWindowListMutex = new QMutex;
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

    totalElapsedTimer = new QElapsedTimer;
    instanceElapsedTimer = NULL;
    instanceTimerAccumulator = 0;

    totalElapsedTimer->start();

    timerElapsed = new QTimer;
    timerElapsed->setInterval(1000);
    timerElapsed->start();
    connect(timerElapsed, SIGNAL(timeout()), this, SLOT(timeMeasureSlot()));


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

    averageRenderTime = renderIterationCount = 0;
    informationTableTimer = NULL;


    visualizationTimer = new QTimer(this);
    visualizationScene = new radarScene(settings, settingsMutex, this);

    visualizationView = new radarView(visualizationScene, settings, settingsMutex, this);

    visualizationView->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);

    ui->radarViewLayout->addWidget(visualizationView, 0, 0);

    visualizationManager = new animationManager(visualizationScene, visualizationView, visualizationData, visualizationColor, visualizationDataMutex, settings, settingsMutex);
    connect(this, SIGNAL(gridScaleValueUpdate(double)), visualizationManager, SLOT(updateObjectsScales(double)));

    connect(visualizationTimer, SIGNAL(timeout()), this, SLOT(visualizationSlot()));
    visualizationTimer->setInterval(settings->getVisualizationInterval());

    if(settings->getPeriodicalImgBackup())
    {
        periodicalExportTimer = new QTimer;
        connect(periodicalExportTimer, SIGNAL(timeout()), this, SLOT(periodicalExportViewImageSlot()));
        periodicalExportTimer->start(settings->getPeriodicalImgBackupInterval());
    }
    else periodicalExportTimer = NULL;

    if(settings->getHistoryPath()) ui->pathHistoryCheckButton->setChecked(true);
    else ui->pathHistoryCheckButton->setChecked(false);

    if(settings->getVisualizationEnabled()) ui->enableRenderingButton->setChecked(true);
    else ui->enableRenderingButton->setChecked(false);

    /* ------------------------------------------------- VISUALIZATION ------------------------------------------- */


    // update radar list widget with initial records and operator record
    radarListUpdated();

    /* ------------------------------------------------- SCENE CONTROLS ------------------------------------------ */

    connect(ui->rotationControlDial, SIGNAL(sliderMoved(int)), this, SLOT(sceneRotationChangedSlot(int)));
    connect(ui->sceneRotationSettingsButton, SIGNAL(clicked()), this, SLOT(sceneRotationManualChangeSlot()));
    connect(ui->moveToXYButton, SIGNAL(clicked()), this, SLOT(sceneMoveToXYSlot()));
    connect(ui->centerToZeroButton, SIGNAL(clicked()), this, SLOT(centerToZeroSlot()));
    connect(ui->pathHistoryCheckButton, SIGNAL(clicked()), this, SLOT(pathHistoryShow()));
    connect(ui->exportImageButton, SIGNAL(clicked()), this, SLOT(exportViewImageSlot()));
    connect(ui->enableRenderingButton, SIGNAL(clicked()), this, SLOT(enableRenderingSlot()));
    connect(ui->gridScaleSpinBox, SIGNAL(valueChanged(int)), this, SLOT(gridScaleValueChanged(int)));

    /* ------------------------------------------------- SCENE CONTROLS ------------------------------------------ */

    /* ------------------------------------------------- RADARS CONTROLS ----------------------------------------- */

    connect(ui->refreshRadarListButton, SIGNAL(clicked()), this, SLOT(radarListUpdated()));
    connect(ui->showInSubWindowButton, SIGNAL(clicked()), this, SLOT(addRadarSubWindow()));
    connect(ui->showInCentralButton, SIGNAL(clicked()), this, SLOT(showInCentral()));

    /* ------------------------------------------------- RADARS CONTROLS ----------------------------------------- */

    /* ------------------------------------------------- DIALOGS SLOTS ------------------------------------------- */

    connect(ui->actionData_Input_Dialog, SIGNAL(triggered()), this, SLOT(openDataInputDialog()));
    connect(ui->actionStack_Manager_Dialog, SIGNAL(triggered()), this, SLOT(openStackManagerDialog()));
    connect(ui->actionRadar_List_Dialog, SIGNAL(triggered()), this, SLOT(openRadarListDialog()));
    connect(ui->actionScene_renderer, SIGNAL(triggered()), this, SLOT(openSceneRendererDialog()));
    connect(ui->actionData_backup, SIGNAL(triggered()), this, SLOT(openDataBackupDialog()));
    connect(ui->actionMTT_Settings_Dialog, SIGNAL(triggered()), this, SLOT(openMTTSettingsDialog()));

    /* ------------------------------------------------- DIALOGS SLOTS ------------------------------------------- */
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // destroy all subwindows if exist
    radarSubWindowListMutex->lock();
    while(!radarSubWindowList->isEmpty())
    {
        // close will automatically emit signal which is passed into delete window slot. There also all deletions in list/vectors are done.
        radarSubWindowList->first()->close();
    }
    radarSubWindowListMutex->unlock();
}

MainWindow::~MainWindow()
{
    // if some of data recieving is still runnning, stopping it
    this->destroyDataInputThreadSlot();

    // destroy all subwindows if exist
    radarSubWindowListMutex->lock();
    while(!radarSubWindowList->isEmpty())
    {
        // close will automatically emit signal which is passed into delete window slot. There also all deletions in list/vectors are done.
        radarSubWindowList->first()->close();
    }
    radarSubWindowListMutex->unlock();

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

            // pausing instance time measurement
            if(instanceElapsedTimer!=NULL)
            {
                instanceTimerAccumulator += instanceElapsedTimer->elapsed();
                instanceElapsedTimer->invalidate();
            }
            // pause visualization
            visualizationTimer->stop();
            // if periodical image generation was selected, stop the timer
            if(periodicalExportTimer!=NULL) periodicalExportTimer->stop();
            // stop the information table update timer
            informationTableTimer->stop();
        }
        else
        {
            // stop blink effect
            pauseBlinkEffect->stop();
            delete pauseBlinkEffect;
            pauseBlinkEffect = NULL;
            blinker = true;
            ui->actionPause->setIcon(QIcon(":/mainToolbar/icons/pause.png"));

            // resuming instance time measurement
            if(instanceElapsedTimer!=NULL) instanceElapsedTimer->restart();
            // return visualization running state
            visualizationTimer->start();
            // if periodical image generation was selected, continue in timing
            if(periodicalExportTimer!=NULL) periodicalExportTimer->start();
            // resume information table timer
            informationTableTimer->start(5000);
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
    // turn image periodical generation on, if selected
    if(periodicalExportTimer!=NULL) periodicalExportTimer->start();
    // start time measurement
    if(instanceElapsedTimer!=NULL) delete instanceElapsedTimer;
    instanceElapsedTimer = new QElapsedTimer;
    instanceElapsedTimer->start();
    // create and start information table update timer
    informationTableTimer = new QTimer;
    connect(informationTableTimer, SIGNAL(timeout()), this, SLOT(informationTableUpdateSlot()));
    informationTableTimer->start(5000);

    // starting/closing data backup on disk according to user settings
    manageDiskBackupSlot();
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

    // deleting objects (DELETING only thread seems to be enough since, worker object is a child of thread object and in Qt by deleting parent, also child objects are deleted)
    if(dataInputThread!=NULL) delete dataInputThread;
    if(dataInputWorker!=NULL) delete dataInputWorker;

    dataInputWorker = NULL;
    dataInputThread = NULL;

    qDebug() << "Data recieving thread canceled...";

    // cancel the stack manager thread
    destroyStackManagementThread();

    // stop visualization
    visualizationTimer->stop();
    // turn off timer causing periodical image generation
    if(periodicalExportTimer!=NULL) periodicalExportTimer->stop();
    // deleting time measurement timer
    if(instanceElapsedTimer!=NULL) delete instanceElapsedTimer;
    instanceElapsedTimer=NULL;
    instanceTimerAccumulator = 0;
    // zero variables hodling helper values for average render time calculation
    averageRenderTime = renderIterationCount = 0;
    // delete information table update timer
    informationTableTimer->stop();
    disconnect(informationTableTimer, SIGNAL(timeout()), this, SLOT(informationTableUpdateSlot()));
    delete informationTableTimer;

    // if data backup is enabled, on stopping data recieving we should close the file and delete file handlers (return the program to the initiall state)
    deleteDiskBackupDependencies();
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
    stackManagerWorker = new stackManager(dataStack, dataStackMutex, radarList, radarListMutex, visualizationData, visualizationColor, radarSubWindowList, radarSubWindowListMutex, visualizationDataMutex, settings, settingsMutex);
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
    if(stackManagerThread!=NULL) delete stackManagerThread;
    if(stackManagerWorker!=NULL) delete stackManagerWorker;

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

        // Now because of performance issues we need to switch to basic Qt 2D painting engine.
        if(lastKnownEngine==OPEN_GL_ENGINE)
        {
            visualizationView->setViewport(new QWidget);
        }

        // Taking absolute control of scene rendering.
        visualizationView->setMouseTracking(true);
        visualizationView->setViewportUpdateMode(QGraphicsView::NoViewportUpdate); 

        // Global update
        // Delete/hide all items that should be not displayed during path drawing mode.
        visualizationManager->clearRadarMarkerList(true); // clear marker list and delete related objects
        visualizationManager->clearEllipseList(true); // clear all ellipse items representing objects and clearing list holding pointers to them
        QList<QGraphicsItem * > item_list = visualizationScene->items();
        for(int i = 0; i<item_list.count(); i++)
        {
            delete item_list.at(i);
            visualizationScene->removeItem(item_list.at(i));
        }

        // Note, that also radar markers are now deleted. But we can easily refresh them by calling radar list update function.
        visualizationManager->updateRadarMarkerList(radarList, radarListMutex);

        // Now if the history protocol is switched on, we can add all data from list to scene and do a global update.
        // If no history protocol was saved, if some data are availible in list, delete them.
        if(pathHistorySaveEnabled) visualizationManager->loadPathsList();
        else visualizationManager->clearPathsList();

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

        // enable or disable show in central view button
        realTimeRecordingChanged(true);
    }
    else
    {
        // stop drawing paths
        bool pathHistorySaveEnabled = false;
        settingsMutex->lock();
        // setup back visualization schema, tapping options and engine to lastly used ones
        settings->setVisualizationSchema(lastKnownSchema);
        settings->setRenderingEngine(lastKnownEngine);
        settings->setTappingRenderMethod(lastKnownTappingOptions);
        settings->setSmootheTransitions(lastKnownSmoothTransitionsState);
        // Check if so far history saving has been enabled.
        pathHistorySaveEnabled = settings->getHistoryPath();
        settingsMutex->unlock();

        // Now because of performance issues we need to switch to basic Qt 2D painting engine because of performance issues.
        if(lastKnownEngine==OPEN_GL_ENGINE)
        {
            renderingEngineChangedSlot(OPEN_GL_ENGINE);
        }

        // Restore the rendering methods
        visualizationView->setMouseTracking(false);
        visualizationView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

        // Remove all history objects/paths from scene.
        visualizationManager->removePathsFromScene();

        // Now reveal all common flow objects
        if(lastKnownSchema==COMMON_FLOW) visualizationManager->revealAllCommonFlowSchemaObjects();

        // Note, that also radar markers are now deleted. But we can easily refresh them by calling radar list update function.
        visualizationManager->updateRadarMarkerList(radarList, radarListMutex);

        // Finally update scene without cross items
        visualizationScene->update();

        // NOW WE HAVE SUCCESSFULY SWITCHED BACK TO PREVIOUS REGIME

        // enable or disable show in central view button
        realTimeRecordingChanged(pathHistorySaveEnabled);
    }
}

void MainWindow::periodicalImgBackupSlot(bool enabled)
{
    if(enabled)
    {
        settingsMutex->lock();
        unsigned int ms = settings->getPeriodicalImgBackupInterval();
        settingsMutex->unlock();

        if(periodicalExportTimer==NULL)
        {
            periodicalExportTimer = new QTimer;
            // if visualization is running, activate backup
            if(visualizationTimer->isActive())
            {
                connect(periodicalExportTimer, SIGNAL(timeout()), this, SLOT(periodicalExportViewImageSlot()));
                periodicalExportTimer->start(ms);
            }
            else
            {
                // if visualization is not running, we only create timer without starting it
                connect(periodicalExportTimer, SIGNAL(timeout()), this, SLOT(periodicalExportViewImageSlot()));
                periodicalExportTimer->setInterval(ms);
            }
        }
        else
        {
            // if timer is not NULL, that means that timer is created and we should turn it on if visualization is running/off if not
            if(visualizationTimer->isActive())
            {
                // start timer
                if(!periodicalExportTimer->isActive()) periodicalExportTimer->start(ms);
                else
                {
                    // if timer is already running, we will stop it and change the interval according to the newly set value
                    periodicalExportTimer->stop();
                    periodicalExportTimer->start(ms); // !!!
                }
            }
            else
            {
                // stopping timer to save CPU from unnecessary processing
                periodicalExportTimer->stop();
                periodicalExportTimer->setInterval(ms); // !!!!!
            }
        }
    }
    else
    {
        if(periodicalExportTimer!=NULL)
        {
            periodicalExportTimer->stop();
            disconnect(periodicalExportTimer, SIGNAL(timeout()), this, SLOT(periodicalExportViewImageSlot()));
            delete periodicalExportTimer;
        }

        periodicalExportTimer = NULL;
    }
}

void MainWindow::enableRenderingSlot()
{
    settingsMutex->lock();
    settings->setVisualizationEnabled(!settings->getVisualizationEnabled());
    settingsMutex->unlock();
}

void MainWindow::timeMeasureSlot()
{
    // update global timer
    ui->globalTimeCounterLabel->setText(timeToString(totalElapsedTimer->elapsed()));
    // update instance timer
    if(instanceElapsedTimer!=NULL && instanceElapsedTimer->isValid()) ui->instanceTimeCounterLabel->setText(timeToString(instanceElapsedTimer->elapsed()+instanceTimerAccumulator));
}

void MainWindow::informationTableUpdateSlot()
{
    qDebug() << "Updating information table...";
    // NOTE THAT ALSO 'visualizationSlot()', WHERE AVERAGE RENDER TIME VARIABLE IS UPDATED IS SLOT AS WELL AS THIS FUNCTION, SO THANKS TO THE SIGNAL-SLOT QUEUE
    // MECHANISM WE DO NOT NEED USE MUTEXES HERE.
    ui->averageRenderTimeLabel->setText(QString("%1").arg(averageRenderTime));

    visualizationDataMutex->lock();
    ui->targetsCountLabel->setText(QString("%1").arg(visualizationData->count()));
    visualizationDataMutex->unlock();

    // we do not have to use mutexes since it is done in 'getAverageProcessingSpeed()' method
    ui->averageProcessingTimeLabel->setText(QString("%1").arg(stackManagerWorker->getAverageProcessingSpeed()));
}

/* EXPORT VIEW TO IMAGE */
void MainWindow::exportViewImageSlot()
{
    QString path;
    settingsMutex->lock();
    path = settings->getExportPath();
    settingsMutex->unlock();

    QString file = QString("/EXPORT_%1.png").arg(QDateTime::currentDateTime().toString());

    // coordinates in window
    QRect windowRect;
    windowRect.setTopLeft(visualizationView->mapTo(visualizationView->window(), visualizationView->rect().topLeft()));
    windowRect.setBottomRight(visualizationView->mapTo(visualizationView->window(), visualizationView->rect().bottomRight()));

    QPixmap filePixmap = QWidget::grab(windowRect);

    file.replace(" ", "_");
    file.replace(":", "_");

    path.append(file);

    qDebug() << "Saving to: " << path;

    filePixmap.save(path);
}

/* PERIODICAL EXPORT VIEW TO IMAGE */
void MainWindow::periodicalExportViewImageSlot()
{
    QString path;
    settingsMutex->lock();
    path = settings->getPeriodicalImgBackupPath();
    settingsMutex->unlock();

    QString file = QString("/EXPORT_%1.png").arg(QDateTime::currentDateTime().toString());

    // coordinates in window
    QRect windowRect;
    windowRect.setTopLeft(visualizationView->mapTo(visualizationView->window(), visualizationView->rect().topLeft()));
    windowRect.setBottomRight(visualizationView->mapTo(visualizationView->window(), visualizationView->rect().bottomRight()));

    QPixmap filePixmap = QWidget::grab(windowRect);

    file.replace(" ", "_");
    file.replace(":", "_");

    path.append(file);

    qDebug() << "Saving to: " << path;

    filePixmap.save(path);
}

QString MainWindow::timeToString(qint64 timems)
{
    qint64 g_elapsed_secs = (timems/1000)%60;
    qint64 g_elapsed_mins = (timems/1000/60)%60;
    qint64 g_elapsed_hrs = (timems/1000/60/60);

    QString time_string("");
    (g_elapsed_hrs < 10) ? time_string.append(QString("0%1").arg(g_elapsed_hrs)) : time_string.append(QString("%1").arg(g_elapsed_hrs));
    (g_elapsed_mins < 10) ? time_string.append(QString(":0%1").arg(g_elapsed_mins)) : time_string.append(QString(":%1").arg(g_elapsed_mins));
    (g_elapsed_secs < 10) ? time_string.append(QString(":0%1").arg(g_elapsed_secs)) : time_string.append(QString(":%1").arg(g_elapsed_secs));

    return time_string;
}

void MainWindow::manageDiskBackupSlot()
{
    // if data backup is enabled we need to open/create file in write in mode and prepare QTextStream object
    settingsMutex->lock();
    bool backupIsEnabled = settings->getDiskBackupEnabled();
    settingsMutex->unlock();

    if(backupIsEnabled)
    {
        settingsMutex->lock();
        // if enabled backup, we need to prepare all objects required for writing into file
        QString filePath(settings->getDiskBackupFilePath());
        filePath.append(QString("//%1.txt").arg(settings->getBackupFileName()));
        QFile * backup_file = new QFile(filePath);
        backup_file->open(QFile::Text | QIODevice::WriteOnly); // if doeas not exist, it will be created

        QTextStream * in_file_stream = new QTextStream(backup_file); // since now we can use just this stream object to write into file

        // saving new objects
        settings->setBackupFileHandler(in_file_stream);
        settings->setBackupMainFileHandler(backup_file);
        settingsMutex->unlock();
    }
    else deleteDiskBackupDependencies();
}

void MainWindow::realTimeRecordingChanged(bool status)
{
    // If user decides to display also all history, we need to prevent from switching to another radar data visualization in central
    // view since only one buffer/memory is used to store data. If user could switch during memorizing, data would be mixed and no
    // reasonable result would be achieved. So then, only one view can be allowed when recording data.
    // !!! NOTE: This function is also called when user enters the recording mode although history recording (after switching back)
    // is turned off. The reason is same. If user could switch view during this mode, only mixing of absolutely different coordinates
    // !!! NOTE: YOU CAN PLAY WITH THIS PROBLEM IF YOU WANT, BUT CAREFULY. THIS 'SWITCHING' FEATURE WAS NOT IMPLEMENTED FOR NOW BECAUSE
    // OF TIME LIMITS.

    if(status) ui->showInCentralButton->setDisabled(true);
    else ui->showInCentralButton->setDisabled(false);
}

void MainWindow::gridScaleValueChanged(int m_to_pix_ratio)
{
    // NOTE: METER_TO_PIXEL_RATIO IS A GLOBAL VARIABLE!!! USE SIGNAL/SLOT MECHANISM TO MODIFY IT, OTHERWISE PROGRAM MAY CRASH

    double OLD_METER_TO_PIXEL_RATIO = METER_TO_PIXEL_RATIO;
    METER_TO_PIXEL_RATIO = m_to_pix_ratio;

    // move all objects in central view to correct, new position
    emit gridScaleValueUpdate(OLD_METER_TO_PIXEL_RATIO);
}

void MainWindow::deleteDiskBackupDependencies()
{

    settingsMutex->lock();
    // if file pointers are not NULL, some of previous backup is still pending and we should close the file securely
    if(settings->getBackupFileHandler()!=NULL)
    {
        settings->getBackupFileHandler()->device()->close(); // equivalent to closing MainFileHandler directly.
        delete settings->getBackupFileHandler();
        delete settings->getBackupMainFileHandler();
    }

    // now we will set back to NULL so program will cannot use old (unexisting) pointers to files/streams
    settings->setBackupFileHandler(NULL);
    settings->setBackupMainFileHandler(NULL);

    settingsMutex->unlock();
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

    connect(&dialog, SIGNAL(radarListUpdated()), this, SLOT(radarListUpdated()));

    dialog.exec();
}

void MainWindow::openSceneRendererDialog()
{
    sceneRendererDialog dialog(settings, settingsMutex, visualizationScene, visualizationView, this);
    connect(&dialog, SIGNAL(renderingEngineChanged(rendering_engine)), this, SLOT(renderingEngineChangedSlot(rendering_engine)));
    connect(&dialog, SIGNAL(periodicalImgBackup(bool)), this, SLOT(periodicalImgBackupSlot(bool)));
    connect(&dialog, SIGNAL(realTimeRecordingStatus(bool)), this, SLOT(realTimeRecordingChanged(bool)));
    dialog.exec();
}

void MainWindow::openDataBackupDialog()
{
    backupOptionsDialog dialog(settings, settingsMutex, this);
    connect(&dialog, SIGNAL(acceptedSignal()), this, SLOT(manageDiskBackupSlot()));
    dialog.exec();
}

void MainWindow::openMTTSettingsDialog()
{
    mttsettingsdialog dialog(settings, settingsMutex, this);

    dialog.exec();
}

/* ------------------------------------------------- RADAR SUBWINDOWS MANAGEMENT ----------------------------- */

void MainWindow::radarListUpdated()
{
    visualizationManager->updateRadarMarkerList(radarList, radarListMutex);

    // delete all items in list
    for(int i=0; i<ui->radarListWidget->count(); i++)
    {
        delete ui->radarListWidget->item(i);
    }
    ui->radarListWidget->clear();

    // fill with new records (mutexes, just in case)

    // firstly create operator item
    QListWidgetItem * item = new QListWidgetItem;
    item->setText(QString("OPERATOR"));
    item->setData(Qt::UserRole, -1);
    ui->radarListWidget->addItem(item);

    radarListMutex->lock();
    for(int a = 0; a<radarList->count(); a++)
    {
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(QString("Radar unit id: %1").arg(radarList->at(a)->id));
        // data will be used when user clicks the display in subwindow or central view to enable this functionality, for identifying radar unit.
        item->setData(Qt::UserRole, radarList->at(a)->id);
        ui->radarListWidget->addItem(item);
    }

    // now we need to go throught all opened subwindows and check if theier radar units still exist. If not, delete them.
    int id;
    bool found;
    QVector<int> rem_index; // stores all indexes where deletion is about to be applied (cannot delete immediately because list will change it size and 'for' loop will then go outside of list)

    radarSubWindowListMutex->lock();

    for(int i=0; i<radarSubWindowList->count(); i++)
    {
        found = false;
        id = radarSubWindowList->at(i)->getRadarId();
        qDebug() << "ID: " << id;
        // iterate over all radars and try to find such id
        for(int j = 0; j<radarList->count(); j++)
        {
            if(radarList->at(j)->id==id)
            {
                found = true;
                break;
            }
        }

        // radar unit do not exist, delete
        if(!found)
        {
            rem_index.append(i);
        }
    }

    radarListMutex->unlock();

    // now remove all items in list that in fact handles no window
    if(!rem_index.isEmpty())
    {
        for(int h = 0; h<rem_index.count(); h++)
        {
            // close signal will automatically call delete function/slot
            radarSubWindowList->at(rem_index.at(h))->close();
        }
    }

    // since we have only relevant sub windows, we can now update radar markers in their local managers
    for(int j=0; j<radarSubWindowList->count(); j++)
    {
        radarSubWindowList->at(j)->updateRadarMarkers();
    }

    radarSubWindowListMutex->unlock();
}

void MainWindow::deleteRadarSubWindow(radarSubWindow *subWindow)
{
    // find opened subwindow and delete it
    for(int i = 0; i<radarSubWindowList->count(); i++)
    {
        if(radarSubWindowList->at(i)==subWindow)
        {
            subWindow->close();
            delete subWindow;
            radarSubWindowList->removeAt(i);
            qDebug() << "Sub window successfuly closed.";
            return;
        }
    }
}

void MainWindow::addRadarSubWindow()
{
    if(ui->radarListWidget->currentItem()==NULL) return; // if no item was selected yet

    // if selected item is OPERATOR
    if(ui->radarListWidget->currentItem()->text()==QString("OPERATOR")) { qDebug() << "Cannot create OPERATOR sub window."; return; }

    int selectedRadarId = ui->radarListWidget->currentItem()->data(Qt::UserRole).toInt();

    // If window for this radar already exist, no need to create another one.
    radarSubWindowListMutex->lock();
    for(int i = 0; i<radarSubWindowList->count(); i++) if(radarSubWindowList->at(i)->getRadarId()==selectedRadarId) { qDebug() << "Sub window for this radar unit already exists."; radarSubWindowListMutex->unlock(); return; }
    radarSubWindowListMutex->unlock();

    radarSubWindow * newRadarWindow = new radarSubWindow(selectedRadarId, settings, settingsMutex, radarList, radarListMutex, visualizationColor, visualizationDataMutex, 0);

    // connect close event signal, so main window can safely remove object
    connect(newRadarWindow, SIGNAL(radarSubWindowClosed(radarSubWindow*)), this, SLOT(deleteRadarSubWindow(radarSubWindow*)));

    radarSubWindowListMutex->lock();
    radarSubWindowList->append(newRadarWindow);
    radarSubWindowListMutex->unlock();

    // Connect gridScaleValueUpdate signal to updateObjectsScales slot in case that user globally changes meter to pixel ratio
    connect(this, SIGNAL(gridScaleValueUpdate(double)), newRadarWindow->getVisualizationManager(), SLOT(updateObjectsScales(double)));

    newRadarWindow->show();
}

void MainWindow::showInCentral(int id)
{
    if(ui->radarListWidget->currentItem()==NULL) return; // if no item was selected yet

    int selectedRadarId = 0;
    if(id<0)
    {
        // check which radar is selected in radar list
        selectedRadarId = ui->radarListWidget->currentItem()->data(Qt::UserRole).toInt();
    }
    else selectedRadarId = id;

    if(selectedRadarId<0) selectedRadarId = 0;

    visualizationManager->setActiveRadarId(selectedRadarId);
    visualizationManager->updateRadarMarkerList(radarList, radarListMutex, selectedRadarId);


    // if stackManagerWorker is not NULL we need to update its active radar  variable
    if(stackManagerWorker!=NULL) stackManagerWorker->changeActiveRadarId(selectedRadarId);
}
