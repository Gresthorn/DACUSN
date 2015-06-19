/**
 * @file backupOptionsDialog.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class for establishing object allowing user to set new target directory, name of backup file and allow backup operations.
 *
 * @section DESCRIPTION
 *
 * The following dialog is using the uwbSettings based object to load all settings needed for backup
 * sequence to store data in text file on disk. Dialog provides graphical interface for setting up
 * paths and filename of target txt. Availible is also checkbox for allowing or disabling this functionality.
 */

#ifndef BACKUPOPTIONSDIALOG_H
#define BACKUPOPTIONSDIALOG_H

#include <QDialog>
#include <QMutex>
#include <QFileDialog>
#include <QMessageBox>

#include "uwbsettings.h"


namespace Ui {
class backupOptionsDialog;
}

class backupOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit backupOptionsDialog(uwbSettings * setts, QMutex * settings_mutex, QWidget *parent = 0);
    ~backupOptionsDialog();

private:
    Ui::backupOptionsDialog *ui;

    QString backupFileName;
    uwbSettings * settings;
    QMutex * settingsMutex;

signals:
    void acceptedSignal(void);

private slots:
    void changeFilePathSlot(void);
    void acceptedSlot(void);
};

#endif // BACKUPOPTIONSDIALOG_H
