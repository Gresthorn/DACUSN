/**
 * @file mttsettingsdialog.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief This class provides simple GUI for MTT algorithm setup.
 *
 * @section DESCRIPTION
 *
 * Class mttsettingsdialog is simple class for establishing GUI allowing user to
 * allow or disable MTT algorithms in program. This dialog can be enhanced of some
 * other options/edit abilities later if needed.
 */

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

signals:
    void restartMTT(void);
};

#endif // MTTSETTINGSDIALOG_H
