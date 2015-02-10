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



    /* ---------------------------------- THREADS ------------------------------------- */

    dataInput = new dataInputThread(this, dataStack, dataStackMutex, settings, settingsMutex);
    // setting the maximum priority to ensure that data will not be lost because of processing the old one
    dataInput->start(QThread::TimeCriticalPriority);

    this->setWindowTitle(tr("Centrum asociácie dát v UWB sensorovej sieti"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
