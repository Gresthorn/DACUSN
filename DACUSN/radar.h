/***************************************************************************/
/* Conversion of Matlab code to C code for new UWB radar prototype         */
/*                                                                         */
/* project: Radiotect                                                      */
/* autor:   Milos Drutarovsky (MD)                                         */
/* v.       1.00  2009-02-28                                               */
/***************************************************************************/


#ifndef RADAR_H
#define RADAR_H

#include <stdio.h>

typedef float   real;                   // use 32-bit float format
//typedef double  real;                 // use 64-bit double format (not tested!)
typedef int     word;                   // use 32-bit integer format

#define SCANLENGHT         (4095)        // length of processed m-sequence
#define CH_NUMBER            (2)        // number of channels
#define FREQUENCY        (13E9)        // radar frequency
#define SA                   (5)        // Software averaging
#define HA                 (256)        // Hardware averaging
#define uS                 (512)        // subsampling
#define SPEED_OF_LIGHT     (3E8)

#define MAX_N               (10)        // max dimension of cost matrix in Munkres algorithm
                                        // (max number of targets)
#define LARGE_NUMBER       (100)        // arbitrary large value ...
#define MIN_Y_COORDINATE  (0.02)        // points [x,y] with y<MIN_Y_COORDINATE => [0,0]
#define EMPTY               (-1)

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


/* radar data processing functions */
float* exponential_bg_subtraction (float* data_in, int ch, float exp_factor);
int* detector_cfar( float *inptr,int* outptr, float beta_fast, float beta_slow, float alpha_det );

void trace_online( real *inptr, word ch, word t_size, real thr, word min_integration,
                   word h_window, word m, word samples);
void TOA_couple (word max_nn_tg, word center[][3], word m, real TOA_m[][MAX_N]);

void InitScan( void );



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
void connected_covering (word value_ch_b, word ch, word cover_b, word w, word trace_con[], word c[]);
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
float* trace_connection (int* inptr1,int* inptr2,int* TC,int t_size, int min_int, int m, int max_nn_tg, float* TOA_mem);
void new_tg_ident ( word nn_NTI, word min_NTI, word NTI[], real r_last_obs[], real fi_last_obs[],
                    real r, real fi, real dif_d, real dif_fi, word nn_track, word cl[], word max_nn_tr,
                    word *new_track, word clearing[]
                  );
void obs_less_gate_ident (word *OLGI_last,word track, word nn_track, real Y_e_last[], real *P_e_last,
                          real Y_e[], real *P_e, word *OLGI);
void polar2cartesian (real Y_e[], real *x, real *y);
void point_targets(word nn_ch, word int_IR[], word min_int, word m, word rx, word first_reflection[] );
float* exponential_bg_subtraction (float* data_in, int ch, float exp_factor);
float* MT_localization (float* TOA_mem, float x1, float x2);


float* MTT(float* P_mem,float r[], float q[],float dif_d,float dif_fi, int min_OLGI,int min_NTI);

float* normalizing(float *inptr);
float* normalizing_new(float *inptr );
void connected_reflections(word* ch, word trace_con[],word c[],word m,word* nn_obs);
#endif
