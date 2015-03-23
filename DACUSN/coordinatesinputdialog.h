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
