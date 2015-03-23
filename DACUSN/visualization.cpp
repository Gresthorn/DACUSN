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
            // modify the last known schema status (sometimes when switching between schemas, some actions are needed)
            if(lastKnownSchema!=COMMON_FLOW)
            {
                lastKnownSchema = COMMON_FLOW;
            }

            settingsMutex->unlock();
            visualizationManager->launchCommonFlow();
        }
        else if(settings->getVisualizationSchema()==COMET_EFFECT)
        {
            // modify the last known schema status (sometimes when switching between schemas, some actions are needed)
            if(lastKnownSchema!=COMET_EFFECT)
            {
                if(lastKnownSchema==COMMON_FLOW)
                {
                    visualizationManager->hideAllCommonFlowSchemaObjects();
                }

                lastKnownSchema = COMET_EFFECT;
            }

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

    qDebug() << "Items in scene: " << visualizationScene->items().count();
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

    meter_to_pixel_ratio = 50;
    x_pixel = y_pixel = 0;
    x_width = y_width = 10;
}

/******************************************* CUSTOM OPENGL WIDGET ********************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/

openGLWidget::openGLWidget(QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags f)
    : QGLWidget(parent, shareWidget, f)
{
    // Ready to implement next stuff if needed
}


/******************************************* CUSTOM GRAPHICS VIEW ********************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/

radarView::radarView(radarScene *scene, uwbSettings * setts, QMutex * settings_mutex, QWidget *parent) : QGraphicsView(scene, parent)
{

    settings = setts;
    settingsMutex = settings_mutex;

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

    // setting scale factor to default value;
    scaleFactor = 1.01;
    scaleDepth = 10;
    scaleCounter = scaleCounterGlobal = 0;
    currentRotation = 0.0;

    moveToXYSteps = 20;
    angleStep = 5;

    // setting up scene
    setScene(scene);
    setSceneRect(-50000.0, -50000.0, 100000.0, 100000.0);
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

void radarView::moveToXYAnimation(const QPointF &target)
{
    // calculate the line between current center and target center
    QPointF currCenter = mapToScene(viewport()->rect().center());

    // calculate distances between x and y of points
    double x_steps_dx = target.x()-currCenter.x();
    double y_steps_dy = target.y()-currCenter.y();

    dx = x_steps_dx/((double)(moveToXYSteps));
    dy = y_steps_dy/((double)(moveToXYSteps));
    moveToXYCounter = 0;

    targetPoint = target;

    // animation is enabled, run animation timer
    QTimer * sceneMoveToXYAnimationTimer = new QTimer;
    connect(sceneMoveToXYAnimationTimer, SIGNAL(timeout()), this, SLOT(centerOnXYAnimationSlot()));
    sceneMoveToXYAnimationTimer->start(30);
}

void radarView::setRotationAngleAnimation(int target)
{
    targetAngle = target;

    // start animation
    QTimer * angleChangeAnimationTimer = new QTimer;
    connect(angleChangeAnimationTimer, SIGNAL(timeout()), this, SLOT(sceneRotationAnimationSlot()));
    angleChangeAnimationTimer->start(25);
}

void radarView::mouseReleaseEvent(QMouseEvent *event)
{
    // we have to update status before any update
    radarScene * scene = static_cast<radarScene * >(this->scene());
    scene->setTappingSequence(false);

    QGraphicsView::mouseReleaseEvent(event);

    this->viewport()->update();
}

void radarView::mousePressEvent(QMouseEvent *event)
{
    radarScene * scene = static_cast<radarScene * >(this->scene());
    scene->setTappingSequence(true);
    QGraphicsView::mousePressEvent(event);
}

void radarView::wheelEvent(QWheelEvent *event)
{
    bool smoothTransitionEnabled;
    settingsMutex->lock();
        smoothTransitionEnabled = settings->getSmoothTransitions();
    settingsMutex->unlock();

    if(event->delta()>0)
    {
        if(smoothTransitionEnabled)
        {
            // animation is enabled
            QTimer * scaleInTimer = new QTimer;
            connect(scaleInTimer, SIGNAL(timeout()), this, SLOT(scaleInSlot()));
            scaleInTimer->start(50);
        }
        else
        {
            // animation is disabled
            scale(pow(scaleFactor, (double)(scaleDepth)), pow(scaleFactor, (double)(scaleDepth)));
            scaleCounterGlobal++;
        }
    }
    else
    {
        if(smoothTransitionEnabled)
        {
            // animation is enabled
            QTimer * scaleOutTimer = new QTimer;
            connect(scaleOutTimer, SIGNAL(timeout()), this, SLOT(scaleOutSlot()));
            scaleOutTimer->start(50);
        }
        else
        {
            // animation is disabled
            scale((1.0)/pow(scaleFactor, (double)(scaleDepth)), (1.0)/pow(scaleFactor, (double)(scaleDepth)));
            scaleCounterGlobal--;
        }
    }

}

void radarView::scaleInSlot()
{
    if(scaleCounter<scaleDepth)
    {
        // continue in scale in animating
        scale(scaleFactor, scaleFactor);
        scaleCounter++;
    }
    else
    {
        // finish scaling animation
        QTimer * scaleInTimer = qobject_cast<QTimer * >(sender());
        scaleInTimer->stop();
        delete scaleInTimer;

        scaleCounter = 0;
        scaleCounterGlobal++;
    }
}

void radarView::scaleOutSlot()
{

    if(scaleCounter<scaleDepth)
    {
        // continue in scale out animating
        scale((1.0)/scaleFactor, (1.0)/scaleFactor);
        scaleCounter++;
    }
    else
    {
        // finish scaling animation
        QTimer * scaleOutTimer = qobject_cast<QTimer * >(sender());
        scaleOutTimer->stop();
        delete scaleOutTimer;

        scaleCounter = 0;
        scaleCounterGlobal--;
    }
}

void radarView::setRotationAngle(int angle)
{
    QTransform transform;

    transform.rotate(angle);
    currentRotation = (double)(angle);

    int scaleMultiplier = scaleCounterGlobal*scaleDepth;

    qreal scale;

    scale = pow(scaleFactor, (double)(scaleMultiplier));

    transform.scale(scale, -(1.0)*scale);

    setTransform(transform);
}

void radarView::centerOnXYAnimationSlot()
{
    if(moveToXYCounter<moveToXYSteps)
    {
        QPointF currCenter = mapToScene(viewport()->rect().center());
        centerOn(currCenter.x()+dx, currCenter.y()+dy);
        moveToXYCounter++; // incrementing counter to know how much steps needed for reaching destination
    }
    else
    {
        // animation finished
        QTimer * sceneMoveToXYAnimationTimer = qobject_cast<QTimer * >(sender());
        sceneMoveToXYAnimationTimer->stop();
        delete sceneMoveToXYAnimationTimer;

        centerOn(targetPoint);
    }
}

void radarView::sceneRotationAnimationSlot()
{
    if(currentRotation<targetAngle)
    {
        currentRotation += angleStep;
        if(currentRotation>targetAngle)
        {
            // we are done now
            QTimer * rotationAngleAnimationTimer = qobject_cast<QTimer * >(sender());
            rotationAngleAnimationTimer->stop();
            delete rotationAngleAnimationTimer;

            setRotationAngle(targetAngle);
        }
        else
        {
            // continue in decrementing by step size
            setRotationAngle(currentRotation);
        }
    }
    else if(currentRotation>targetAngle)
    {
        currentRotation -= angleStep;
        if(currentRotation<targetAngle)
        {
            // we are done now
            QTimer * rotationAngleAnimationTimer = qobject_cast<QTimer * >(sender());
            rotationAngleAnimationTimer->stop();
            delete rotationAngleAnimationTimer;

            setRotationAngle(targetAngle);
        }
        else
        {
            // continue in decrementing by step size
            setRotationAngle(currentRotation);
        }
    }
    else
    {
        // we are done as well
        QTimer * rotationAngleAnimationTimer = qobject_cast<QTimer * >(sender());
        rotationAngleAnimationTimer->stop();
        delete rotationAngleAnimationTimer;
    }
}

void radarView::mouseMoveEvent(QMouseEvent *event)
{
    settingsMutex->lock();
    // if updates are not allowed
    if(settings->getTappingRenderMethod()==NO_SCENE_UPDATE)
    {
        settingsMutex->unlock();
        return;
    }
    settingsMutex->unlock();

    this->viewport()->update();
    QGraphicsView::mouseMoveEvent(event);
}

/******************************************* CUSTOM GRAPHICS SCENE *******************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/
/*************************************************************************************************************/


radarScene::radarScene(uwbSettings * setts, QMutex * settings_mutex, QObject *parent) : QGraphicsScene(parent)
{
    settings = setts;
    settingsMutex = settings_mutex;

    tappingSequence = false;
}

bool radarScene::isOpenGL(QPainter *painter)
{
    if(painter->paintEngine()->type()==QPaintEngine::OpenGL2) return true;
    else return false;
}

void radarScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // draw background with needed colors
    bool drawBackgroundEnabled;
    settingsMutex->lock();
        drawBackgroundEnabled = settings->backgroundIsEnabled();
    settingsMutex->unlock();

    if(drawBackgroundEnabled)
    {
        QColor * backgroundColor;
        settingsMutex->lock();
            backgroundColor = settings->getBackgroundColor();
        settingsMutex->unlock();

        QBrush background(*backgroundColor, Qt::SolidPattern);
        painter->fillRect(rect, background);
    }


    visualization_tapping_options tapping_opt;

    settingsMutex->lock();

            tapping_opt = settings->getTappingRenderMethod();

    settingsMutex->unlock();

    // no background rendering alloved
    if(tappingSequence && tapping_opt==NO_BACKGROUND) return;

    bool g_one, g_two, g_three;

    settingsMutex->lock();

        g_one = settings->gridOneIsEnabled();
        g_two = settings->gridTwoIsEnabled();
        g_three = settings->gridThreeIsEnabled();

    settingsMutex->unlock();

    qreal left, top;

    // the most smooth/detailed grid
    if(g_three)
    {
        // check if we are tapping scene and if so, we check for conditions
        bool render_decision;
        if(tappingSequence) if(tapping_opt!=NO_3 && tapping_opt!=NO_3_2) render_decision = true; else render_decision = false;
        else render_decision = true;

        if(render_decision)
        {
            int detailedGridSize = 20;

            left = int(rect.left()) - (int(rect.left()) % detailedGridSize);
            top = int(rect.top()) - (int(rect.top()) % detailedGridSize);
            //const int count = (int(rect.left())/detailedGridSize) + (int(rect.top())/detailedGridSize) + 2;

            QVarLengthArray<QLineF, 500> detailedLines;

            for (qreal x = left; x < rect.right(); x += detailedGridSize)
                detailedLines.append(QLineF(x, rect.top(), x, rect.bottom()));

            for (qreal y = top; y < rect.bottom(); y += detailedGridSize)
                detailedLines.append(QLineF(rect.left(), y, rect.right(), y));

            settingsMutex->lock();
            painter->setPen(QPen(QBrush(*settings->getGridThreeColor()), 1.0, Qt::SolidLine));
            settingsMutex->unlock();

            painter->drawLines(detailedLines.data(), detailedLines.size());
        }
    }

    // middle smooth/detailed grid
    if(g_two)
    {
        // check if we are tapping scene and if so, we check for conditions
        bool render_decision;
        if(tappingSequence) if(tapping_opt!=NO_2 && tapping_opt!=NO_3_2) render_decision = true; else render_decision = false;
        else render_decision = true;

        if(render_decision)
        {
            int middleDetailedGridSize = 100;

            left = int(rect.left()) - (int(rect.left()) % middleDetailedGridSize);
            top = int(rect.top()) - (int(rect.top()) % middleDetailedGridSize);

            QVarLengthArray<QLineF, 250> middleDetailedLines;

            for (qreal x = left; x < rect.right(); x += middleDetailedGridSize)
                middleDetailedLines.append(QLineF(x, rect.top(), x, rect.bottom()));

            for (qreal y = top; y < rect.bottom(); y += middleDetailedGridSize)
                middleDetailedLines.append(QLineF(rect.left(), y, rect.right(), y));

            settingsMutex->lock();
            painter->setPen(QPen(QBrush(*settings->getGridTwoColor()), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            settingsMutex->unlock();

            painter->drawLines(middleDetailedLines.data(), middleDetailedLines.size());
        }
    }

    // basic/main lines must go last, so is seen clearly - rendered on top
    if(g_one)
    {
        QVarLengthArray<QLineF, 2> lines;

        lines.append(QLineF(0.0, rect.top(), 0.0, rect.bottom()));
        lines.append(QLineF(rect.left(), 0.0, rect.right(), 0.0));

        settingsMutex->lock();
        painter->setPen(QPen(QBrush(*settings->getGridOneColor()), 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        settingsMutex->unlock();

        painter->drawLines(lines.data(), lines.size());
    }
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


        QSequentialAnimationGroup * animationGroup = new QSequentialAnimationGroup(item);

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

        connect(animationGroup, SIGNAL(finished()), this, SLOT(deleteAnimationGroup()));

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

void animationManager::hideAllCommonFlowSchemaObjects()
{
    int i;
    for(i=0; i<ellipseList->count(); i++) if(ellipseList->at(i)->isVisible()) ellipseList->at(i)->hide();
}

void animationManager::deleteAnimationGroup()
{
    QSequentialAnimationGroup * group = qobject_cast<QSequentialAnimationGroup * >(sender());
    cometItem * comItem = qobject_cast<cometItem * >(group->parent());

    group->clear();

    delete group;
    delete comItem;
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
