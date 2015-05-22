#ifndef MTTSETTINGSDIALOG_H
#define MTTSETTINGSDIALOG_H

#include <QDialog>
#include <QMutex>

#include "uwbsettings.h"

namespace Ui {
class mttsettingsdialog;
}

class mttsettingsdialog : public QDialog
{
    Q_OBJECT

public:
    explicit mttsettingsdialog(uwbSettings * setts, QMutex * settings_mutex, QWidget *parent = 0);
    ~mttsettingsdialog();

private:
    Ui::mttsettingsdialog *ui;

    uwbSettings * settings;
    QMutex * settingsMutex;

public slots:
    void acceptedSlot(void);
};

#endif // MTTSETTINGSDIALOG_H
