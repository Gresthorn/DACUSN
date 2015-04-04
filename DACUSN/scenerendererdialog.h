#ifndef SCENERENDERERDIALOG_H
#define SCENERENDERERDIALOG_H

#include <QDialog>
#include <QMutex>
#include <QColorDialog>
#include <QColor>
#include <QDebug>
#include <QFileDialog>

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
    QColor * backgroundColor;

    Ui::sceneRendererDialog *ui;

    QString imageExportPath;
    QString imagePeriodicalExportPath;


private slots:
    void accepted(void);

    void colorSelectGridOneSlot(void);
    void colorSelectGridTwoSlot(void);
    void colorSelectGridThreeSlot(void);
    void colorSelectionBackgroundSlot(void);

    void imageExportPathDialogSlot(void);
    void imagePeriodicalExportPathDialogSlot(void);

signals:
    void renderingEngineChanged(rendering_engine);
    void periodicalImgBackup(bool);
};

#endif // SCENERENDERERDIALOG_H
