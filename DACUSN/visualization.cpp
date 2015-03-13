#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QElapsedTimer>


void MainWindow::visualizationSlot()
{
    qDebug() << "Visualization running...";

    QElapsedTimer time;
    time.start();

    int meter_to_pixel_ratio = 20;
    int x_pixel, y_pixel;
    int i;


    visualizationDataMutex->lock();

    // if there are more targets to display then we have item in the scene
    while(ellipseList->count()<visualizationData->count())
    {
        ellipseList->append(new QGraphicsEllipseItem(0.0, 0.0, 10.0, 10.0));
        ellipseList->last()->setVisible(false);
        visualizationScene->addItem(ellipseList->last());
    }

    for(i=0; i<visualizationData->count(); i++)
    {
        x_pixel = meter_to_pixel_ratio*visualizationData->at(i)->x();
        y_pixel = meter_to_pixel_ratio*visualizationData->at(i)->y();

        ellipseList->at(i)->setPos(x_pixel, y_pixel);
        ellipseList->at(i)->setBrush(QBrush(*visualizationColor->at(i)));

        if(!ellipseList->at(i)->isVisible()) ellipseList->at(i)->setVisible(true);
    }

    // if unused items, hide them

    for(i=visualizationData->count(); i<ellipseList->count(); i++)
    {
        if(ellipseList->at(i)->isVisible()) ellipseList->at(i)->setVisible(false);
    }

    visualizationDataMutex->unlock();

    // repaint scene
    visualizationScene->update();

    qint64 elapsed = time.nsecsElapsed();
    qDebug() << "Elapsed : " << elapsed;

}
