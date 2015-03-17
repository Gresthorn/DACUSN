#ifndef VISUALIZATION
#define VISUALIZATION

#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QMutex>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsObject>
#include <QList>
#include <QGridLayout>

#include "uwbsettings.h"

/**
 * @file visualization.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class which is responsible for all visual effects in the graphics scene.
 *
 * @section DESCRIPTION
 *
 * The 'animationManager' is class which provides possibility of universal management
 * of different animation schemes selected by user end ensures deletion and creation
 * of new graphics objects safely. 'VisualizationSchema' variable tells what visualization
 * is selected and appropriate algorithms are running. All schemes are availible
 * through enumeration types and user may modify this settings via GUI.
 *
 */

class animationManager : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief Constructs the animationManager object.
     * @param[in] visualizationData The final positions of targets
     * @param[in] visualizationColor The colors assigned to all targets
     * @param[in] visualizationDataMutex Mutex protecting visualization data from being accessed by multiple threads at the same time
     * @param[in] setts Pointer to the basic application settings object.
     * @param[in] settings_mutex Pointer to the mutex locking the settings object.
     */
    animationManager(QGraphicsScene * visualization_Scene, QList<QPointF * > * visualization_Data, QList<QColor * > * visualization_Color, QMutex * visualization_Data_Mutex, uwbSettings * setts, QMutex * settings_mutex);

    /**
     * @brief Creates and sets the animation controls to make the comet effect.
     */
    void launchCometItems(void);

    /**
     * @brief Starts the most common visualization sequence where only one circle represents the targets in front of operator.
     */
    void launchCommonFlow(void);

    /**
     * @brief This function will set the new meter_to_pixel_ratio for child methods when displaying targets.
     * @param[in] ratio Is the new ratio between one meter and pixels on the screen.
     */
    void setMeterToPixelRatio(int ratio) { meter_to_pixel_ratio = ratio; }

    /**
     * @brief Returns the currently set ratio between one meter and pixels on screen.
     * @return The return value is the ratio currently used.
     */
    int getMeterToPixelRatio(void) { return meter_to_pixel_ratio; }

    /**
     * @brief Function will hide all items in ellipse list.
     */
    void hideAllCommonFlowSchemaObjects(void);


private:
    int meter_to_pixel_ratio;
    int x_pixel, y_pixel, x_width, y_width;

    uwbSettings * settings; ///< Pointer to the basic application settings object
    QMutex * settingsMutex; ///< Pointer to the mutex locking the settings object

    QList<QPointF * > * visualizationData; ///< The final positions of targets
    QList<QColor * > * visualizationColor; ///< The colors assigned to all targets
    QMutex * visualizationDataMutex; ///< Mutex protecting visualization data from being accessed by multiple threads at the same time

    QGraphicsScene * visualizationScene; ///< Is the scene where all items/objects are rendered on
    QList<QGraphicsEllipseItem * > * ellipseList; ///< Is the vector of ellipses positioned in current target's [x,y]

public slots:
};

/******************************************* CUSTOM GRAPHICS VIEW ********************************************/

class radarView : public QGraphicsView
{
public:
    /**
     * @brief The constructor sets basic parameters of the view and creates grid layout to allow rulers to be displayed in widget.
     * @param[in] scene Is the scene where all visualizations will be drawn.
     * @param[in] parent Is the parent widget (usually mainwindow).
     */
    radarView(class radarScene * scene, uwbSettings * setts, QMutex * settings_mutex, QWidget * parent = 0);

private:
    QGridLayout * radarViewLayout; ///< Grid layout for placing rulers or maybe in future another additional widgets.
    QWidget * emptyCorner; ///< Used to fill empty corner in grid layout.

    uwbSettings * settings;
    QMutex * settingsMutex;

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
};

/******************************************* CUSTOM GRAPHICS SCENE *******************************************/

class radarScene : public QGraphicsScene
{

public:
    /**
     * @brief This simple constructor just calls the QGraphicsScene to do all needed stuff like loading basic colors grid, background etc.
     * @param[in] parent Is parent widget of scene.
     */
    radarScene(uwbSettings * setts, QMutex * settings_mutex, QObject *parent = 0);

    /**
     * @brief This function is used primarily by view widget. If user is tapping, the status is on, else is off to notify scene about wether to check rendering conditions.
     * @param[in] status Is true if user is tapping the view.
     */
    void setTappingSequence(bool status) { tappingSequence = status; }

private:
    uwbSettings * settings;
    QMutex * settingsMutex;

    bool tappingSequence; ///< If user is tapping the scene, this value is set to true so rendering may check tapping options

protected:
    /**
     * @brief This reimplemented function ensures that grid is painted as a background. How grid is being drawn can be set by user (colors, smoothness, etc)
     * @param[in] painter Painter object used for drawing/rendering on screen.
     * @param[in] rect Is the rendering area, usually all scene.
     */
    virtual void drawBackground(QPainter * painter, const QRectF & rect);
};

/******************************************* CUSTOM GRAPHICS ITEMS *******************************************/

class cometItem :  public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT

    Q_PROPERTY(qreal size READ getRectSize WRITE setRectSize)


public:
    /**
     * @brief This constructor simplify the parameters passed into object to basic parameters, easier to understand from another threads/functions.
     * @param[in] position Is the position of target that we wish to display.
     * @param[in] size Is the size of the circle representing the target in scene.
     * @param[in] color The color assigned to the target.
     */
    cometItem (const QPointF & position, qreal size, QColor color);

    /**
     * @brief Function sets new pixel size of items rect. New rect is recalculated to fit into targets position [x, y].
     * @param[in] rectSize Is the new rect size of item.
     */
    void setRectSize(qreal rectSize);

    /**
     * @brief Returns the current rect size. This function is primarily used by QAnimationProperty objects.
     * @return The return value is currently used rect size.
     */
    qreal getRectSize(void) { return size; }

private:
    qreal size; ///< Is the current size of item representing object. This value is animated and changed by animation property. Note that this class is drawing circles, so bounding is in fact square represented by this parameter.
    QPointF targetPosition; ///< Is the target position from where bounding rect corners are calculated.
};

#endif // VISUALIZATION

