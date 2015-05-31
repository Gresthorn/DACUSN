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

    // load availible COM ports and compare with currently set com port index
    QSerialPortInfo portInfo;
    QList<QSerialPortInfo> portList = portInfo.availablePorts();
    int portIndexFoundItemPosition = -1;
    portIndexLoaded = settings->getComPortNumber();
    for(int i=0; i<portList.count(); i++)
    {
        // find port index in port name
        QRegExp portParser(QString("(\\d+)"));
        QString portName = portList.at(i).portName();
        QStringList numbersList;

        int pos = 0;
        while((pos = portParser.indexIn(portName, pos)) != -1)
        {
            numbersList << portParser.cap(1);
            pos += portParser.matchedLength();
        }

        int portIndex = -1;
        if(!numbersList.isEmpty())
        {
            portIndex = numbersList.first().toInt()-1;

            if(portIndex==portIndexLoaded) portIndexFoundItemPosition = i;
        }

        ui->recieverSerialComPortComboBox->addItem(portList.at(i).portName(), portIndex);
    }

    // if com port was found, we can set this comport as the current choice
    if(portIndexFoundItemPosition) ui->recieverSerialComPortComboBox->setCurrentIndex(portIndexFoundItemPosition);

    // list possible baudrates (only baudrates common for windows and linux were selected)
    ui->recieverSerialBaudRateComboBox->addItem(tr("110"), 110);
    ui->recieverSerialBaudRateComboBox->addItem(tr("300"), 300);
    ui->recieverSerialBaudRateComboBox->addItem(tr("600"), 600);
    ui->recieverSerialBaudRateComboBox->addItem(tr("1200"), 1200);
    ui->recieverSerialBaudRateComboBox->addItem(tr("2400"), 2400);
    ui->recieverSerialBaudRateComboBox->addItem(tr("4800"), 4800);
    ui->recieverSerialBaudRateComboBox->addItem(tr("9600"), 9600);
    ui->recieverSerialBaudRateComboBox->addItem(tr("19200"), 19200);
    ui->recieverSerialBaudRateComboBox->addItem(tr("38400"), 38400);
    ui->recieverSerialBaudRateComboBox->addItem(tr("57600"), 57600);
    ui->recieverSerialBaudRateComboBox->addItem(tr("115200"), 115200);
    ui->recieverSerialBaudRateComboBox->addItem(tr("500000"), 500000);
    ui->recieverSerialBaudRateComboBox->addItem(tr("1000000"), 1000000);

    // show the same baudrate as is set in settings object
    int baudrateLoaded = settings->getComPortBaudRate();

    // find such baudrate
    int index = ui->recieverSerialBaudRateComboBox->findData(baudrateLoaded);
    // if found, highlight the item as current
    if(index>=0) ui->recieverSerialBaudRateComboBox->setCurrentIndex(index);
    else ui->recieverSerialBaudRateComboBox->setCurrentIndex(6); // 9600 baud by default (change this as well if adding new items into combobox)

    // load the reciever method and check correct radio button. If RS232 method is not set, disabe changing baudrates and COM ports
    reciever_method temp_method = settings->getRecieverMethod();
    ui->recieverSerialWidget->setDisabled(true); // initially disable com port settings

    if(temp_method==RS232)
    {
        ui->methodSerialRadioButton->setChecked(true);
        ui->recieverSerialWidget->setDisabled(false);
    }
    #if defined (__WIN32__)
    else if(temp_method==SYNTHETIC)
    {
        ui->methodSyntheticRadioButton->setChecked(true);
    }
    #endif
    else
    {
        ui->methodUndefinedRadioButton->setChecked(true);
    }

    ui->recieverIdleTimeSpinBox->setValue(settings->getRecieverIdleTime());
    ui->recieverMaxErrorCountSpinBox->setValue(settings->getMaximumRecieverErrorCount());

    settingsMutex->unlock();

    connect(this, SIGNAL(accepted()), this, SLOT(accepted()));
    connect(ui->methodSelection, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(methodSelectionChangedSlot(QAbstractButton*)));
}

dataInputDialog::~dataInputDialog()
{
    delete ui;
}

void dataInputDialog::accepted()
{
    settingsMutex->lock();

    if(ui->methodSerialRadioButton->isChecked()) settings->setRecieverMethod(RS232);
    #if defined (__WIN32__)
    else if(ui->methodSyntheticRadioButton->isChecked()) settings->setRecieverMethod(SYNTHETIC);
    #endif
    else settings->setRecieverMethod(UNDEFINED);

    settings->setRecieverIdleTime(ui->recieverIdleTimeSpinBox->value());
    settings->setMaximumRecieverErrorCount(ui->recieverMaxErrorCountSpinBox->value());
    settings->setComPortBaudRate(ui->recieverSerialBaudRateComboBox->currentData().toInt());
    settings->setComPortNumber(ui->recieverSerialComPortComboBox->currentData().toInt());

    settingsMutex->unlock();

    qDebug() << "Setting up new reciever method. Please restart the input thread to apply changes.";
}

void dataInputDialog::methodSelectionChangedSlot(QAbstractButton *button)
{
    if(button==ui->methodSerialRadioButton) ui->recieverSerialWidget->setDisabled(false);
    else  ui->recieverSerialWidget->setDisabled(true);
}
