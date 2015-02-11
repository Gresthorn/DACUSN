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

    /* ---------------------------------- THREADS ------------------------------------- */

    dataInputThread = NULL;
    dataInputWorker = NULL;
    establishDataInputThread();

    /* ------------------------------ SIGNALS and SLOTS ------------------------------- */

    connect(ui->pauseUnpause, SIGNAL(clicked()), this, SLOT(pauseDataInputSlot()));
    connect(ui->stop, SIGNAL(clicked()), this, SLOT(stopDataInputSlot()));
    connect(ui->Restart, SIGNAL(clicked()), this, SLOT(restartDataInputSlot()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::pauseDataInputSlot()
{
    dataInputWorker->switchPauseState();
}

void MainWindow::restartDataInputSlot()
{
    // kill old thread
    destroyDataInputThread();

    // start new thread
    establishDataInputThread();

    qDebug() << "Data recieving restarted...";
}

void MainWindow::stopDataInputSlot()
{
    // kill the old thread
    destroyDataInputThread();

    qDebug() << "Data recieving stopped...";
}

void MainWindow::establishDataInputThread()
{
    // create wait for condition object so it will be able to pause thread if needed
    dataInputWorker = new dataInputThreadWorker(dataStack, dataStackMutex, settings, settingsMutex);
    // setting the maximum priority to ensure that data will not be lost because of processing the old one
    dataInputThread = new QThread(this);
    dataInputWorker->moveToThread(dataInputThread);
    dataInputThread->start(QThread::TimeCriticalPriority);
    QMetaObject::invokeMethod(dataInputWorker, "runWorker", Qt::QueuedConnection);

    qDebug() << "Data recieving thread started...";
}

void MainWindow::destroyDataInputThread()
{
    // NULL condition is used, just in case
    if(dataInputWorker!=NULL) dataInputWorker->stopWorker();

    // wait until worker is stopped properly
    while(!dataInputWorker->checkStoppedStatus()) continue;

    // closing thread
    if(dataInputThread!=NULL) dataInputThread->quit();

    if(dataInputThread!=NULL && dataInputWorker!=NULL)
    {
        delete dataInputWorker;
        delete dataInputThread;
    }

    qDebug() << "Data recieving thread cancelled...";
}
