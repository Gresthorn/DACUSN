#ifndef RADARLISTDIALOG_H
#define RADARLISTDIALOG_H

#include <QDialog>
#include <QMutex>
#include <QVector>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>

#include "uwbsettings.h"
#include "radarunit.h"
#include "radar_handler.h"

namespace Ui {
class radarListDialog;
}

class radarListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit radarListDialog(QVector<radar_handler * > * radar_List, QMutex * radar_List_Mutex, uwbSettings * setts, QMutex * settings_Mutex, QWidget *parent = 0);
    ~radarListDialog();

private:
    Ui::radarListDialog *ui;

    void addRadarUnit(bool enabled, int id, double x_pos, double y_pos, double angle);
    int findFreeId(void);

    uwbSettings * settings;
    QMutex * settingsMutex;

    QVector<radar_handler * > * radarList;
    QMutex * radarListMutex;

    QString rememberedValue;

    enum tableColumns
    {
        ENABLE, ID, X, Y, ANGLE
    };

private slots:
    void cellChangedControlSlot(int row, int column);
    void cellClickedRememberSlot(int row, int column);
    void addRadarUnitSlot(void);
    void rowSelectionSlot(void);
    void deleteRadarUnitSlot(void);
};

#endif // RADARLISTDIALOG_H
