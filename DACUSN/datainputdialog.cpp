#include "datainputdialog.h"
#include "ui_datainputdialog.h"

dataInputDialog::dataInputDialog(uwbSettings * setts, QMutex * settings_mutex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dataInputDialog)
{
    ui->setupUi(this);

    settings = setts;
    settingsMutex = settings_mutex;

    settingsMutex->lock();

    reciever_method temp_method = settings->getRecieverMethod();
    if(temp_method==SYNTHETIC)
    {
        ui->methodSyntheticRadioButton->setChecked(true);
    }
    else
    {
        ui->methodUndefinedRadioButton->setChecked(true);
    }

    ui->recieverIdleTimeSpinBox->setValue(settings->getRecieverIdleTime());
    ui->recieverMaxErrorCountSpinBox->setValue(settings->getMaximumRecieverErrorCount());

    settingsMutex->unlock();

    connect(this, SIGNAL(accepted()), this, SLOT(accepted()));
}

dataInputDialog::~dataInputDialog()
{
    delete ui;
}

void dataInputDialog::accepted()
{
    settingsMutex->lock();

    if(ui->methodSyntheticRadioButton->isChecked()) settings->setRecieverMethod(SYNTHETIC);
    else settings->setRecieverMethod(UNDEFINED);

    settings->setRecieverIdleTime(ui->recieverIdleTimeSpinBox->value());
    settings->setMaximumRecieverErrorCount(ui->recieverMaxErrorCountSpinBox->value());

    settingsMutex->unlock();

    qDebug() << "Setting up new reciever method. Please restart the input thread to apply changes.";
}
