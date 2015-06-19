/**
 * @file coordinatesInputDialog.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class for establishing object allowing user to set new "center on" coordinates
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
 */

#ifndef COORDINATESINPUTDIALOG_H
#define COORDINATESINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class coordinatesInputDialog;
}

class coordinatesInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit coordinatesInputDialog(const QPointF & initial, QPointF * target, bool * result, QWidget *parent = 0);
    ~coordinatesInputDialog();

private:
    Ui::coordinatesInputDialog *ui;

    QPointF * targetCoordinates;
    bool * userDecision;

public slots:
    void accepted(void);
    void rejected(void);
};

#endif // COORDINATESINPUTDIALOG_H
