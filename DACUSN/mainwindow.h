#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QTimer>
#include <QPointF>

#include <QDebug>

#include "reciever.h"
#include "rawdata.h"
#include "stddefs.h"
#include "uwbsettings.h"
#include "datainputthreadworker.h"
#include "stackmanager.h"
#include "radarunit.h"
#include "radar_handler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    uwbSettings * settings; ///< All application settings are stored here
    QMutex * settingsMutex; ///< Mutex protecting settings object

    QVector<rawData * > * dataStack; ///< Stack for data recieved by sensor network
    QMutex * dataStackMutex; ///< Mutex protecting dataStack object

    QList<QPointF * > * visualizationData; ///< The final positions of targets
    QList<QColor * > * visualizationColor; ///< The colors assigned to all targets
    QMutex * visualizationDataMutex; ///< Mutex protecting visualization data from being accessed by multiple threads at the same time

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
     * @brief This private function establishes the data reciever thread.
     *
     * This function is used internally to create new objects for threads and data input worker. After thread creation
     * it ensures that all settings and priority is correctly passed and the thread is started.
     */
    void establishDataInputThreadSlot(void);

    /**
     * @brief This private function stops and deletes all objects connected with the data recieving thread.
     *
     * This function is used internally, but may be directly be evoked by signal from user by 'stopDataInputSlot'.
     * It removes and deletes all objects connected with main input data thread and stops data recieving.
     */
    void destroyDataInputThreadSlot(void);

    /**
     * @brief This slot is called if 'pauseBlinkEffect' timer signal is emitted to change the button style to warn youser about pause state.
     */
    void changeDataInputPauseButtonSlot(void);

    /**
     * @brief Starts the new stack management thread.
     */
    void establishStackManagementThread(void);

    /**
     * @brief Destroys currently running stack management thread but before deleting all data from stack are read first
     */
    void destroyStackManagementThread(void);

private:
    Ui::MainWindow *ui;

    QTimer * pauseBlinkEffect; ///< Blink effect is used to warn user that the data inpu is paused
    bool blinker; ///< Boolean value which is changed periodically when 'pauseBlinkEffect' timer emits signal. Used for changing pause action button style.
    dataInputThreadWorker * dataInputWorker; ///< The worker object doing all stuff around the data recieving
    QThread * dataInputThread; ///< The main thread where 'dataInputThreadWorker' may run

    stackManager * stackManagerWorker; ///< Object with infinite cycle managing the stack
    QThread * stackManagerThread; ///< Thread where 'stackManagerWorker' object can run

    QVector<radar_handler * > * radarList; ///< Vector of all availible radars
    QMutex * radarListMutex; ///< Mutex protecting the 'radarList' from multithread access
};

#endif // MAINWINDOW_H
