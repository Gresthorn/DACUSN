/**
 * @file backupoptionsdialog.cpp
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Definition of backupOptionsDialog class methods is placed here
 *
 * @section DESCRIPTION (see backupoptionsdialog.h)
 *
 * The following dialog is using the uwbSettings based object to load all settings needed for backup
 * sequence to store data in text file on disk. Dialog provides graphical interface for setting up
 * paths and filename of target txt. Availible is also checkbox for allowing or disabling this functionality.
 *
 */


#include "backupoptionsdialog.h"
#include "ui_backupoptionsdialog.h"

backupOptionsDialog::backupOptionsDialog(uwbSettings *setts, QMutex *settings_mutex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::backupOptionsDialog)
{
    ui->setupUi(this);

    settings = setts;
    settingsMutex = settings_mutex;

    settingsMutex->lock();

    backupFileName.append(settings->getBackupFileName());

    ui->filenameLineEdit->setText(backupFileName);
    ui->backupFileLineEdit->setText(settings->getDiskBackupFilePath());
    ui->enableDataBackupCheckBox->setChecked(settings->getDiskBackupEnabled());

    settingsMutex->unlock();

    connect(ui->changePathButton, SIGNAL(clicked()), this, SLOT(changeFilePathSlot()));
    connect(this, SIGNAL(accepted()), this, SLOT(acceptedSlot()));
}

backupOptionsDialog::~backupOptionsDialog()
{
    delete ui;
}

void backupOptionsDialog::changeFilePathSlot()
{
    QString backupPath(ui->backupFileLineEdit->text());

    QString dir = QFileDialog::getExistingDirectory(this, tr("Select target directory for backup file"), backupPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dir.isEmpty()) { ui->backupFileLineEdit->setText(backupPath); return; }

    ui->backupFileLineEdit->setText(dir);
}

void backupOptionsDialog::acceptedSlot()
{
    settingsMutex->lock();

    if(ui->filenameLineEdit->text().isEmpty()) settings->setBackupFileName(backupFileName);
    else settings->setBackupFileName(ui->filenameLineEdit->text());

    settings->setDiskBackupFilePath(ui->backupFileLineEdit->text());
    settings->setDiskBackupEnabled(ui->enableDataBackupCheckBox->isChecked());

    settingsMutex->unlock();

    if(ui->filenameLineEdit->text().isEmpty()) QMessageBox::information(this, tr("Null file name"), tr("The file name line is empty. The lastly known file name will be used instead."), QMessageBox::Ok);

    emit acceptedSignal();
}
