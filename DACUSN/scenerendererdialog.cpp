#include "scenerendererdialog.h"
#include "ui_scenerendererdialog.h"

sceneRendererDialog::sceneRendererDialog(uwbSettings * setts, QMutex * settings_mutex, radarScene * scene, radarView * view, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sceneRendererDialog)
{
    ui->setupUi(this);

    settings = setts;
    settingsMutex = settings_mutex;

    visualizationView = view;
    visualizationScene = scene;

    settingsMutex->lock();

    gridOneColor = new QColor(*settings->getGridOneColor());
    gridTwoColor = new QColor(*settings->getGridTwoColor());
    gridThreeColor = new QColor(*settings->getGridThreeColor());
    backgroundColor = new QColor(*settings->getBackgroundColor());

    ui->gridOneCheckBox->setChecked(settings->gridOneIsEnabled());
    ui->gridTwoCheckBox->setChecked(settings->gridTwoIsEnabled());
    ui->gridThreeCheckBox->setChecked(settings->gridThreeIsEnabled());
    ui->backgroundCheckBox->setChecked(settings->backgroundIsEnabled());
    ui->allowSmoothTransitionsCheckBox->setChecked(settings->getSmoothTransitions());

    ui->backgroundRenderingComboBox->setCurrentIndex(settings->getTappingRenderMethod());

    ui->targetDisplayMethodComboBox->setCurrentIndex(settings->getVisualizationSchema());

    ui->renderingEngineComboBox->setCurrentIndex(settings->getRenderingEngine());

    ui->realTimeRecordingCheckBox->setChecked(settings->getHistoryPath());

    // OpenGL settings

    if(settings->oglGetBufferType() == DOUBLE_BUFFERING) ui->doubleBufferingRadio->setChecked(true);
    else if(settings->oglGetBufferType() == SINGLE_BUFFERING) ui->singleBufferingRadio->setChecked(true);

    if(settings->getVisualizationSchema()==PATH_HISTORY)
    {
        // disable few widgets that are not allowed to modify during PATH_HISTORY mode
        ui->renderingEngineComboBox->setDisabled(true);
        ui->backgroundRenderingComboBox->setDisabled(true);
        ui->allowSmoothTransitionsCheckBox->setDisabled(true);
    }
    else
    {
        // enable widgets that could been disabled because of PATH_HISTORY mode
        ui->renderingEngineComboBox->setDisabled(false);
        ui->backgroundRenderingComboBox->setDisabled(false);
        ui->allowSmoothTransitionsCheckBox->setDisabled(false);
    }

    ui->directRenderingCheckBox->setChecked(settings->oglGetDirectRendering());
    ui->depthBufferCheckBox->setChecked(settings->oglGetDepthBuffer());
    ui->accumulationBufferCheckBox->setChecked(settings->oglGetAccumulationBuffer());
    ui->stencilBufferCheckBox->setChecked(settings->oglGetStencilBuffer());
    ui->multisampleBuffersCheckBox->setChecked(settings->oglGetMultisampleBuffer());

    ui->redSpinBox->setValue(settings->oglGetRedBufferSize());
    ui->greenSpinBox->setValue(settings->oglGetGreenBufferSize());
    ui->blueSpinBox->setValue(settings->oglGetBlueBufferSize());
    ui->alphaSpinBox->setValue(settings->oglGetAlphaBufferSize());
    ui->depthBufferSpinBox->setValue(settings->oglGetDepthBufferSize());
    ui->accumulationBufferSpinBox->setValue(settings->oglGetAccumulationBufferSize());
    ui->stencilBufferSpinBox->setValue(settings->oglGetStencilBufferSize());
    ui->multisamplesBuffersSpinBox->setValue(settings->oglGetMultisampleBufferSize());
    ui->swapIntervalSpinBox->setValue(settings->oglGetSwapInterval());

    settingsMutex->unlock();

    //grid colors select
    connect(ui->colorPickerGridOne, SIGNAL(clicked()), SLOT(colorSelectGridOneSlot()));

    connect(ui->colorPickerGridTwo, SIGNAL(clicked()), SLOT(colorSelectGridTwoSlot()));

    connect(ui->colorPickerGridThree, SIGNAL(clicked()), SLOT(colorSelectGridThreeSlot()));

    connect(ui->backgroundColorButton, SIGNAL(clicked()), SLOT(colorSelectionBackgroundSlot()));

    connect(this, SIGNAL(accepted()), this, SLOT(accepted()));
}

sceneRendererDialog::~sceneRendererDialog()
{
    delete ui;
}

void sceneRendererDialog::accepted()
{
    settingsMutex->lock();

    delete settings->getGridOneColor();
    delete settings->getGridTwoColor();
    delete settings->getGridThreeColor();

    settings->setGridOneColor(gridOneColor);
    settings->setGridTwoColor(gridTwoColor);
    settings->setGridThreeColor(gridThreeColor);
    settings->setBackgroundColor(backgroundColor);

    settings->setGridOneEnable(ui->gridOneCheckBox->isChecked());
    settings->setGridTwoEnable(ui->gridTwoCheckBox->isChecked());
    settings->setGridThreeEnable(ui->gridThreeCheckBox->isChecked());
    settings->setBackgroundColorEnable(ui->backgroundCheckBox->isChecked());
    settings->setSmootheTransitions(ui->allowSmoothTransitionsCheckBox->isChecked());

    settings->setHistoryPath(ui->realTimeRecordingCheckBox->isChecked());

    settings->setTappingRenderMethod((visualization_tapping_options)(ui->backgroundRenderingComboBox->currentIndex()));

    settings->setVisualizationSchema((visualization_schema)(ui->targetDisplayMethodComboBox->currentIndex()));

    if(ui->doubleBufferingRadio->isChecked()) settings->oglSetBufferType(DOUBLE_BUFFERING);
    else if(ui->singleBufferingRadio->isChecked()) settings->oglSetBufferType(SINGLE_BUFFERING);

    settings->oglSetDirectRendering(ui->directRenderingCheckBox->isChecked());
    settings->oglSetDepthBuffer(ui->depthBufferCheckBox->isChecked());
    settings->oglSetAccumulationBuffer(ui->accumulationBufferCheckBox->isChecked());
    settings->oglSetStencilBuffer(ui->stencilBufferCheckBox->isChecked());
    settings->oglSetMultisampleBuffer(ui->multisampleBuffersCheckBox->isChecked());

    settings->oglSetRedBufferSize(ui->redSpinBox->value());
    settings->oglSetGreenBufferSize(ui->greenSpinBox->value());
    settings->oglSetBlueBufferSize(ui->blueSpinBox->value());
    settings->oglSetAlphaBufferSize(ui->alphaSpinBox->value());
    settings->oglSetDepthBufferSize(ui->depthBufferSpinBox->value());
    settings->oglSetAccumulationBufferSize(ui->accumulationBufferSpinBox->value());
    settings->oglSetStencilBufferSize(ui->stencilBufferSpinBox->value());
    settings->oglSetMultisampleBufferSize(ui->multisamplesBuffersSpinBox->value());
    settings->oglSetSwapInterval(ui->swapIntervalSpinBox->value());


    settings->setRenderingEngine((rendering_engine)(ui->renderingEngineComboBox->currentIndex()));

    settingsMutex->unlock();

    emit renderingEngineChanged((rendering_engine)(ui->renderingEngineComboBox->currentIndex()));
}

void sceneRendererDialog::colorSelectGridTwoSlot()
{
    QColor * temp = gridTwoColor;
    gridTwoColor = new QColor(QColorDialog::getColor(*gridTwoColor, this, tr("Select color for grid #2")));
    if(gridTwoColor) delete temp;
    else gridTwoColor = temp;
}

void sceneRendererDialog::colorSelectGridThreeSlot()
{
    QColor * temp = gridThreeColor;
    gridThreeColor = new QColor(QColorDialog::getColor(*gridThreeColor, this, tr("Select color for grid #3")));
    if(gridThreeColor) delete temp;
    else gridThreeColor = temp;
}

void sceneRendererDialog::colorSelectGridOneSlot()
{
    QColor * temp = gridOneColor;
    gridOneColor = new QColor(QColorDialog::getColor(*gridOneColor, this, tr("Select color for grid #1")));
    if(gridOneColor) delete temp;
    else gridOneColor = temp;
}

void sceneRendererDialog::colorSelectionBackgroundSlot()
{
    QColor * temp = backgroundColor;
    backgroundColor = new QColor(QColorDialog::getColor(*backgroundColor, this, tr("Select color for scene background")));
    if(backgroundColor) delete temp;
    else backgroundColor = temp;
}
