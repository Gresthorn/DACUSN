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
