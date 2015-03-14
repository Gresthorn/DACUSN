#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "visualization.h"

#include <QElapsedTimer>

void MainWindow::visualizationSlot()
{
    // PROTOTYPE OF THIS METHOD IS IN MAINWINDOW

    QElapsedTimer timer;
    timer.start();

        settingsMutex->lock(); // UNLOCK MUTEX IMMEDIATELY AFTER IF/CASE BLOCK STARTS SO OTHER THREADS CAN ACCESS SETTINGS IF NEEDED!!!!!!!

        if(settings->getVisualizationSchema()==COMMON_FLOW)
        {
            settingsMutex->unlock();
            visualizationManager->launchCommonFlow();
        }
        else if(settings->getVisualizationSchema()==COMET_EFFECT)
        {
            settingsMutex->unlock();
            visualizationManager->launchCometItems();

        }
        else
        {
            settingsMutex->unlock();
            qDebug() << "Unknown visualization method selected.";
        }

    // repaint scene
    visualizationScene->update();

    qDebug() << "ELAPSED " << timer.nsecsElapsed();

}


animationManager::animationManager(QGraphicsScene * visualization_Scene, QList<QPointF * > * visualization_Data, QList<QColor * > * visualization_Color, QMutex * visualization_Data_Mutex, uwbSettings * setts, QMutex * settings_mutex)
{
    visualizationScene = visualization_Scene;

    ellipseList = new QList<QGraphicsEllipseItem * >;

    settings = setts;
    settingsMutex = settings_mutex;

    visualizationData = visualization_Data;
    visualizationColor = visualization_Color;
    visualizationDataMutex = visualization_Data_Mutex;

    meter_to_pixel_ratio = 20;
    x_pixel = y_pixel = 0;
    x_width = y_width = 10;
}

/******************************************* CUSTOM GRAPHICS VIEW ********************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/

radarView::radarView(radarScene *scene, QWidget *parent) : QGraphicsView(scene, parent)
{
    // prepare new layout for ruler and view widget placing
    radarViewLayout = new QGridLayout;

    // remove borders between cells of grid layout
    radarViewLayout->setSpacing(0);
    radarViewLayout->setMargin(0);

    emptyCorner = new QWidget(this);
    emptyCorner->setFixedSize(30, 30);

    // placing widgets
    radarViewLayout->addWidget(emptyCorner, 0, 0);
    radarViewLayout->addWidget(this->viewport(), 1, 1);

    // setting up scene
    setScene(scene);
    setSceneRect(-50000.0, -50000.0, 100000, 100000);
    scale(1.0, -1.0);
    centerOn(0.0, 0.0);

    // scene is big so we need to hide the scroll bars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // setting rendering
    setRenderHint(QPainter::Antialiasing);
    setInteractive(true);
    setDragMode(QGraphicsView::ScrollHandDrag);

    this->setLayout(radarViewLayout);
}

void radarView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    this->viewport()->update();
}

void radarView::mouseMoveEvent(QMouseEvent *event)
{
    this->viewport()->update();
    QGraphicsView::mouseMoveEvent(event);
}

/******************************************* CUSTOM GRAPHICS SCENE *******************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/


void radarScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    qreal left, top;

    // basic/main lines
    QVarLengthArray<QLineF, 2> lines;

    lines.append(QLineF(0.0, rect.top(), 0.0, rect.bottom()));
    lines.append(QLineF(rect.left(), 0.0, rect.right(), 0.0));

    painter->setPen(QPen(QBrush(QColor(111, 111, 111, 255)), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    painter->drawLines(lines.data(), lines.size());

    // middle smooth/detailed grid
    int middleDetailedGridSize = 100;

    left = int(rect.left()) - (int(rect.left()) % middleDetailedGridSize);
    top = int(rect.top()) - (int(rect.top()) % middleDetailedGridSize);

    QVarLengthArray<QLineF, 250> middleDetailedLines;

    for (qreal x = left; x < rect.right(); x += middleDetailedGridSize)
        middleDetailedLines.append(QLineF(x, rect.top(), x, rect.bottom()));

    for (qreal y = top; y < rect.bottom(); y += middleDetailedGridSize)
        middleDetailedLines.append(QLineF(rect.left(), y, rect.right(), y));

    painter->setPen(QPen(QBrush(QColor(111, 111, 111, 100)), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    painter->drawLines(middleDetailedLines.data(), middleDetailedLines.size());

    // the most smooth/detailed grid
    int detailedGridSize = 20;

    left = int(rect.left()) - (int(rect.left()) % detailedGridSize);
    top = int(rect.top()) - (int(rect.top()) % detailedGridSize);
    //const int count = (int(rect.left())/detailedGridSize) + (int(rect.top())/detailedGridSize) + 2;

    QVarLengthArray<QLineF, 500> detailedLines;

    for (qreal x = left; x < rect.right(); x += detailedGridSize)
        detailedLines.append(QLineF(x, rect.top(), x, rect.bottom()));

    for (qreal y = top; y < rect.bottom(); y += detailedGridSize)
        detailedLines.append(QLineF(rect.left(), y, rect.right(), y));

    painter->setPen(QPen(QBrush(QColor(0,0,0, 150)), 0.2, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));

    painter->drawLines(detailedLines.data(), detailedLines.size());
}

/******************************************* CUSTOM GRAPHICS ITEMS *******************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/


void animationManager::launchCometItems()
{
    cometItem * item;
    int i;

    QList<QSequentialAnimationGroup * > animationGroupList;

    visualizationDataMutex->lock();

    // prepare animations
    for(i=0; i<visualizationData->count(); i++)
    {
        x_pixel = meter_to_pixel_ratio*visualizationData->at(i)->x();
        y_pixel = meter_to_pixel_ratio*visualizationData->at(i)->y();

        item = new cometItem(QPointF(x_pixel, y_pixel), 0.0, *visualizationColor->at(i));


        QSequentialAnimationGroup * animationGroup = new QSequentialAnimationGroup;

        // rising animation
        QPropertyAnimation * resizeUp = new QPropertyAnimation(item, "size");
        resizeUp->setDuration(50);
        resizeUp->setStartValue(15.0);
        resizeUp->setEndValue(15.0);
        animationGroup->addAnimation(resizeUp);

        // falling animation
        QPropertyAnimation * resizeDown = new QPropertyAnimation(item, "size");
        resizeDown->setDuration(2000);
        resizeDown->setStartValue(15.0);
        resizeDown->setEndValue(0.0);
        animationGroup->addAnimation(resizeDown);

        visualizationScene->addItem(item);

        animationGroupList.append(animationGroup);
    }


    visualizationDataMutex->unlock();

    // now start all animations

    for(i=0; i<animationGroupList.count(); i++) animationGroupList.at(i)->start(QAbstractAnimation::DeleteWhenStopped);

    //update scene
    visualizationScene->update();
}

void animationManager::launchCommonFlow()
{
    int i;

    visualizationDataMutex->lock();

    // if there are more targets to display then we have item in the scene
    while(ellipseList->count()<visualizationData->count())
    {
        ellipseList->append(new QGraphicsEllipseItem(0.0, 0.0, x_width, y_width));
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
}



cometItem::cometItem(const QPointF &position, qreal size, QColor color) : QGraphicsEllipseItem()
{
    // we need to set new coordinates of bounding rect so the circle is centered exactly at the 'position'
    setRect(position.x()-size/2.0, position.y()-size/2.0, size, size);
    //setBrush(QBrush(color, Qt::SolidPattern));
    setPen(QPen(QBrush(color), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    size = size;
    targetPosition = position;
}

void cometItem::setRectSize(qreal rectSize)
{
    size = rectSize;
    // we need to set new coordinates of bounding rect so the circle is centered exactly at the 'position'
    setRect(targetPosition.x()-rectSize/2.0, targetPosition.y()-rectSize/2.0, rectSize, rectSize);
}

