#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // create settings object filled with default values
    settings = new uwbSettings();
    // creating protector mutex for settings
    settingsMutex = new QMutex;

    dataStack = new QList<rawData * >;
    dataStackMutex = new QMutex;

    this->setWindowTitle(tr("Centrum asociácie dát v UWB sensorovej sieti"));

    /* ------------------------------------------------- THREADS ------------------------------------------------- */


    /* ------------------------------------------------- DATA RECIEVING ------------------------------------------ */
    dataInputThread = NULL;
    dataInputWorker = NULL;

    connect(ui->pauseUnpause, SIGNAL(clicked()), this, SLOT(pauseDataInputSlot()));
    connect(ui->stop, SIGNAL(clicked()), this, SLOT(destroyDataInputThreadSlot()));
    connect(ui->Restart, SIGNAL(clicked()), this, SLOT(restartDataInputSlot()));
    connect(ui->start, SIGNAL(clicked()), this, SLOT(establishDataInputThreadSlot()));

    /* ------------------------------------------------- DATA RECIEVING ------------------------------------------ */

    /* ------------------------------ SIGNALS and SLOTS ------------------------------- */

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::pauseDataInputSlot()
{
    // if the pointers are NULL, no thread or object was created what means either error or no running thread
    if(dataInputWorker==NULL || dataInputWorker==NULL)
    {
        qDebug() << "The data recieving thread seems not running. Nothing to pause.";
    }
    else dataInputWorker->switchPauseState();
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
