#ifndef STACKMANAGER_H
#define STACKMANAGER_H

#include <QObject>
#include <QMutex>
#include <QVector>
#include <QPointF>
#include <QList>
#include <QThread>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <limits>

#include <QDebug>

#include "rawdata.h"
#include "stddefs.h"
#include "uwbsettings.h"
#include "radar_handler.h"
#include "radarsubwindow.h"

/**
 * @file reciever.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class which provides interface for data stack management
 *
 * @section DESCRIPTION
 *
 * The 'stackManager' class provides a basic interface for manipulation with the stack.
 * After the data are recieved by 'reciever', they are pushed onto the stack which then
 * must be protected by mutex. This stack, however can obtain more data, than this class
 * may be able to process. Reasons may differ (too many input data, very complex processing
 * algorithms, time consuming rendering session etc.). In such cases some of special methods
 * are needed to solve this issue. The stack must free memory in time and 'rawData' objects
 * must be deleted after processing. The instance based on this class serves as a worker
 * object for stack management thread. This thread is never deleted and remains active
 * during the application run. Only actions applied on the reciever thread as pause, stop will
 * pause/unpause the thread.
 *
 */

class stackManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief The main constructor of 'stackManager' class will prepare the instance for first start.
     * @param[in] raw_data_stack Is the pointer to the stack of recieved data.
     * @param[in] raw_data_stack_mutex The mutex locking the stack during data processing.
     * @param[in] setts The pointer to basic settings object.
     * @param[in] settings_mutex The mutex locking the settings object during reading some settings value.
     * @param[in] radar_list Is the list with all radar handler structures that are currently registered by application.
     * @param[in] radar_list_mutex The mutex locking the radar_list object during reading/writing values or doing some processing on recieved data.
     *
     * This constructor will obtain all pointers and mutex pointers when creating the object from the main
     * application thread. These pointers are saved and a few important values are initialized to their
     * default values (later they are rewritten by settings values if they are specified).
     */
    stackManager(QVector<rawData * > * raw_data_stack, QMutex * raw_data_stack_mutex, QVector<radar_handler * > * radar_list, QMutex * radar_list_mutex,
                 QList<QPointF * > * visualization_data, QList<QColor * > * visualization_color, QList<radarSubWindow * >  * radar_Sub_Window_List, QMutex * radar_Sub_Window_List_Mutex, QMutex * visualization_data_mutex,
                 uwbSettings * setts, QMutex * settings_mutex);
    ~stackManager();

    /**
      * @brief The main function of stack management thread, running in infinite loop.
      */
    Q_INVOKABLE void runWorker(void);

    /**
     * @brief Switches the pause state value.
     */
    void switchPauseState(void);

    /**
     * @brief This method will change the stop indicator to true and ensures that in next repetition of the infinite cycle, this will break
     */
    void stopWorker(void);

    /**
     * @brief Checks if the worker is still in infinite cycle.
     * @return The return value indicates if the infinite cycle is still running
     */
    bool checkStoppedStatus(void);

    /**
     * @brief This method is usually used only when destroying the thread to continue in infinite cycle if worker is paused so it can meet the stop condition.
     */
    void releaseIfInPauseState(void);

    /**
     * @brief Returns the avarage processing time in nanoseconds. Mutexes and all needed stuff is done inside, therefore upper classes does not need to take care about mutex protection.
     * @return The avarage processing time in nanoseconds.
     */
    qint64 getAverageProcessingSpeed(void);

    /**
     * @brief Returns the last iteration processing time in nanoseconds. Mutexes and all needed stuff is done inside, therefore upper classes does not need to take care about mutex protection.
     * @return The lastly calculated processing time in nanoseconds.
     */
    qint64 getCurrentProcessingSpeed(void);

private:
    int active_radar_ID; ///< This variable holds information about, what radar data should be pumped into visualizationData list.
    int active_radar_ID_index; ///< This variable stores index of active radar specified by active_radar_ID in radarList.

    QVector<rawData * > * rawDataStack; ///< Pointer to the stack
    QMutex * rawDataStackMutex; ///< Pointer to the mutex locking the stack
    uwbSettings * settings; ///< Pointer to the basic application settings object
    QMutex * settingsMutex; ///< Pointer to the mutex locking the settings object
    QList<QPointF * > * visualizationData; ///< The final positions of targets
    QList<radarSubWindow * >  * radarSubWindowList; ///< If some subwindows for displaying specific radar data are created, we need to push relevant values into their prive vectors.
    QList<QColor * > * visualizationColor; ///< The colors assigned to all targets
    QMutex * visualizationDataMutex; ///< Mutex protecting visualization data from being accessed by multiple threads at the same time
    QMutex * radarSubWindowListMutex; ///< Mutex protecting radar sub windows vectors from being accessed by multiple threads at the same time

    unsigned int idleTime; ///< The idle time, during the thread is sleeping after it find out that the stack is empty
    unsigned int stackControlPeriodicity; ///< Number of cycles that must pass until the 'stackControl' function is run
    unsigned int lastStackCount; ///< The last known stack items count from the lastly done stack control
    unsigned int stackWarningCount; ///< The warning count is incremented when the stack control find some undesired issue
    unsigned int maxStackWarningCount; ///< The maximum warning count until the 'rescue' function is run

    bool rescueEnabled; ///< Determines wether run the rescue function or just inform the user about critical state

    QMutex * pauseMutex; ///< Mutex protecting the pause variable from being accessed by multiple threads at once
    bool pauseState; ///< Determines if pause state is on/off
    QWaitCondition * pause; ///< The wait condition that ensures the correct pause of worker cycle in stack management thread

    QMutex * stoppedMutex; ///< Mutex protecting the stop variable from being accessed by multiple threads at once
    bool stopped; ///< If this value is true, the infinite cycle will break

    QMutex * stoppedCheckMutex; ///< Mutex protecting the stopCheck variable from being accessed by multiple threads at once
    bool stoppedCheck; ///< If this value is set to true, the infinite loop has been finished

    QVector<radar_handler * > * radarList; ///< Is the list with all radar handler structures that are currently registered by application.
    QMutex * radarListMutex; ///< The mutex locking the radar_list object during reading/writing values or doing some processing on recieved data.

    qint64 averageProcessingSpeed; ///< Holds the average processing time since the stack manager started.
    qint64 currentProcessingSpeed; ///< Holds the last processing time in nanoseconds.
    qint64 processingIterator; ///< Counts how many processing iterations there were so far.

    /**
     * @brief Function is used to check values that come from MTT. Sometimes they can be NaN or +-infinite. These values should not be considered
     * @param[in] x X-coordinate of target.
     * @param[in] y Y-coordinate of target.
     * @return The return value is true, if values are valid, false if one or both of values are NaN or +-infinite.
     */
    bool coordinatesAreValid(float x, float y);

    /**
     * @brief This function will control the stack situation and its behaviour in time (checks for speed of data input) and prevent the stack from being filled too fastly.
     * @param[in] stackControlCounter Is the adress of variable declared inside the thread and serves as a counter for 'stackControl' function.
     */
    void stackControl(unsigned int * stackControlCounter);

    /**
     * @brief The data processing function will obtain a pointer to 'rawData' object and process the obtained data and passes them to 'radarUnit' objects.
     * @param[in] data Is the pointer to the 'rawData' object lastly token from the stack.
     */
    void dataProcessing(rawData * data);

    /**
     * @brief Checks for all known 'radarUnit' objects if they were updated with new data.
     * @return The return value is true if all 'radarUnit' objects are updated with new data.
     */
    bool checkRadarDataUpdateStatus(void);

    /**
     * @brief This function is applying the fusion algorithm and updating visualization list.
     */
    void applyFusion(void);

    /**
     * @brief Clears all vector with positions of targets in the scene and prepares the vector for new data.
     */
    void clearVisualizationData(void);

    /**
     * @brief While updating data, we must add specific radar data to the privet vectors contained in radar subwindows.
     */
    void updateRadarSubWindowList(void);

    /**
     * @brief Function will make backup (write to file) in file specified in settings object from value passed as parameter.
     * @param[in] val Value to be written.
     * @param[in] write_val If is set to true, value will be written.
     * @param[in] newline If new line is true, no separator will be prepended.
     * @param[in] endline If end line is true, '\n' will be appended.
     */
    void makeDataBackup(float val, bool write_val=true, bool newline=false, bool endline=false);

    /**
     * @brief This is overloaded function. Allows writing strings into file instead the float numbers. Used primarily for proving time information.
     * @param[in] val String value to be written.
     * @param[in] write_val If is set to true, value will be written.
     * @param[in] newline If new line is true, no separator will be prepended.
     * @param[in] endline If end line is true, '\n' will be appended.
     */
    void makeDataBackup(QString val, bool write_val=true, bool newline=false, bool endline=false);

    /**
     * @brief This is overloaded function. Allows writing epochal time in qint64 format into file.
     * @param[in] val qint64 value to be written.
     * @param[in] write_val If is set to true, value will be written.
     * @param[in] newline If new line is true, no separator will be prepended.
     * @param[in] endline If end line is true, '\n' will be appended.
     */
    void makeDataBackup(qint64 val, bool write_val=true, bool newline=false, bool endline=false);


    /********************************************************************************************************/
    /********************************************************************************************************/
    /********************************************************************************************************/
    /**************************************** DATA PROCESSING FUNCTIONS *************************************/
    /********************************************************************************************************/
    /********************************************************************************************************/
    /********************************************************************************************************/

    void matrix_mul_4x2( real c[][2], real a[][2], real b[][2] );
    void matrix_mul_4x4( real c[][4], real a[][4], real b[][4] );
    void matrix_transpose_4x4( real out[][4], real in[][4] );
    void matrix_add_4x4( real c[][4], real a[][4], real b[][4] );
    void prediction ( real Y_e_p[], real *P_e_p, real Q[][4], real Y_p[], real *P_p );
    void init_estimation (real r, real fi, real Y_e_2_init,real Y_e_4_init,real P_init[][4], real Y_e[], real *P_e);
    void cartesian2polar( real x, real y, real *r, real *fi);
    void polar2cartesian ( real Y_e[], real *x, real *y);
    void gate_checker ( real r, real fi, real Y_p[], real R[][2], real *P_p, word *m, real *c);
    void obs_less_gate_ident (word *OLGI_last,word track, word nn_track, real Y_e_last[], real *P_e_last,
                              real Y_e[], real *P_e, word *OLGI);
    void munkres( real costMat[][MAX_N], word rows, word cols, real *cost, word assign[][MAX_N] );
    void insert_to_valid( word in[][MAX_N], word m, word n, word out[][MAX_N], word row_sel[], word col_sel[] );
    void insert_to_not_valid( real in[][MAX_N], word m, word n, real out[][MAX_N], word row_sel[], word col_sel[] );
    void extract_not_valid( real in[][MAX_N], word row_sel[], word col_sel[], word m, word n, real out[][MAX_N], word *r, word *c);
    void extract_valid( real in[][MAX_N], word row_sel[], word col_sel[], real out[][MAX_N], word m, word n);
    real find_min ( real in[][MAX_N], word m, word n );
    word find_max ( word in[][MAX_N], word row );
    void copy_col ( word in[][MAX_N], word m, word n, word out[] );
    void copy_row ( word in[][MAX_N], word m, word n, word out[] );
    void find( word in[][MAX_N], word m, word n, word *r, word *s );
    void any_col( word in[][MAX_N], word m, word n, word out[]  );
    word any_vector_not_selected ( word in[], word m );
    word any_not_selected ( word in[][MAX_N], word row_sel[], word col_sel[], word m, word n );
    word any_vector( word in[], word n );
    word any( word in[][MAX_N], word m, word n );
    void falses_vector( word in[], word n );
    void falses( word in[][MAX_N], word m, word n );
    void log_negate( real in[][MAX_N], word m, word n, word out[][MAX_N]  );
    void zeros( real in[][MAX_N], word m, word n );
    word find_vec( word in[], word n );
    void correction ( real Y_e[], real *P_e, real Y_p[], real *P_p, real r, real fi, real R[][2]);
    void new_tg_ident ( word nn_NTI, word min_NTI, word NTI[], real r_last_obs[], real fi_last_obs[],
                        real r, real fi, real dif_d, real dif_fi, word nn_track, word cl[], word max_nn_tr,
                        word *new_track, word clearing[]
                      );

    // MTT functions
    float* MTT(float* P_mem,float r[], float q[],float dif_d,float dif_fi, int min_OLGI,int min_NTI);
    void MTT2 (real P[][MAX_N], real r[], real q[], real dif_d, real dif_fi, word min_OLGI, word min_NTI, real T[][MAX_N], word *start );

    /********************************************************************************************************/
    /********************************************************************************************************/
    /********************************************************************************************************/
    /**************************************** DATA PROCESSING VARIABLES *************************************/
    /********************************************************************************************************/
    /********************************************************************************************************/
    /********************************************************************************************************/

    real P_init[4][4];
    real Y_e_2_init;
    real Y_e_4_init;
    real R[2][2];
    real Q[4][4];


    real Z[MAX_N][2];
    real Y_p[MAX_N][4];
    real P_p[MAX_N][4][4];
    real K[MAX_N][4][2];
    real Y_e[MAX_N][4];
    real P_e[MAX_N][4][4];
    word M[MAX_N][MAX_N];
    word MA[MAX_N][MAX_N];
    real C[MAX_N][MAX_N];
    real P[2][MAX_N];
    real T[2][MAX_N];

    word NTI[3];
    real last_obs[2][3];
    word OLGI[MAX_N];
    word nn_obs, obs, start_im, nn_track;
    word start;



public slots:
    /**
     * @brief This function is called always when the processing is slower than the data obtaining and there is a danger that stack will be overflowed.
     */
    void rescue(void);

    /**
     * @brief Changes active radar ID so data from specified radar will be pumped into visualizationData list
     * @param[in] id New radar to be considered when filling visualizationData list with new values.
     */
    void changeActiveRadarId(int id);

signals:
    /**
     * @brief This signal is emitted when runWorker() function will reach its end.
     */
    void finished(void);
};

#endif // STACKMANAGER_H
