#ifndef RADARSUBWINDOW_H
#define RADARSUBWINDOW_H

#include <QDialog>

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
    explicit radarSubWindow(QWidget *parent = 0);
    ~radarSubWindow();

private:
    Ui::radarSubWindow *ui;
};

#endif // RADARSUBWINDOW_H
