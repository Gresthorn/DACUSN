#ifndef RADARUNIT_H
#define RADARUNIT_H

#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <QDebug>

#include "uwbsettings.h"
#include "stddefs.h"
#include "rawdata.h"


/**
 * @file radarunit.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class for processing raw data and their storage until they are graphically represented
 *
 * @section DESCRIPTION
 *
 * The radarUnit class is the target where all data are stored after they are processed. The objects
 * based on this class are stored transparently in list placed in main window object. We can consider
 * them as the virtual interpretation of real radars. Each radar unit has own identification and will
 * be processing the only data that belongs to it. Each radar unit is also independently on the others
 * making its own protocol about how many targets are visible, how are they organized, ... The final
 * data are stored in vectors. Also color assignments are done by the private methods and they are stored
 * in needed order in vectors of same size as vectors for coordinate storage.
 */


typedef float   real;                   ///< use 32-bit float format
//typedef double  real;                 ///< use 64-bit double format (not tested!)
typedef int     word;                   ///< use 32-bit integer format

#define SCANLENGHT         (4095)       ///< length of processed m-sequence
#define CH_NUMBER            (2)        ///< number of channels
#define FREQUENCY        (13E9)         ///< radar frequency
#define SA                   (5)        ///< Software averaging
#define HA                 (256)        ///< Hardware averaging
#define uS                 (512)        ///< subsampling
#define SPEED_OF_LIGHT     (3E8)

#define MAX_N               (10)        ///< max dimension of cost matrix in Munkres algorithm
                                        ///< (max number of targets)
#define LARGE_NUMBER       (100)        ///< arbitrary large value ...
#define MIN_Y_COORDINATE  (0.02)        ///< points [x,y] with y<MIN_Y_COORDINATE => [0,0]
#define EMPTY               (-1)

///< !!! code is fixed for 2 channels!!!
#define CH1                   (0)
#define CH2                   (1)

///< control of finite state machine in MTT algorithm
#define SKIP_IR              (14)       ///< # of IRs not to process at the beginning of TRACK algorithm
///< finite state machine states
#define START_MTT_INIT        (1)       ///< initialize all MTT variables and process 1-st IR
#define WAIT_MTT              (2)       ///< skip IRs
#define START_MTT_TRACK       (3)       ///< process next IR (from 1st up to START_IR)
#define MTT_TRACK             (0)       ///< process all other IRs
#define PI      (3.141592653589793)
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define TRUE    (1)
#define FALSE   (0)

class radarUnit
{
public:
    /**
     * @brief This construtor will ensure correct allocation for all data needed when program running
     * @param[in] radarId Is the radar identificator.
     */
    radarUnit(int radarId);
    ~radarUnit();

    /**
     * @brief This function will read data from the container and apply MTT processing functions.
     * @param[in] data Container with all data obtained by reciever and retrieved from stack.
     * @return If the data processing was successful, the return value is true and the data in radarUnit are considered as updated.
     */
    bool processNewData(class rawData * data);

private:
    int radar_id; ///< Main radar identificator



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
};

#endif // RADARUNIT_H
