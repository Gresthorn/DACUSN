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
#include <QElapsedTimer>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGLWidget>
#include <QRadialGradient>
#include <QGraphicsTextItem>
#include <QGraphicsSceneHoverEvent>
#include <QDateTime>
#include <QGraphicsTextItem>

#include "uwbsettings.h"
#include "radar_handler.h"

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

class crossItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:

    QRectF boundingRect() const;

    /**
     * @brief The constructor of single cross item.
     * @param[in] x The x coordinate of target.
     * @param[in] y The y coordinate of target.
     * @param[in] size Is the width and height of cross item.
     * @param[in] color Is the pointer color object for painting the cross item. For user this color indicates the target.
     * @param[in] scene Is the pointer to graphics scene used for drawing texts if additional info are required.
     * @param[in] parent Parent item.
     */
    crossItem(qreal x, qreal y, QColor * color, QGraphicsScene * scene, qreal size=5.0, QGraphicsItem * parent=0);

    /**
     * @brief Returns the time when the item was created in epochal form.
     * @return The return value is the time expressed in epochal form since 1970.
     */
    unsigned int getTime(void) { return timems; }

    /**
     * @brief Function returns the x-position of target. This position represents the real position without conversion to pixels.
     * @return The return value is the x-position in 'qreal' form.
     */
    qreal getX(void) { return xPos; }

    /**
     * @brief Function returns the y-position of target. This position represents the real position without conversion to pixels.
     * @return The return value is the y-position in 'qreal' form.
     */
    qreal getY(void) { return yPos; }

    ~crossItem();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    //void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    //void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    qreal xPos, yPos; ///< Represents the real position of target without any conversions to pixels.

    unsigned int timems; ///< The time when the object was generated in epochal form (may be considered as the time of targets position at 'xPos' and 'yPos')

    //QGraphicsTextItem * infoLabel; ///< Is the pointer to text displaying additional info after hovering.

    QVector<QLineF> * lines; ///< List for 'lineA' and 'lineB'

    QColor crossColor; ///< The color being used for item drawing.

    QGraphicsScene * graphicsScene; ///< The pointer to graphics scene where the item is placed.
};

class radarMarker : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:

    QRectF boundingRect() const;

    /**
     * @brief This object represents the radar in the scene. Marker is designed to indicate radars position and orientation.
     * @param x Is the x-coordinate of radar unit.
     * @param y Is the y-coordinate of radar unit.
     * @param name The string displayed together with icon.
     */
    radarMarker(qreal x, qreal y, const QString &name);

    /**
     * @brief Sets the new description or name displyed under marker
     * @param[in] desc New value for description or name.
     */
    void setMarkerDescription(const QString &desc);

    /**
     * @brief Gets the angle expressed in radians, converts it to degrees and sets the item angle. If angle is expressed in degrees, common 'setRotation()' function can be used.
     * @param[in] angle New angle to be set up.
     */
    void setRotationRadians(double angle);

    ~radarMarker();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    const double pi; ///< PI constant

    qreal xPos, yPos; ///< Represents the real position of target without any conversions to pixels.

    QString description; ///< Text displayed together with marker, usually name of radar or ID.

    QGraphicsScene * graphicsScene; ///< The pointer to graphics scene where the item is placed.
};

/******************************************* ANIMATION MANAGER CLASS *****************************************/

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
    animationManager(QGraphicsScene * visualization_Scene, QGraphicsView * visualization_View, QList<QPointF * > * visualization_Data, QList<QColor * > * visualization_Color, QMutex * visualization_Data_Mutex, uwbSettings * setts, QMutex * settings_mutex);

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

    /**
     * @brief Function will reveal all items in ellipse list.
     */
    void revealAllCommonFlowSchemaObjects(void);

    /**
     * @brief Is used by history path regime to discover if all ellipses are hidden and block the hiding algorithm to repeteadly hide ellipse items.
     * @return The return value is true if all items are hidden.
     */
    bool allCommonItemsHidden(void) { return ellipseListInvisible; }

    /**
     * @brief If user selects drawing history paths of movements, this function is called to update polygons and draws them.
     */
    void launchPathDrawing(void);

    /**
     * @brief This function will register new 'crossItem' in path handler.
     * @param[in] item Pointer to fully initialized and set up 'crossItem'.
     */
    void appendPathItem(crossItem * item) { pathHandler->append(item); }

    /**
     * @brief This function is called very rarely. Usually only in situation when radar list changes and radar markers need to be updated. Function will compare its local list and sets updated data.
     * @param[in] radarList Pointer to the vector where all radar info is stored.
     * @param[in] radarListMutex Mutex for radar vector protection.
     * @param[in] id Specifies which radar ID belongs to currently central radar (e.g in subwindows central radar can be radar N) so radar can read an transform other radar units position to its own.
     */
    void updateRadarMarkerList(QVector<radar_handler * > * radarList, QMutex * radarListMutex, int id = -1);

    /**
     * @brief Returns currently set radar id. When updating radar markers, this radar is used as reference/base for transformation.
     * @return Id of currently set radar id.
     */
    int getActiveRadarId(void) { return active_radar; }

    /**
     * @brief Sets new radar id. When updating radar markers, this radar is used as reference/base for transformation.
     * @param[in] id New radar id. Will be used primarily by 'updateRadarMarkerList()' function if no id is explicitly set.
     */
    void setActiveRadarId(int id) { active_radar = id; }

    /**
     * @brief Clears all pointers to all marker objects. May be usefull when refreshing scene and all objects are deleted by some global function. Then we need to call this to prevent animationManager from trying to access existing objects.
     * @param[in] deletion If set to true, function will also try to delete objects, not only clearing the list.
     */
    void clearRadarMarkerList(bool deletion);

    /**
     * @brief Clears all pointers to all ellipse objects. May be usefull when refreshing scene and all objects are deleted by some global function. Then we need to call this to prevent animationManager from trying to access existing objects.
     * @param[in] deletion If set to true, function will also try to delete objects, not only clearing the list.
     */
    void clearEllipseList(bool deletion);

private:
    int meter_to_pixel_ratio;
    int x_pixel, y_pixel, x_width, y_width;

    int active_radar; ///< Holds the ID of radar to which view other radar markers should be transformated. Default value is zero.

    uwbSettings * settings; ///< Pointer to the basic application settings object.
    QMutex * settingsMutex; ///< Pointer to the mutex locking the settings object.

    QList<radarMarker * > * radarMarkerList; ///< Holds all objects visualizating radar unit's positions and orientation
    QList<QPointF * > * visualizationData; ///< The final positions of targets
    QList<QColor * > * visualizationColor; ///< The colors assigned to all targets
    QMutex * visualizationDataMutex; ///< Mutex protecting visualization data from being accessed by multiple threads at the same time

    QGraphicsScene * visualizationScene; ///< Is the scene where all items/objects are rendered on.
    QGraphicsView * visualizationView; ///< Pointer to the graphics view used for rendering.
    QList<QGraphicsEllipseItem * > * ellipseList; ///< Is the vector of ellipses positioned in current target's [x,y]
    bool ellipseListInvisible; ///< Is set to true only if all ellipses in 'ellipseList' are invisible

    QList<class crossItem * > * pathHandler; ///< This list holds points to for path drawing if a history of targets movements is required.


public slots:
    /**
     * @brief When animation group finishes its animation sequence, the 'finished()' signal emitted starts this slot, so all related objects can be safely deleted.
     */
    void deleteAnimationGroup(void);

    /**
     * @brief This function will add all items from paths list to the scene. This is used if we have some history saved and switched into draw history mode.
     */
    void loadPathsList(void);

    /**
     * @brief Function will go throught all list and deletes all paths.
     */
    void clearPathsList(void);

    /**
     * @brief This method will remove all cross items from scene but will not remove objects itself.
     */
    void removePathsFromScene(void);
};

/******************************************* CUSTOM OPENGL WIDGET ********************************************/

class openGLWidget : public QGLWidget
{
    Q_OBJECT

public:
    /**
     * @brief The 'openGLwidget' class is derived from original QGLWidget
     *
     * By setting the viewport to this widget allows using openGL rendering engine and therefore may
     * improve performance capabilities of application. Also allows to use many features related with
     * openGL including 3D rendering. Here by reimplementing different protected functions many other
     * improvements can be achieved. Class is now prepared for future use.
     *
     */
    openGLWidget(QWidget *parent = 0, const QGLWidget *shareWidget = 0, Qt::WindowFlags f = 0);
};

/******************************************* CUSTOM GRAPHICS VIEW ********************************************/

class radarView : public QGraphicsView
{

    Q_OBJECT

public:
    /**
     * @brief The constructor sets basic parameters of the view and creates grid layout to allow rulers to be displayed in widget.
     * @param[in] scene Is the scene where all visualizations will be drawn.
     * @param[in] parent Is the parent widget (usually mainwindow).
     */
    radarView(class radarScene * scene, uwbSettings * setts, QMutex * settings_mutex, QWidget * parent = 0);

    /**
     * @brief Returns the pointer to 'QGLWidget', usually 'openGLWidget' if is present. Else returns NULL.
     * @return The pointer to 'QGLWidget' or 'openGLWidget'.
     */
    openGLWidget * getOpenGLWidget(void) { return openGLW; }

    /**
     * @brief This derived class can hold additional parameter about rotation angle that user has set up and retrieved it when needed.
     * @return The return value is the current rotation angle.
     */
    double getRotationAngle(void) { return currentRotation; }

    void moveToXYAnimation(const QPointF & target);

    void setRotationAngleAnimation(int target);

private:
    QGridLayout * radarViewLayout; ///< Grid layout for placing rulers or maybe in future another additional widgets.
    QWidget * emptyCorner; ///< Used to fill empty corner in grid layout.

    uwbSettings * settings;
    QMutex * settingsMutex;

    openGLWidget * openGLW; ///< Pointer to openGL widget if is present. Else this pointer must be NULL.

    double scaleFactor; ///< Represents the scale factor by which axis are multiplied when making zooming effect.
    short scaleDepth; ///< When smooth effect is enabled, this number is used for indicating the QTimer maximum repetitions doing multiplication axis by 'scaleFactor'.
    short scaleCounter; ///< Used for controlling if the 'scaleDepth' is reached.
    short scaleCounterGlobal; ///< Used for controlling the absolute scale state ('scaleCounter' is periodically zeroed after the animation is finished so after several scales we do not know what scale is the current). The current scale is scaleFactor*scaleCounterGlobal*scaleDepth.

    double dx; ///< Is the interval that is added to current x center position during animated centering.
    double dy; ///< Is the interval that is added to current y center position during animated centering.
    int moveToXYCounter; ///< Is used for couning the steps. Each centering takes 2 seconds (default).
    int moveToXYSteps; ///< Specifies how many steps should it take to center on some position.
    QPointF targetPoint; ///< Holds the target position where we want to center on.

    double currentRotation; ///< Holds the information about current scene rotation angle.
    int targetAngle; ///< Is the target angle during smooth transition rotation animation.
    int angleStep; ///< Is the step added to angle during animation.

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

public slots:
    void scaleInSlot(void);
    void scaleOutSlot(void);
    void setRotationAngle(int angle);
    void centerOnXYAnimationSlot(void);
    void sceneRotationAnimationSlot(void);
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

    bool tappingSequence; ///< If user is tapping the scene, this value is set to true so rendering may check tapping options.

    bool isOpenGL(QPainter *painter); ///< Checks if the openGL functionality is availible and returns true if yes. Otherwise returns false.

protected:
    /**
     * @brief This reimplemented function ensures that grid is painted as a background. How grid is being drawn can be set by user (colors, smoothness, etc)
     * @param[in] painter Painter object used for drawing/rendering on screen.
     * @param[in] rect Is the rendering area, usually all scene.
     */
    virtual void drawBackground(QPainter * painter, const QRectF & rect);
};


#endif // VISUALIZATION

