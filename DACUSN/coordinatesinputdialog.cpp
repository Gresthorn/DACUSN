#include "coordinatesinputdialog.h"
#include "ui_coordinatesinputdialog.h"

coordinatesInputDialog::coordinatesInputDialog(const QPointF & initial, QPointF * target, bool * result, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::coordinatesInputDialog)
{
    ui->setupUi(this);

    targetCoordinates = target;
    userDecision = result;

    ui->xSpinBox->setValue(initial.x());
    ui->ySpinBox->setValue(initial.y());

    connect(this, SIGNAL(accepted()), this, SLOT(accepted()));
    connect(this, SIGNAL(rejected()), this, SLOT(rejected()));
}

coordinatesInputDialog::~coordinatesInputDialog()
{
    delete ui;
}

void coordinatesInputDialog::accepted()
{
    targetCoordinates->setX(ui->xSpinBox->value());
    targetCoordinates->setY(ui->ySpinBox->value());

    *userDecision = true;
}

void coordinatesInputDialog::rejected()
{
    *userDecision = false;
}
