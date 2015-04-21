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
