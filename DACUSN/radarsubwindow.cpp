#include "radarsubwindow.h"
#include "ui_radarsubwindow.h"

radarSubWindow::radarSubWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::radarSubWindow)
{
    ui->setupUi(this);
}

radarSubWindow::~radarSubWindow()
{
    delete ui;
}
