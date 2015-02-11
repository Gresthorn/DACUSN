#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include <QDebug>

#include "reciever.h"
#include "rawdata.h"
#include "stddefs.h"
#include "uwbsettings.h"
#include "datainputthreadworker.h"

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

public slots:

    /**
     * @brief This slot is used to pause data recieving thread. If it has been paused already, the thread is unpaused.
     */
    void pauseDataInputSlot(void);

    /**
     * @brief This slot will terminate the running thread and starts new by establishing new connection.
     */
    void restartDataInputSlot(void);

    /**
     * @brief This slot will terminate the running thread.
     */
    void stopDataInputSlot(void);

private:
    Ui::MainWindow *ui;

    dataInputThreadWorker * dataInputWorker; ///< The worker object doing all stuff around the data recieving
    QThread * dataInputThread; ///< The main thread where 'dataInputThreadWorker' may run

    /**
     * @brief This private function establishes the data reciever thread.
     *
     * This function is used internally to create new objects for threads and data input worker. After thread creation
     * it ensures that all settings and priority is correctly passed and the thread is started.
     */
    void establishDataInputThread(void);

    /**
     * @brief This private function stops and deletes all objects connected with the data recieving thread.
     *
     * This function is used internally, but may be directly be evoked by signal from user by 'stopDataInputSlot'.
     * It removes and deletes all objects connected with main input data thread and stops data recieving.
     */
    void destroyDataInputThread(void);

};

#endif // MAINWINDOW_H
