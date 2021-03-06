/**
 * @file MainWindow.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief This is the MAIN class of basic GUI window displayed after the program starts.
 *
 * @section DESCRIPTION
 *
 * Class MainWindow provides complex interface for creating and displaying main window
 * evoked immediately after the program starts. The 'show()' function is called in main.cpp
 * before the QApplication object enters its infinite loop in its 'exec()' function.
 *
 * Class also provides signals and slots for communication with other objects in program.
 * Full GUI is defined in mainwindow.ui. From this window, all dialogs are initiated, so
 * class provides appropriately named slots for doing so. Class holds pointers to all worker
 * objects (later passed into separate threads) during their existance. After stopping input
 * algorithms, these pointers should be NULL.
 *
 * EstablishDataInputThreadSlot() initializes/starts the data recieving thread according to
 * availible settings. DestroyDataInputThreadSlot() does the inverse operation.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QTimer>
#include <QPointF>
#include <QGraphicsEllipseItem>
#include <QImage>
#include <QGLWidget>
#include <QElapsedTimer>


#include <QDebug>

#include <QInputDialog>
#include "stddefs.h"
#include "datainputdialog.h"
#include "stackmanagerdialog.h"
#include "radarlistdialog.h"
#include "scenerendererdialog.h"
#include "coordinatesinputdialog.h"
#include "backupoptionsdialog.h"
#include "mttsettingsdialog.h"

#include "reciever.h"
#include "rawdata.h"
#include "uwbsettings.h"
#include "datainputthreadworker.h"
#include "stackmanager.h"
#include "radarunit.h"
#include "radar_handler.h"
#include "visualization.h"
#include "radarsubwindow.h"


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

    radarScene * visualizationScene; ///< Is the scene where all items/objects are rendered on.
    radarView * visualizationView; ///< The view widget where the viewport of 'visualizationScene' will be placed.
    animationManager * visualizationManager; ///< Is the object that handles all methods for different visualization schemas.

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

    /**
     * @brief Starts new thread and begin the infinite recieving loop.
     */
    void establishDataInputRutineSlot(void);

    /**
     * @brief Destroys currently running infinite loop for data recieving from UWB sensor network
     */
    void destroyDataInputRutineSlot(void);

    /**
     * @brief This slot is run periodically to update visual scene with updated positions.
     */
    void visualizationSlot(void);

    /**
     * @brief If the rendering engine was changed in rendering settings via dialog window, this slot ensures safe switching to new engine.
     */
    void renderingEngineChangedSlot(rendering_engine rengine);

    /**
     * @brief This slot is called when from the renderer dialog, openGL engine is detected and flags are required to be set from settings
     */
    void setOglEngineFormatSlot(openGLWidget * oglwidget);

    /**
     * @brief When user changes rotation with dial widget, the scene can be accessed via slot to change the rotation angle value.
     * @param[in] angle Is the new rotation angle in degrees.
     * @param[in] animation Is parameter specifying if the angle is being changed from animation sequence. If yes, the value is set to true and the slot will not use 'centerOn' function.
     */
    void sceneRotationChangedSlot(int angle);

    /**
     * @brief Is slot evoked after user clicks the button for manual rotation angle select.
     */
    void sceneRotationManualChangeSlot(void);

    /**
     * @brief This slot is called when user clicks appropriate button to move center of the scene on specific [x, y] point.
     */
    void sceneMoveToXYSlot(void);

    /**
     * @brief Center view to [0, 0] coordinates and restores the view of angle to zero.
     */
    void centerToZeroSlot(void);

    /**
     * @brief This slot is evoked when user select/unselect drawing path history.
     */
    void pathHistoryShow(void);

    /**
     * @brief Slot will start/ends timer and creates/deletes QTimer object if the periodical image export is enabled/disabled.
     * @param[in] enabled Specifies the periodical image export status.
     */
    void periodicalImgBackupSlot(bool enabled);

    /**
     * @brief This slot is called after user clicks the checkable rendering button in basic scene controls list. Function will negotiate the current rendering status.
     */
    void enableRenderingSlot(void);

    /**
     * @brief This slot is evoked by timer each second to update time measurement labels.
     */
    void timeMeasureSlot(void);

    /**
     * @brief Following slot will take updated data and converts them into string while updating information table.
     */
    void informationTableUpdateSlot(void);

    /**
     * @brief If radar list is updated, this slot will update the list widget in front panel.
     */
    void radarListUpdated(void);

    /**
     * @brief Returns all disk backup dependencies to their initial state. Deletes all objects and close file used for backup.
     */
    void deleteDiskBackupDependencies(void);

    /**
     * @brief Function will iterate over all radar units and restarts their MTT algorithms to initial state.
     */
    void resetAllMTTs(void);

    /**
     * @brief Function will reset the MTT of single radar unit. If you want to reset MTT of radar at specific index, set ID to id<=0 and give the index as second argument.
     * @param[in] id Id of radar which MTT should be restarted.
     * @param[in] index Index of radar unit in radarList, which MTT will be restarted.
     */
    void resetMTTat(int id, int index = -1);

protected:

    void closeEvent(QCloseEvent *event);

private slots:

    /**
     * @brief Opens dialog window for data input method management.
     */
    void openDataInputDialog(void);

    /**
     * @brief Opens dialog window for stack management setting.
     */
    void openStackManagerDialog(void);

    /**
     * @brief Opens dialog window for radar units list management settings.
     */
    void openRadarListDialog(void);

    /**
     * @brief Opens dialog window for setting related to scene backgrounds/grids and rendering options.
     */
    void openSceneRendererDialog(void);

    /**
     * @brief Opens dialog window for setting the data backup. The backup target file is set up here and enable/disable option is added.
     */
    void openDataBackupDialog(void);

    /**
     * @brief Opens dialog window for setting the MTT algorithm and its parameters, if this configurable attitude is allowed.
     */
    void openMTTSettingsDialog(void);

    /**
     * @brief After clicking the 'export' controll button, this slot will ensure that present view will be exported in directory at in settings specified location.
     */
    void exportViewImageSlot(void);

    /**
     * @brief If the user selects the periodical image backup option, specalized timer is created, which will evoke this slot for exporting image from current view.
     */
    void periodicalExportViewImageSlot(void);

    /**
     * @brief Deletes radar subwindow when is closed.
     * @param[in] subWindow Is the pointer to subwindow. This is passed when subwindow is closed, so mainwindow can find it in list and delete record.
     */
    void deleteRadarSubWindow(radarSubWindow * subWindow);

    /**
     * @brief Creates new subWindow and adds it into list.
     */
    void addRadarSubWindow(void);

    /**
     * @brief Sets new radar data to be displayed in central view.
     */
    void showInCentral(int id = -1);

    /**
     * @brief This slot ensures creation of file handlers and streams for the write into file sequence when user desires saving all recieved data from all radars. If backup is not enabled, frees all memory reserved by file handlers/streams etc.
     */
    void manageDiskBackupSlot(void);

    /**
     * @brief The following slot is called when user accepts new settings in renderer dialog. Real time recording current status is passed as parameter. If true, function will disable option for displaying another radar data in central view.
     * @param[in] status Will recieve new status of real time recording setting.
     */
    void realTimeRecordingChanged(bool status);

    /**
     * @brief This function reacts on slider changes in main window and change global METER_TO_PIXEL_RATIO variable to new scale value.
     */
    void gridScaleValueChanged(int m_to_pix_ratio);

signals:

    /**
     * @brief This signal should be connected to all animation managers (local or global) and inform them about user action, which changes METER_TO_PIXEL_RATIO global variable, so all positions of all items should be updatet.
     * @param old_grid_scale_value
     */
    void gridScaleValueUpdate(double old_grid_scale_value);

private:

    /**
     * @brief This function can convert time expressed in miliseconds to QString in format hh:mm:ss
     * @param[in] timems Time expressed in miliseconds.
     * @return Time in readable QString form.
     */
    QString timeToString(qint64 timems);

    Ui::MainWindow *ui;

    QList<radarSubWindow * >  * radarSubWindowList; ///< Lists all radar subwindows displaying data from specific radar unit in current use.
    QMutex * radarSubWindowListMutex; ///< Mutex protecting sub windows during update seqence.

    QTimer * pauseBlinkEffect; ///< Blink effect is used to warn user that the data inpu is paused
    bool blinker; ///< Boolean value which is changed periodically when 'pauseBlinkEffect' timer emits signal. Used for changing pause action button style.
    dataInputThreadWorker * dataInputWorker; ///< The worker object doing all stuff around the data recieving
    QThread * dataInputThread; ///< The main thread where 'dataInputThreadWorker' may run

    QTimer * visualizationTimer; ///< Emits signals periodically so scene can update with new values
    visualization_schema * visualizationSchema; ///< Handles the user choice of how the targets should be displayed;

    stackManager * stackManagerWorker; ///< Object with infinite cycle managing the stack
    QThread * stackManagerThread; ///< Thread where 'stackManagerWorker' object can run

    QVector<radar_handler * > * radarList; ///< Vector of all availible radars
    QMutex * radarListMutex; ///< Mutex protecting the 'radarList' from multithread access

    visualization_schema lastKnownSchema; ///< Is the lastly known visualization schema. Used when user suddenly changes schema during rendering and some clear functions are needed.
    rendering_engine lastKnownEngine; ///< Is the lastly known/used engine for view rendering. Used when switching to history drawing so after turning this feature off we can return to previous engine.
    visualization_tapping_options lastKnownTappingOptions; ///< The lastly used tapping option before switching to draw history regime.
    bool lastKnownSmoothTransitionsState; ///< Saves the smooth transitions state for time when user is switched into path drawing mode.

    QTimer * periodicalExportTimer; ///< If user selected the periodical backup, this timer manages 'periodicalExportSlot()' to generate new image.
    QTimer * timerElapsed; ///< This timer each second evokes 'timeMeasureSlot()' to update timer label in user interface.
    QTimer * informationTableTimer; ///< This timer is evoking the 'informationTableUpdateSlot()' aproximately each 5 seconds to update all indicators in information table.

    QElapsedTimer * totalElapsedTimer; ///< This timer is created and started immediately when the program starts. Therefore it indicates the time elapsed from the initializing program.
    QElapsedTimer * instanceElapsedTimer; ///< This timer is started when the measurement/observing starts. When observing is stopped/paused, timer follows this signals as well.
    qint64 instanceTimerAccumulator; ///< Since QElapsedTimer cannot be paused, after pause button is clicked we save currently elapsed time to accumulator and after instance timer restart we still add this value to elapsed time befor converting to string.

    qint64 averageRenderTime; ///< Average time from all rendering iterations since the one measurement instance started.
    qint64 renderIterationCount; ///< Holds the information about how many rendering iteration were done so far.

};

#endif // MAINWINDOW_H
