#ifndef DATAINPUTDIALOG_H
#define DATAINPUTDIALOG_H

#include <QDialog>
#include <QMutex>
#include <QDebug>

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

private slots:
    void accepted(void);
};

#endif // DATAINPUTDIALOG_H
