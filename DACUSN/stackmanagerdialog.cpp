#include "stackmanagerdialog.h"
#include "ui_stackmanagerdialog.h"

stackManagerDialog::stackManagerDialog(uwbSettings * setts, QMutex * settings_Mutex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::stackManagerDialog)
{
    ui->setupUi(this);

    settings = setts;
    settingsMutex = settings_Mutex;

    settingsMutex->lock();

    ui->stackRescueCheckBox->setChecked(settings->getStackRescueState());

    ui->controlPeriodicitySpinBox->setValue(settings->getStackControlPeriodicity());
    ui->maximumWarningCountSpinBox->setValue(settings->getMaxStackWarningCount());
    ui->idleTimeSpinBox->setValue(settings->getStackIdleTime());

    settingsMutex->unlock();

    connect(this, SIGNAL(accepted()), this, SLOT(accepted()));
}

stackManagerDialog::~stackManagerDialog()
{
    delete ui;
}

void stackManagerDialog::accepted()
{
    settingsMutex->lock();

    settings->setStackControlPeriodicity(ui->controlPeriodicitySpinBox->value());
    settings->setMaxStackWarningCount(ui->maximumWarningCountSpinBox->value());
    settings->setStackIdleTime(ui->idleTimeSpinBox->value());

    settings->setStackRescueState(ui->stackRescueCheckBox->isChecked());

    settingsMutex->unlock();
}
