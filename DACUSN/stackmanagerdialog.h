/**
 * @file scenerendererdialog.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class which provides methods for displaying dialog window with options for stack management.
 *
 * @section DESCRIPTION
 *
 * This is another simple dialog window class specialized for stack management. It allows user
 * to modify basic settings of stackmanager thread algorithms. The pointer to settings object is
 * passed into the constructor and for accessing its methods and variables, mutex is passed as well.
 *
 */

#ifndef STACKMANAGERDIALOG_H
#define STACKMANAGERDIALOG_H

#include <QDialog>
#include <QMutex>

#include "uwbsettings.h"

namespace Ui {
class stackManagerDialog;
}

class stackManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit stackManagerDialog(uwbSettings * setts, QMutex * settings_Mutex, QWidget *parent = 0);
    ~stackManagerDialog();

private:
    Ui::stackManagerDialog *ui;

    uwbSettings * settings;
    QMutex * settingsMutex;

private slots:
    void accepted(void);
};

#endif // STACKMANAGERDIALOG_H
