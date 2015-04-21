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
