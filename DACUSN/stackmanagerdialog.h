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
