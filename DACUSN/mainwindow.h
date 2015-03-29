#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QTimer>
#include <QPointF>
#include <QGraphicsEllipseItem>
#include <QGLWidget>

#include <QDebug>

#include <QInputDialog>
#include "datainputdialog.h"
#include "stackmanagerdialog.h"
#include "radarlistdialog.h"
#include "scenerendererdialog.h"
#include "coordinatesinputdialog.h"

#include "reciever.h"
#include "rawdata.h"
#include "stddefs.h"
#include "uwbsettings.h"
#include "datainputthreadworker.h"
#include "stackmanager.h"
#include "radarunit.h"
#include "radar_handler.h"
#include "visualization.h"

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

    radarScene * visualizationScene; ///< Is the scene where all items/objects are rendered on
    radarView * visualizationView; ///< The view widget where the viewport of 'visualizationScene' will be placed;
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



private:
    Ui::MainWindow *ui;

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

    // HELPER VARIABLES
};

#endif // MAINWINDOW_H
