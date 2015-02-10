#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    reciever rcvr(SYNTHETIC);

    qDebug() << rcvr.check_status_message();
    qDebug() << rcvr.calibration_status();

    //int counter = 0;

    /*if(rcvr.calibration_status())
    {
        rawData * rd_data;
        while(rd_data!=NULL)
        {
            counter++;
            //if(counter==70) rcvr.set_new_method_code(UNDEFINED);

            rd_data = rcvr.listen();

            if(rd_data!=NULL) qDebug() << rd_data->getSyntheticTime();
            else { qDebug() << rcvr.check_status_message(); break; }

            delete rd_data;

        }
    }*/

    this->setWindowTitle(tr("Centrum asociácie dát v UWB sensorovej sieti"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
