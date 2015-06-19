/**
 * @file scenerendererdialog.cpp
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Definitions of sceneRendererDialog class methods.
 *
 * @section DESCRIPTION
 *
 * This is another simple dialog window class specialized for stack management. It allows user
 * to modify basic settings of stackmanager thread algorithms. The pointer to settings object is
 * passed into the constructor and for accessing its methods and variables, mutex is passed as well.
 *
 */

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
