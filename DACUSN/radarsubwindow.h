#ifndef RADARSUBWINDOW_H
#define RADARSUBWINDOW_H

#include <QDialog>
#include <QVector>
#include <QList>
#include <QMutex>
#include <QDebug>

#include "uwbsettings.h"
#include "stddefs.h"
#include "radar_handler.h"
#include "visualization.h"

/**
 * @file radarsubwindow.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class is equipped with all stuff needed for displaying just one radar view.
 *
 * @section DESCRIPTION
 *
 * While the 'mainwindow' contains the basic/central view widget, the user sometimes may want to see
 * what can radar units individually be observing. This can be interesting or usefull when one or more
 * radar units can observe targets which another units cannot. This ability must be done in separate window
 * as far as user might want to display more than one radar unit at once. This class provides basic interface
 * by which the main window thread can provide such information. Each widget needs to be equipped with
 * its own 'animationManager' object as well as 'radarView' and 'radarScene' objects. Also slots for stop, pause,
 * run or restart states are required. It is important to mention that this window/widget is not to be
 * the equal to main scene. Therefore there is also no need to apply full functionality of main view object.
 * User can any time use the 'switchToMainView' functionality to display the specific radar unit scene in
 * central view widget. This widget is designed to provide quick and simple information about what specific
 * radar unit can see.
 *
 */

namespace Ui {
class radarSubWindow;
}

class radarSubWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief This constructor takes pointers to important stuff like settings object as parameters and holds them for later use. 'radar_List' is used for finding the correct radar unit which values should be displayed.
     * @param[in] setts Pointer to the settings object.
     * @param[in] settings_mutex Mutex protecting 'settings' objec.
     * @param[in] radar_List The list of all radar units availible.
     * @param[in] radar_List_Mutex Mutex protecting 'radarList'.
     * @param[in] visualization_Color Is pointer to the color vector. For now used original vector because it is updated during processing. It is supposed that always has enough colors for all targets.
     * @param[in] visualization_Data_Mutex Is needed because of 'visualization_Color' list. NOTE: SHOULD BE CONSIDERED ANOTHER SOLUTION BECAUSE WITH MANY SUBWINDOWS, TOO MANY MUTEX LOCKS CAN REALLY SLOW DOWN APPLICATION.
     * @param[in] parent Pointer to the parent widget/object. Should be NULL or zero if dialog should be displayed as a separate window.
     */
    explicit radarSubWindow(int radar_Id, uwbSettings * setts, QMutex * settings_mutex, QVector<radar_handler * > * radar_List, QMutex * radar_List_Mutex, QList<QColor * > * visualization_Color, QMutex * visualization_Data_Mutex, QWidget *parent = 0);
    ~radarSubWindow();

    /**
     * @brief Function returns the radar ID of unit which values are being displayed in the window. This is primarily used for higher classes to check if the specific radar unit has already its own subwindow in use.
     * @return The return value is integer number representing the radar unit which values are being rendered here.
     */
    int getRadarId(void) { return radarId; }

private:
    Ui::radarSubWindow *ui;

    int radarId;

    QList<QColor * > * visualizationColor; ///< The colors assigned to all targets
    QMutex * visualizationDataMutex; ///< Mutex protecting visualization data from being accessed by multiple threads at the same time

    uwbSettings * settings; ///< All application settings are stored here
    QMutex * settingsMutex; ///< Mutex protecting settings object

    QVector<radar_handler * > * radarList; ///< Vector of all availible radars
    QMutex * radarListMutex; ///< Mutex protecting the 'radarList' from multithread access

    radarUnit * thisRadarUnit; ///< Pointer to the radar unit which values should be displayed in this dialog. Allows direct access to the unit's resources.

    animationManager * thisVisualizationManager; ///< This window's own 'animationManager' object.
    radarScene * thisVisualizationScene; ///< This window's own scene.
    radarView * thisVisualizationView; ///< This window's own view.

    QList<QPointF * > * thisVisualizationData; ///< Own visualizationData list. Must be filled by higher classes.
};

#endif // RADARSUBWINDOW_H
