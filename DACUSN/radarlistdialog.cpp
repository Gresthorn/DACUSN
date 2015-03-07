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
    // FOR COMFORT USE IN POPULATING FUNCTION AND ALSO ADD RADAR UNIT FUNCTION ITSELF !!!

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
        int offset = 100 + ui->radarListGroupBox->width(); // add space for slider if there is too many rows (the offset is set for fine display of basic set of values - first 5)
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

    connect(ui->radarListTable, SIGNAL(cellChanged(int,int)), this, SLOT(cellChangedControlSlot(int,int)));
    connect(ui->radarListTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(cellClickedRememberSlot(int,int)));
    connect(ui->addRadarUnitButton, SIGNAL(clicked()), this, SLOT(addRadarUnitSlot()));
    connect(ui->radarListTable, SIGNAL(cellClicked(int,int)), SLOT(rowSelectionSlot()));
    connect(ui->deleteRadarUnitButton, SIGNAL(clicked()), this, SLOT(deleteRadarUnitSlot()));
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
        QCheckBox * enableCheckBox = new QCheckBox;
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
