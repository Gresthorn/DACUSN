/**
 * @file sceneRendererDialog.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class which provides methods for displaying dialog window with options for visualizations.
 *
 * @section DESCRIPTION
 *
 * All visualizations settings are stored in uwbSettings based object. This dialog is responsible
 * for extracting them out and display for the user. Since some of settings changes need to
 * run some special operations, dialog can inform about the changes via signals like:
 *
 * void renderingEngineChanged(rendering_engine);
 * void periodicalImgBackup(bool);
 * void realTimeRecordingStatus(bool);
 *
 * By default options for setting up OpenGL buffers is hidden, since default Qt values are usualy
 * acceptable. However, dialog must allow to unhide these and consider their modifications
 * during saving settings as well.
 *
 */

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

    void showAdvancedOglSlot(void);

    void imageExportPathDialogSlot(void);
    void imagePeriodicalExportPathDialogSlot(void);

signals:
    void renderingEngineChanged(rendering_engine);
    void periodicalImgBackup(bool);
    void realTimeRecordingStatus(bool);
};

#endif // SCENERENDERERDIALOG_H
