/**
 * @file coordinatesinputdialog.cpp
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Definition of coordinatesInputDialog class methods.
 *
 * @section DESCRIPTION
 *
 * The coordinatesInputDialog class inherits a QDialog class so it is able to easily build
 * dialog windows by calling show() or exec() methods. This is simple dialog for changing coordinates
 * where the view will be centered on.
 *
 * Dialog will inform upper functions about confirmation via the 'result' and 'target' pointers.
 * Higher function will check for boolean in 'result' and if it is true, they will call needed
 * methods to center the view at 'target'.
 *
 */

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
