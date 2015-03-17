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

    ui->gridOneCheckBox->setChecked(settings->gridOneIsEnabled());
    ui->gridTwoCheckBox->setChecked(settings->gridTwoIsEnabled());
    ui->gridThreeCheckBox->setChecked(settings->gridThreeIsEnabled());

    ui->backgroundRenderingComboBox->setCurrentIndex(settings->getTappingRenderMethod());

    ui->targetDisplayMethodComboBox->setCurrentIndex(settings->getVisualizationSchema());

    settingsMutex->unlock();


    //grid colors select
    connect(ui->colorPickerGridOne, SIGNAL(clicked()), SLOT(colorSelectGridOneSlot()));

    connect(ui->colorPickerGridTwo, SIGNAL(clicked()), SLOT(colorSelectGridTwoSlot()));

    connect(ui->colorPickerGridThree, SIGNAL(clicked()), SLOT(colorSelectGridThreeSlot()));

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

    settings->setGridOneEnable(ui->gridOneCheckBox->isChecked());
    settings->setGridTwoEnable(ui->gridTwoCheckBox->isChecked());
    settings->setGridThreeEnable(ui->gridThreeCheckBox->isChecked());

    settings->setTappingRenderMethod((visualization_tapping_options)(ui->backgroundRenderingComboBox->currentIndex()));

    settings->setVisualizationSchema((visualization_schema)(ui->targetDisplayMethodComboBox->currentIndex()));

    settingsMutex->unlock();
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
