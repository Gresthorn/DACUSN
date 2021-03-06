/**
 * @file mttsettingsdialog.cpp
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Definitions of mttSettingsDialog class methods.
 *
 * @section DESCRIPTION
 *
 * Class mttsettingsdialog is simple class for establishing GUI allowing user to
 * allow or disable MTT algorithms in program. This dialog can be enhanced of some
 * other options/edit abilities later if needed.
 */

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

    if(ui->mttPerSingleRadarUnitCheckBox->isChecked() && !settings->getSingleRadarMTT())
    {
        // If MTT is selected as enabled, but it was not, user is starting new MTT.
        // Since this can happen in situation that MTT was configured and turned off
        // again, we need to restart it so default values are set again. If this is not
        // done, incorrect results may occure.
        emit restartMTT();
    }

    settings->setSingleRadarMTT(ui->mttPerSingleRadarUnitCheckBox->isChecked());
    settings->setGlobalRadarMTT(ui->mttGlobalCheckBox->isChecked());

    settingsMutex->unlock();
}
