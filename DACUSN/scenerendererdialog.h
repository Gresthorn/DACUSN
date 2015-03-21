#ifndef SCENERENDERERDIALOG_H
#define SCENERENDERERDIALOG_H

#include <QDialog>
#include <QMutex>
#include <QColorDialog>
#include <QColor>
#include <QDebug>

#include "stddefs.h"
#include "uwbsettings.h"
#include "visualization.h"

namespace Ui {
class sceneRendererDialog;
}

class sceneRendererDialog : public QDialog
{
    Q_OBJECT

public:
    explicit sceneRendererDialog(uwbSettings * setts, QMutex * settings_mutex, radarScene * scene, radarView * view, QWidget *parent = 0);
    ~sceneRendererDialog();

private:
    uwbSettings * settings;
    QMutex * settingsMutex;

    radarView * visualizationView;
    radarScene * visualizationScene;

    QColor * gridOneColor;
    QColor * gridTwoColor;
    QColor * gridThreeColor;

    Ui::sceneRendererDialog *ui;

private slots:
    void accepted(void);

    void colorSelectGridOneSlot(void);
    void colorSelectGridTwoSlot(void);
    void colorSelectGridThreeSlot(void);

signals:
    void renderingEngineChanged(rendering_engine);
};

#endif // SCENERENDERERDIALOG_H
