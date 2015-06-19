/**
 * @file dataInputDialog.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class for establishing object allowing user to set up new parameters of input sequence.
 *
 * @section DESCRIPTION
 *
 * The dataInputDialog class inherits a QDialog class so it is able to build a graphical interface
 * for dialog window. This dialog provides interface for changing/setting up parameters of data
 * input sequence. Note that all data must be loaded first from the uwbSettings based object and
 * after confirmation of changes save them back into the settings object. Since that is used accross
 * the application, mutex is neccessary to be locked.
 */

#ifndef DATAINPUTDIALOG_H
#define DATAINPUTDIALOG_H

#include <QDialog>
#include <QMutex>
#include <QDebug>
#include <QAbstractButton>
#include <QtSerialPort>

#include "uwbsettings.h"

namespace Ui {
class dataInputDialog;
}

class dataInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit dataInputDialog(uwbSettings * setts, QMutex * settings_mutex, QWidget *parent = 0);
    ~dataInputDialog();    

private:
    Ui::dataInputDialog *ui;

    uwbSettings * settings;
    QMutex * settingsMutex;

    int baudrateLoaded; // loaded baudrate from settings (can be used to restore old baudrate if needed)
    int portIndexLoaded; // loaded port index from settings (can be used to restore old port index if needed)

private slots:
    void accepted(void);
    void methodSelectionChangedSlot(QAbstractButton * button);
};

#endif // DATAINPUTDIALOG_H
