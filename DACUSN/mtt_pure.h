#ifndef MTT_PURE_H
#define MTT_PURE_H

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <QDebug>

#include "stddefs.h"

#define UNUSED(x) (void)x

// MAX_N commented because is availible in stddefs.h
//#define MAX_N               (10)        // max dimension of cost matrix in Munkres algorithm
                                        // (max number of targets)
#define EMPTY               (-1)
#define TRUE                (1)
#define FALSE               (0)

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define FREQUENCY         (13E9)        // radar frequency
#define SCANLENGHT        (4095)        // length of processed m-sequence
#define CH_NUMBER            (2)        // number of channels
#define SA                   (5)        // Software averaging
#define HA                 (256)        // Hardware averaging
#define uS                 (512)        // subsampling

#define LARGE_NUMBER       (100)        // arbitrary large value ...
#define MIN_Y_COORDINATE  (0.02)        // points [x,y] with y<MIN_Y_COORDINATE => [0,0]

#define INDEX_CORRECTION     (1)        // to compensate Matlab indexing from 1

#define SPEED_OF_LIGHT     (3E8)

// !!! code is fixed for 2 channels!!!
#define CH1                   (0)
#define CH2                   (1)

// control of finite state machine in MTT algorithm
#define SKIP_IR              (14)       // # of IRs not to process at the beginning of TRACK algorithm
// finite state machine states
#define START_MTT_INIT        (1)       // initialize all MTT variables and process 1-st IR
#define WAIT_MTT              (2)       // skip IRs
#define START_MTT_TRACK       (3)       // process next IR (from 1st up to START_IR)
#define MTT_TRACK             (0)       // process all other IRs


class mtt_pure
{
public:
    mtt_pure();
    ~mtt_pure();

    float* MTT(float* P_mem,float r[], float q[],float dif_d,float dif_fi, int min_OLGI,int min_NTI);

private:
    typedef float   real;                   // use 32-bit float format
    //typedef double  real;                 // use 64-bit double format (not tested!)
    typedef int     word;                   // use 32-bit integer format

    int center_previous[MAX_N][3];   // temporary memory between scans
    int out_det[2][SCANLENGHT];
    real TOA_m[2][MAX_N];

    float tmp[ SCANLENGHT ];           // temporary vector storage
    float data[ SCANLENGHT ];          // temporary vector storage
    float X[ SCANLENGHT ];             // temporary vector storage
    float Y[ SCANLENGHT  ];            // temporary vector storage
    float S_2[ SCANLENGHT ];           // temporary vector storage

    float IR_buffer[2][ SCANLENGHT ];
    float bg_estimation[ SCANLENGHT ];
    float data_out[ SCANLENGHT ];

    /* MTT variables and arrays */
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
    int start_ex_av; // this start was defined as static in original C library, here it is replaced with start_ex_av in 'exponential_bg_subtraction' function

    /* supporting functions */
    real mean_vector( real *inptr, word samples);
    real std_vector( real *inptr, word samples);
    real max_vector( real *inptr, word samples);
    real max_abs_vector( real *inptr, word samples);
    void clear_vector( real *inptr, word samples);
    void clear_int_vector( word *inptr, word samples);
    void abs_vector( real *inptr, word samples);
    real sum_vector( real *inptr, word samples );
    word round2( real in );
    word max_of_2( word a, word b);
    word find_vec( word in[], word n );
    void matrix_mul_4x2( real c[][2], real a[][2], real b[][2] );
    void matrix_mul_4x4( real c[][4], real a[][4], real b[][4] );
    void matrix_transpose_4x4( real out[][4], real in[][4] );
    void matrix_add_4x4( real c[][4], real a[][4], real b[][4] );
    void cartesian2polar( real x, real y, real *r, real *fi);
    void connected_covering (word value_ch_b, word ch, word cover_b, word w, word trace_con[], word c[]); // ???? definition not found in original C library
    void munkres( real costMat[][MAX_N], word rows, word cols, real *cost, word assign[][MAX_N] );
    void correction ( real Y_e[], real *P_e, real Y_p[], real *P_p, real r, real fi, real R[][2]);
    void prediction ( real Y_e_p[], real *P_e_p, real Q[][4], real Y_p[], real *P_p );
    void covering (word value_ch_b, word *ch, word m, word trace_con[], word c[], word *cover_b);
    void different_values( word trace_con[], word *ch, word m, word value_ch_b, word value_ch_e, word nn_ch, word c[]);
    void gate_checker ( real r, real fi, real Y_p[], real R[][2], real *P_p, word *m, real *c);
    void identical_values (word value_ch_b, word *ch, word m, word max_nn_tg, word center_previous[][3], word c[3]);
    void init_estimation (real r, real fi, real Y_e_2_init,real Y_e_4_init,real P_init[][4], real Y_e[], real *P_e);
    void t_integration( word *inptr, word t_size, word min_int, word nn_ch );
    void intersection2ellipses(real d0, real d1, real x1, real x2, real *x, real *y);
    float *trace_connection (int* inptr1,int* inptr2,int* TC,int t_size, int min_int, int m, int max_nn_tg, float* TOA_mem);
    void trace_connection2 (word out_det[][SCANLENGHT], word t_size, word min_int, word m, word max_nn_tg,
                           word center_previous[][3], word* TC, real TOA_m[][MAX_N]);
    void new_tg_ident ( word nn_NTI, word min_NTI, word NTI[], real r_last_obs[], real fi_last_obs[],
                        real r, real fi, real dif_d, real dif_fi, word nn_track, word cl[], word max_nn_tr,
                        word *new_track, word clearing[]
                      );
    void obs_less_gate_ident (word *OLGI_last,word track, word nn_track, real Y_e_last[], real *P_e_last,
                              real Y_e[], real *P_e, word *OLGI);
    void polar2cartesian (real Y_e[], real *x, real *y);
    void point_targets(word nn_ch, word int_IR[], word min_int, word m, word rx, word first_reflection[] );
    float *MT_localization (float* TOA_mem, float x1, float x2);

    float *normalizing(float *inptr);
    float *normalizing_new(float *inptr );
    void connected_reflections(word* ch, word trace_con[],word c[],word m,word* nn_obs);

    /* munkres support function */
    void zeros( real in[][MAX_N], word m, word n );
    void log_negate(real in[][MAX_N], word m, word n, word out[][MAX_N]);
    void falses(word in[][MAX_N], word m, word n);
    void falses_vector(word in[], word n);
    word any(word in[][MAX_N], word m, word n);
    word any_vector(word in[], word n);
    word any_not_selected(word in[][MAX_N], word row_sel[], word col_sel[], word m, word n);
    word any_vector_not_selected(word in[], word m);
    void any_col(word in[][MAX_N], word m, word n, word out[]);
    void find(word in[][MAX_N], word m, word n, word *r, word *s);
    void copy_row(word in[][MAX_N], word m, word n, word out[]);
    void copy_col(word in[][MAX_N], word m, word n, word out[]);
    real find_min(real in[][MAX_N], word m, word n);
    void extract_valid(real in[][MAX_N], word row_sel[], word col_sel[], real out[][MAX_N], word m, word n);
    void extract_not_valid(real in[][MAX_N], word row_sel[], word col_sel[], word m, word n, real out[][MAX_N], word *r, word *c);
    void insert_to_not_valid(real in[][MAX_N], word m, word n, real out[][MAX_N], word row_sel[], word col_sel[]);
    void insert_to_valid(word in[][MAX_N], word m, word n, word out[][MAX_N], word row_sel[], word col_sel[]);

    /* radar data processing functions */
    float *exponential_bg_subtraction (float* data_in, int ch, float exp_factor);
    int *detector_cfar( float *inptr,int* outptr, float beta_fast, float beta_slow, float alpha_det );
    void trace_online( real *inptr, word ch, word t_size, real thr, word min_integration,                  word h_window, word m, word samples);
    void TOA_couple (word max_nn_tg, word center[][3], word m, real TOA_m[][MAX_N]);
    void InitScan( void );

    /* MTT supporting functions */
    void MTT2 (real P[][MAX_N], real r[], real q[], real dif_d, real dif_fi, word min_OLGI, word min_NTI, real T[][MAX_N], word *start );
    word find_max ( mtt_pure::word in[][MAX_N], mtt_pure::word row );
};

#endif // MTT_PURE_H
