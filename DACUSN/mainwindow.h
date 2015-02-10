#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>
#include <QDebug>

#include "reciever.h"
#include "rawdata.h"
#include "stddefs.h"
#include "uwbsettings.h"
#include "datainputthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    uwbSettings * settings; ///< All application settings are stored here
    QMutex * settingsMutex; ///< Mutex protecting settings object

    QList<rawData * > * dataStack; ///< Stack for data recieved by sensor network
    QMutex * dataStackMutex; ///< Mutex protecting dataStack object


    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    dataInputThread * dataInput;
};

#endif // MAINWINDOW_H
