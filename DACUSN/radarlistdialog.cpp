#include "radarlistdialog.h"
#include "ui_radarlistdialog.h"

radarListDialog::radarListDialog(QVector<radar_handler * > * radar_List, QMutex * radar_List_Mutex, uwbSettings * setts, QMutex * settings_Mutex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::radarListDialog)
{
    ui->setupUi(this);

    settings = setts;
    settingsMutex = settings_Mutex;

    radarList = radar_List;
    radarListMutex = radar_List_Mutex;

    ui->deleteRadarUnitButton->setDisabled(true);

    // !!! IF ADDING NEW ROW DO NOT FORGET TO UPDATE LABELS LIST, COLUMNS WIDTH LIST AND ENUM TYPE (TABLE COLUMNS)
    // FOR COMFORT USE IN POPULATING FUNCTION AND ALSO ADD RADARUNITFUNCTION/ACCEPTED FUNCTIONS ITSELF !!!

    QStringList horizontalLabels;

    horizontalLabels << "Enable/Disable" << "Radar ID" << "X position" << "Y position" << "Rotation";

    QList<int> columnWidths;

    columnWidths << 100 << 100 << 100 << 100 << 100;

    ui->radarListTable->setColumnCount(horizontalLabels.count());
    ui->radarListTable->setHorizontalHeaderLabels(horizontalLabels);

    // setting widths of columns if there is a column width for every unit
    if(horizontalLabels.count()==columnWidths.count())
    {
        int i;
        int widthSum = 0;
        int offset = 110 + ui->radarListGroupBox->width(); // add space for slider if there is too many rows (the offset is set for fine display of basic set of values - first 5)
        int maximumWindowWidth = 760;
        int windowHeight = 300;

        for(i=0; i<horizontalLabels.count(); i++)
        {
            ui->radarListTable->setColumnWidth(i, columnWidths.at(i));
            widthSum += columnWidths.at(i);
        }

        widthSum += offset;

        if(widthSum<=maximumWindowWidth) this->resize(widthSum, windowHeight);
        else this->resize(maximumWindowWidth, windowHeight);
    }

    // fill table with current radarList items
    radarListMutex->lock();

    if(!radarList->isEmpty())
    {
        int i;
        for(i=0; i<radarList->count(); i++)
        {
            addRadarUnit(radarList->at(i)->radar->isEnabled(),
                         radarList->at(i)->id,
                         radarList->at(i)->radar->getXpos(),
                         radarList->at(i)->radar->getYpos(),
                         convertToDegrees(radarList->at(i)->radar->getRotAngle()));
        }
    }

    radarListMutex->unlock();

    connect(ui->radarListTable, SIGNAL(cellChanged(int,int)), this, SLOT(cellChangedControlSlot(int,int)));
    connect(ui->radarListTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(cellClickedRememberSlot(int,int)));
    connect(ui->addRadarUnitButton, SIGNAL(clicked()), this, SLOT(addRadarUnitSlot()));
    connect(ui->radarListTable, SIGNAL(cellClicked(int,int)), SLOT(rowSelectionSlot()));
    connect(ui->deleteRadarUnitButton, SIGNAL(clicked()), this, SLOT(deleteRadarUnitSlot()));
    connect(ui->importFromFileButton, SIGNAL(clicked()), this, SLOT(importFromFileSlot()));
    connect(ui->clearListButton, SIGNAL(clicked()), this, SLOT(clearListSlot()));
    connect(ui->selectAllButton, SIGNAL(clicked()), this, SLOT(selectAllSlot()));
    connect(ui->deselectAllButton, SIGNAL(clicked()), this, SLOT(deselectAllSlot()));

    connect(this, SIGNAL(accepted()), this, SLOT(accepted()));
}

radarListDialog::~radarListDialog()
{
    delete ui;
}

void radarListDialog::addRadarUnit(bool enabled, int id, double x_pos, double y_pos, double angle)
{
    int rowID = ui->radarListTable->rowCount();
    ui->radarListTable->insertRow(rowID);
    ui->radarListTable->setRowHeight(rowID, 40);

    ui->radarListTable->setCellWidget(rowID, ENABLE,  new QWidget(ui->radarListTable));
        QWidget * lb = ui->radarListTable->cellWidget(rowID, ENABLE);
        QHBoxLayout * enableWrapper = new QHBoxLayout(lb);
        QCheckBox * enableCheckBox = new QCheckBox(lb);
        enableCheckBox->setObjectName("enable_status");
        enableCheckBox->setChecked(enabled);
        enableWrapper->addWidget(enableCheckBox);
        enableWrapper->setAlignment(Qt::AlignCenter);
        lb->setLayout(enableWrapper);
    ui->radarListTable->setItem(rowID, ID, new QTableWidgetItem(QString("%1").arg(id)));
        ui->radarListTable->item(rowID, ID)->setTextAlignment(Qt::AlignCenter);
        ui->radarListTable->item(rowID, ID)->setTextColor(Qt::red);
    ui->radarListTable->setItem(rowID, X, new QTableWidgetItem(QString("%1").arg(x_pos)));
        ui->radarListTable->item(rowID, X)->setTextAlignment(Qt::AlignCenter);
    ui->radarListTable->setItem(rowID, Y, new QTableWidgetItem(QString("%1").arg(y_pos)));
        ui->radarListTable->item(rowID, Y)->setTextAlignment(Qt::AlignCenter);
    ui->radarListTable->setItem(rowID, ANGLE, new QTableWidgetItem(QString("%1").arg(angle)));
    ui->radarListTable->item(rowID, ANGLE)->setTextAlignment(Qt::AlignCenter);
}

int radarListDialog::findFreeId()
{
    int i;
    int freeId = 0;
    int temp;
    bool success;

    // go through all ID's and seek for free one
    for(i=0; i<ui->radarListTable->rowCount(); i++)
    {
        temp = QString(ui->radarListTable->item(i, ID)->text()).toInt(&success);
        if(success)
        {
            // if freeId already exists need to increment freeId and try the new one
            if(temp==freeId) ++freeId;
        }
    }

    return freeId;
}

double radarListDialog::convertToRadians(double angle)
{
    return (angle/180.0)*PI;
}

double radarListDialog::convertToDegrees(double angle)
{
    return (angle/PI)*180.0;
}

void radarListDialog::importFromFileSlot()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select the file with radar list"), QDir::currentPath(), QString("Text files (*.txt)"));

    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("The file could not be opened. Check the path to the file or user privileges to file."), QMessageBox::Ok);
        return;
    }

    QTextStream input(&file);
    bool exclamationFound = false;
    int itemsInRow = 8;

    QVector<int> idVector;
    QVector<double> xVector;
    QVector<double> yVector;
    QVector<double> angleVector;

    while(!input.atEnd())
    {
        QString temp = input.readLine();
        // we need to iterate over the file until the exlamation is found (such format is used in Synthetic coordinate generator)
        // after the exlamation the radars information is placed in format id#centralX#centralY#leftAntennaX#leftAntennaY#rightAntennaX#rightAntennaY#rotationAngle
        if(!exclamationFound)
        {
            // check for exclamation
            if(temp=="!") exclamationFound = true;
            else continue;

            // for skipping the line with exclamation
            continue;
        }

        // if exclamation found, each line must correspond with needed format
        QStringList tempSplitted = temp.split("#");
        if(tempSplitted.count()!=itemsInRow)
        {
            QMessageBox::warning(this, tr("Incorrect file structure"), tr("The file structure after starting character has too few items in line."), QMessageBox::Ok);
            return;
        }

        bool success;
        int i;
        for(i=0; i<itemsInRow; i++)
        {
            if(i==0)
            {
                // if radar ID
                idVector.append(tempSplitted.at(i).toInt(&success));
                if(!success)
                {
                    QMessageBox::warning(this, tr("Conversion error"), tr("Could not convert some of input values from text into numbers. Check if your file has all numbers correctly expressed."), QMessageBox::Ok);
                    return;
                }
            }
            else if(i==1)
            {
                // if radar x_pos
                xVector.append(tempSplitted.at(i).toDouble(&success));
                if(!success)
                {
                    QMessageBox::warning(this, tr("Conversion error"), tr("Could not convert some of input values from text into numbers. Check if your file has all numbers correctly expressed."), QMessageBox::Ok);
                    return;
                }
            }
            else if(i==2)
            {
                // if radar y_pos
                yVector.append(tempSplitted.at(i).toDouble(&success));
                if(!success)
                {
                    QMessageBox::warning(this, tr("Conversion error"), tr("Could not convert some of input values from text into numbers. Check if your file has all numbers correctly expressed."), QMessageBox::Ok);
                    return;
                }
            }
            else if(i==7)
            {
                // if radar x_pos
                angleVector.append(tempSplitted.at(i).toDouble(&success));
                if(!success)
                {
                    QMessageBox::warning(this, tr("Conversion error"), tr("Could not convert some of input values from text into numbers. Check if your file has all numbers correctly expressed."), QMessageBox::Ok);
                    return;
                }
            }
            else continue; // we are not interested in other values (for now)
        }
    }

    // if the cycle is over and excalamation was not found, not correct file structure
    if(!exclamationFound)
    {
        QMessageBox::warning(this, tr("Incorrect file structure"), tr("The file structure of file is incorrect. Starting character not found."), QMessageBox::Ok);
        return;
    }

    // if we can reach this point, everything is correct
    int i;
    for(i=0; i<idVector.count(); i++)
    {
        // if id is zero, the radar is OPERATOR radar in synthetic coordinates generator file, so we skip it
        if(idVector.at(i)==0) continue;
        addRadarUnit(true, idVector.at(i), xVector.at(i), yVector.at(i), angleVector.at(i));
    }
}

void radarListDialog::deselectAllSlot()
{
    int i;
    for(i=0; i<ui->radarListTable->rowCount(); i++)
    {
        if(ui->radarListTable->cellWidget(i, ENABLE)->findChild<QCheckBox * >("enable_status")->isChecked())
            ui->radarListTable->cellWidget(i, ENABLE)->findChild<QCheckBox * >("enable_status")->setChecked(false);
    }
}

void radarListDialog::selectAllSlot()
{
    int i;
    for(i=0; i<ui->radarListTable->rowCount(); i++)
    {
        if(!ui->radarListTable->cellWidget(i, ENABLE)->findChild<QCheckBox * >("enable_status")->isChecked())
            ui->radarListTable->cellWidget(i, ENABLE)->findChild<QCheckBox * >("enable_status")->setChecked(true);
    }
}

void radarListDialog::clearListSlot()
{
    while(ui->radarListTable->rowCount()>0)
        ui->radarListTable->removeRow(0);
}

void radarListDialog::cellChangedControlSlot(int row, int column)
{
    bool success;
    switch(column)
    {
        case ENABLE: // do nothing
            break;
        case ID:
            QString(ui->radarListTable->item(row, column)->text()).toInt(&success);
            if(!success)
                ui->radarListTable->item(row, column)->setText(rememberedValue);
            break;
        default:
            QString(ui->radarListTable->item(row, column)->text()).toDouble(&success);
            if(!success)
                ui->radarListTable->item(row, column)->setText(rememberedValue);
            break;
    }
}

void radarListDialog::cellClickedRememberSlot(int row, int column)
{
    if(column==ENABLE) return; // if not widget but editable content

    rememberedValue = ui->radarListTable->item(row, column)->text();
}

void radarListDialog::addRadarUnitSlot()
{
    // defaults
    bool checked = true;
    int id = findFreeId();
    double x_pos = 0.0;
    double y_pos = 0.0;
    double angle = 0.0;

    addRadarUnit(checked, id, x_pos, y_pos, angle);
}

void radarListDialog::rowSelectionSlot()
{
    QModelIndexList selectedList = ui->radarListTable->selectionModel()->selection().indexes();

    // if some rows are selected, then enable deleting them
    if(!selectedList.isEmpty()) ui->deleteRadarUnitButton->setDisabled(false);
    else ui->deleteRadarUnitButton->setDisabled(true);
}

void radarListDialog::deleteRadarUnitSlot()
{
    QModelIndexList selectedList = ui->radarListTable->selectionModel()->selection().indexes();

    // delete all rows
    int i;
    for(i=0; i<selectedList.count(); i++)
        ui->radarListTable->removeRow(selectedList.at(i).row());
}

void radarListDialog::accepted()
{
    radarListMutex->lock();

    // first we clear the old list
    while(!radarList->isEmpty())
    {
        delete radarList->first()->radar;
        delete radarList->first();
        radarList->removeFirst();
    }

    // now we will set up new radarList
    int i;
    for(i=0; i<ui->radarListTable->rowCount(); i++)
    {
        unsigned int id = (unsigned int)(ui->radarListTable->item(i, ID)->text().toInt());
        double x_pos = ui->radarListTable->item(i, X)->text().toDouble();
        double y_pos = ui->radarListTable->item(i, Y)->text().toDouble();
        double angle = convertToRadians(ui->radarListTable->item(i, ANGLE)->text().toDouble());
        QCheckBox * enabled = ui->radarListTable->cellWidget(i, ENABLE)->findChild<QCheckBox * >("enable_status");

        radarList->append(new radar_handler);
        radarList->last()->id = id;
        radarList->last()->updated = false;
        radarList->last()->radar = new radarUnit(id, x_pos, y_pos, angle, enabled->isChecked());
    }

    radarListMutex->unlock();
}