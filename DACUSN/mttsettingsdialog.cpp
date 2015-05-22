#include "mttsettingsdialog.h"
#include "ui_mttsettingsdialog.h"

mttsettingsdialog::mttsettingsdialog(uwbSettings *setts, QMutex *settings_mutex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mttsettingsdialog)
{
    ui->setupUi(this);

    settings = setts;
    settingsMutex = settings_mutex;

    settingsMutex->lock();

    ui->mttGlobalCheckBox->setChecked(settings->getGlobalRadarMTT());
    ui->mttPerSingleRadarUnitCheckBox->setChecked(settings->getSingleRadarMTT());

    settingsMutex->unlock();

    connect(this, SIGNAL(accepted()), this, SLOT(acceptedSlot()));
}

mttsettingsdialog::~mttsettingsdialog()
{
    delete ui;
}

void mttsettingsdialog::acceptedSlot()
{
    settingsMutex->lock();

    settings->setSingleRadarMTT(ui->mttPerSingleRadarUnitCheckBox->isChecked());
    settings->setGlobalRadarMTT(ui->mttGlobalCheckBox->isChecked());

    settingsMutex->unlock();
}
