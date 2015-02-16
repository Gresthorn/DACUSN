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

    /* ------------------------------------------------- DATA RECIEVING ------------------------------------------ */

    stackManagerThread = NULL;
    stackManagerWorker = NULL;

    stackManagerThread = new QThread(this);
    stackManagerWorker = new stackManager(dataStack, dataStackMutex, settings, settingsMutex);
    stackManagerWorker->moveToThread(stackManagerThread);
    stackManagerThread->start(QThread::HighestPriority);
    QMetaObject::invokeMethod(stackManagerWorker, "runWorker", Qt::QueuedConnection);

    /* ------------------------------------------------- DATA RECIEVING ------------------------------------------ */
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

        if(pauseBlinkEffect==NULL)
        {
            // start blink effect
            pauseBlinkEffect = new QTimer(this);
            pauseBlinkEffect->setInterval(1000);
            connect(pauseBlinkEffect, SIGNAL(timeout()), this, SLOT(changeDataInputPauseButtonSlot()));
            pauseBlinkEffect->start();
        }
        else
        {
            // stop blink effect
            pauseBlinkEffect->stop();
            delete pauseBlinkEffect;
            pauseBlinkEffect = NULL;
            blinker = true;
            ui->actionPause->setIcon(QIcon(":/mainToolbar/icons/pause.png"));
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
}

void MainWindow::changeDataInputPauseButtonSlot()
{
    if(blinker) ui->actionPause->setIcon(QIcon(":/mainToolbar/icons/play.png"));
    else ui->actionPause->setIcon(QIcon(":/mainToolbar/icons/pause.png"));
    blinker = !blinker; // negate blinker
}
