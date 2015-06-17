#include "mtt_pure.h"

mtt_pure::mtt_pure()
{
    start = 1;
    start_ex_av = 0;
}

mtt_pure::~mtt_pure()
{
    // so far no dynamic allocation was used - returned array will managed by external functions
}

float *mtt_pure::MTT(float *P_mem, float r[], float q[], float dif_d, float dif_fi, int min_OLGI, int min_NTI)
{
    if(P_mem==NULL) return NULL;

    /* FOR TEST PURPOSES ONLY - TESTING IF COVARIANCE MATRIX IS BEING CHANGED DURING ITERATIONS */
    qDebug() << "Matrix Q: ";
    for(int i = 0; i<4; i++)
        qDebug() << Q[0] << " " << Q[1] << Q[2] << " " << Q[3];

    qDebug() << "Matrix P_p: ";
    for(int i = 0; i<MAX_N; i++)
    {
        for(int j = 0; j<4; j++)
            qDebug() << P_p[0] << " " << P_p[1] << P_p[2] << " " << P_p[3];
    }

    qDebug() << "P_mem echo";
    for(int i = 0;i<MAX_N; i++)
        qDebug() << i << ". " << P_mem[i*2] << " " << P_mem[i*2+1];

    int k;

    /*
     * Jednorozmerny vstup na dvojrozmerne pole
     */
    for(k=0;k<MAX_N;k++){
        P[0][k]=P_mem[2*k];
        P[1][k]=P_mem[2*k+1];
    }
    //Funkciu som zabalil aby sa zbavil parametra &start
    MTT2(P,r,q,dif_d,dif_fi,min_OLGI,min_NTI,T,&start);
    /*
     *Transformacia spat
     */
    for(k=0;k<MAX_N;k++){
        P_mem[2*k]=T[0][k];
        P_mem[2*k+1]=T[1][k];
    }
    return P_mem;
}

void mtt_pure::MTT2(mtt_pure::real P[][MAX_N], mtt_pure::real r[], mtt_pure::real q[], mtt_pure::real dif_d, mtt_pure::real dif_fi, mtt_pure::word min_OLGI, mtt_pure::word min_NTI, mtt_pure::real T[][MAX_N], mtt_pure::word *start )
{

    mtt_pure::word k, j, t, track;
    mtt_pure::real cost;
    mtt_pure::word A[MAX_N][MAX_N];
    mtt_pure::word max_MA, tmp;
    mtt_pure::word Clearing[3];
    mtt_pure::word new_track;


    mtt_pure::word nn_NTI;

    /* nn_NTI = 3; % pocet identifikatorov novych cielov */
       nn_NTI = 3;

    switch( *start ) {
            case START_MTT_INIT:

            /********** START_MTT_INIT ****************/
            /*
            [nn_co, max_nn_tr, nn_im] = size(P);
            T = zeros(nn_co, max_nn_tr, nn_im);

            % nastavitelne parametre
            p_init = [0.01 0.01 0.01 0.01]; % prvky na diagonale kovariancnej matice
            P_init = diag(p_init); % kovariancna matica
            Y_e_2_init = 0.1; % pociatocny odhad rychlosti
            Y_e_4_init = 0.1; % pociatocny odhad uhlovej rychlosti

            */
            for( k=0; k<MAX_N; k++ ) {
                 T[0][k] = 0;
                 T[1][k] = 0;
            }

            for( k=0; k<4; k++ ) {
                for( j=0; j<4; j++ ) {
                    P_init[k][j] = 0;
                }
            }
            P_init[0][0] = P_init[1][1] = P_init[2][2] = P_init[3][3] = (real) 0.01;

            Y_e_2_init = (real) 0.1;
            Y_e_4_init = (real) 0.1;

            /*
            R = diag(r);
            Q = diag(q);
            */

           R[0][1] = R[1][0] = 0.0;
           R[0][0] = r[0];
           R[1][1] = r[1];

           for( k=0; k<4; k++ ) {
                for( j=0; j<4; j++ ) {
                    Q[k][j] = 0;
                }
            }
            Q[0][0] = q[0];
            Q[1][1] = q[1];
            Q[2][2] = q[2];
            Q[3][3] = q[3];

            /*
            % inicializacia premennych
            Z = zeros (nn_co,max_nn_tr,nn_im); % observations
            Y_p = zeros(4,1,max_nn_tr); % state prediction vectors
            P_p = zeros(4,4,max_nn_tr); % prediction error covariance matrix
            K = zeros(4,2,max_nn_tr); % Kalman gain matrix
            Y_e = zeros(4,1,max_nn_tr); % state estimation vectors
            P_e = zeros(4,4,max_nn_tr); % estimation covariance matrix
            */
            for( k=0; k<MAX_N; k++ ) {
                Z[k][0] = 0;
                Z[k][1] = 0;
            }

            for( k=0; k<4; k++ ) {
                for( j=0; j<MAX_N; j++ ) {
                    Y_p[j][k] = 0;
                    Y_e[j][k] = 0;
                    K[j][k][0] = 0;
                    K[j][k][1] = 0;
                }
            }

            for( k=0; k<4; k++ ) {
                for( j=0; j<MAX_N; j++ ) {
                        for( t=0; t<4; t++ ) {
                            P_p[j][t][k] = 0;
                            P_e[j][t][k] = 0;
                        }
                }
            }
            /*
            NTI = zeros(1,nn_NTI); % new target identification
            last_obs = zeros(2,nn_NTI); % posledne pozorovanie
            OLGI = zeros(1,max_nn_tr); % observation-less gate identification
            */
            NTI[0] = NTI[1] = NTI[2] = 0;
            last_obs[0][0] = last_obs[0][1] = last_obs[0][2] = 0;
            last_obs[1][0] = last_obs[1][1] = last_obs[1][2] = 0;
            for( k=0; k< MAX_N; k++ ) {
                OLGI[k] = 0;
            }


            *start = WAIT_MTT;
            start_im = SKIP_IR;
            /********** END of START_MTT_INIT ****************/

            case WAIT_MTT:
            /********** START_WAIT_MTT ****************/
            /*
            % nastartovanie prveho tracku na prvom pozorovani
            nn_obs = 0;
            start_im = START_TRACK_IM;
            */
            if( start_im-- )
                  return;
            *start = START_MTT_TRACK;
            /********** END of START_WAIT_MTT ****************/

            case START_MTT_TRACK:

            /********** START_MTT_TRACK ****************/

            /*
            % nastartovanie prveho tracku na prvom pozorovani

            while nn_obs == 0
               obs = 1;
                    if P(1,obs,start_im) ~= 0 || P(2,obs,start_im) ~= 0
                        nn_obs = nn_obs + 1;
                        Z(:,nn_obs,start_im) = cartesian2polar (P(:,obs,start_im));
                        % prepocet kartezianskych suradnic na polarne
                        [Y_e(:,:,nn_obs),P_e(:,:,nn_obs)] = init_estimation ...
                                   (Z(:,nn_obs,start_im),Y_e_2_init,Y_e_4_init,P_init);
                        % inicializacia Y_e, P_e
                    end
               start_im = start_im + 1;
            end

            nn_track = nn_obs; % jedno pozorovanie, jeden track
            */

            nn_obs = 0;
            if ( nn_obs == 0 ) {
               obs = 0;
               if( (P[0][obs] != 0) || (P[1][obs] != 0) ) {
                   cartesian2polar( P[0][obs], P[1][obs], &Z[nn_obs][0], &Z[nn_obs][1] );
                   init_estimation (Z[nn_obs][0], Z[nn_obs][1], Y_e_2_init, Y_e_4_init, P_init, &Y_e[nn_obs][0], &P_e[nn_obs][0][0]);
                   nn_obs++;
                   nn_track = nn_obs;
                   *start = MTT_TRACK;
                   break;

               }
            }
            break;

            /********** END of START_MTT_TRACK ****************/
            case MTT_TRACK:
            default:
            /*
                nn_obs = 0;
                for obs = 1 : max_nn_tr
                    if P(1,obs,im) ~= 0 || P(2,obs,im) ~= 0
                        nn_obs = nn_obs + 1;
                        Z(:,nn_obs,im) = cartesian2polar (P(:,obs,im));
                        % prepocitavanie kartezianskych suradnic na polarne
                    end
                end
            */
            for( k=0; k<MAX_N; k++ ) {      // Matlab code uses 3-d dimmension (im)
               Z[k][0] = 0;
               Z[k][1] = 0;
            }

            nn_obs = 0;
            for (obs=0; obs < MAX_N; obs++) {
                if ( (P[0][obs] != 0) || (P[1][obs] != 0) ) {
                    cartesian2polar( P[0][obs], P[1][obs], &Z[nn_obs][0], &Z[nn_obs][1] );
                    nn_obs = nn_obs + 1;
                }
            }

            /*
                if nn_obs > 0 && nn_track == 0 % pripad, ked su vymazane vsetky tracky
                    nn_track = 1;
                    [Y_e(:,:,nn_track),P_e(:,:,nn_track)] = init_estimation ...
                                       (Z(:,nn_track,im),Y_e_2_init,Y_e_4_init,P_init);
                    % novy track nastartujem prvym dostupnym pozorovanim
                end
            */

            if ( (nn_obs > 0) && (nn_track == 0) ) {
                 init_estimation (Z[nn_track][0], Z[nn_track][1], Y_e_2_init, Y_e_4_init, P_init, &Y_e[nn_track][0], &P_e[nn_track][0][0]);
                 nn_track = 1;
            }

            /*
                M = zeros(nn_obs,nn_track); % matica "Gate Mask"
                C = 10^2*ones(nn_obs,nn_track); % nakladova matica
            */
                for( k=0; k<MAX_N; k++ ) {
                     for( j=0; j<MAX_N; j++ ) {
                         M[k][j] = 0;
                         C[k][j] = 0;
                     }
                }

                for( k=0; k<nn_obs; k++ )
                    for( j=0; j<nn_track; j++ )
                            C[k][j] = LARGE_NUMBER;



            /*
                for track = 1 : nn_track
                    [Y_p(:,:,track), P_p(:,:,track)] = prediction ...
                                        (Y_e(:,:,track),P_e(:,:,track),Q); % predikcia
                    for obs = 1 : nn_obs
                        [M(obs,track),C(obs,track)] = gate_checker ...
                            (Z(:,obs,im),Y_p(:,:,track),R,P_p(:,:,track)); % kontrola
                        % "bran", t.j. ktore pozorovanie je v blizkosti ktoreho tracku
                    end
                end
            */
                 for( track=0; track<nn_track; track++ ) {
                      prediction ( &Y_e[track][0], &P_e[track][0][0], Q, &Y_p[track][0], &P_p[track][0][0] );
                      for( obs=0; obs<nn_obs; obs++ ) {
                          gate_checker ( Z[obs][0], Z[obs][1], &Y_p[track][0], R, &P_p[track][0][0], &M[obs][track],  &C[obs][track]);
                      }
                 }


        if( nn_obs == 0) {
        /*
           if nn_obs == 0 % pripad, ked nie je dostupne ziadne pozorovanie
                for track = 1 : nn_track
                    OLGI(track) = OLGI(track) + 1; % zvysi sa "observation-less
                    % gate identificator" pre kazdy track
                    if OLGI(track) >= min_OLGI % podmienka pre vylucenie tracku
                        [Y_e(:,:,track),P_e(:,:,track),Y_e(:,:,nn_track),...
                            P_e(:,:,nn_track),OLGI(track),OLGI(nn_track)] = ...
                                obs_less_gate_ident ...
                                    (OLGI(nn_track),track,nn_track,...
                                          Y_e(:,:,nn_track),P_e(:,:,nn_track));
                        nn_track = nn_track - 1;
                    end
                end
        */
                        for( track=0; track<nn_track; track++ ) {
                OLGI[track]++;
                if (OLGI[track] >= min_OLGI) {
                    obs_less_gate_ident (&OLGI[nn_track], track, nn_track, &Y_e[nn_track][0], &P_e[nn_track][0][0],
                              &Y_e[track][0], &P_e[track][0][0], &OLGI[track]);
                    nn_track--;
                }
            }
        }
        else {
        /*
            else % k dispozicii su aj pozorovania, aj tracky
                [A,cost] = munkres(C); % vyriesenie priradovacieho problemu
                MA = M.*A; % matica MA dava vierohodnejsie vysledky ako matica A,
                % ktora najde priradenie aj v pripade nuloveho riadku alebo stlpca
                % matice M (co je nespravny postup)
                if max(MA) == 0
                    MA = M;
                end
        */
            munkres( C, nn_obs, nn_track, &cost, A );
            max_MA = INT_MIN;
            for( k=0; k<nn_obs; k++ ) {
                for( j=0; j<nn_track; j++ ) {
                     tmp =  M[k][j]*A[k][j];
                     MA[k][j] = tmp;
                     if ( tmp > max_MA)
                            max_MA = tmp;
                }
            }

                    if (max_MA == 0 ) {
                for( k=0; k<nn_obs; k++ ) {
                    for( j=0; j<nn_track; j++ ) {
                        MA[k][j] = M[k][j];
                                    }
                            }
                    }
            /*
                    for track = 1 : nn_track
                        obs = find(MA(:,track)==1,1);
                        if isempty(obs) == 0 % existuje priradenie
                            [Y_e(:,:,track), P_e(:,:,track)] = ...
                            correction(Y_p(:,:,track),P_p(:,:,track),Z(:,obs,im),R);
                            % korekcia predikcie podla priradeneho pozorovania
                            OLGI(track) = 0; % vynulovanie identifikatora
                        else
                            % Observation-less Gate Identification
                            OLGI(track) = OLGI(track) + 1;
                            if OLGI(track) >= min_OLGI % podmienka pre vylucenie tracku
                                [Y_e(:,:,track),P_e(:,:,track),Y_e(:,:,nn_track),...
                                    P_e(:,:,nn_track),OLGI(track),OLGI(nn_track)] = ...
                                    obs_less_gate_ident(OLGI(nn_track),track,...
                                    nn_track,Y_e(:,:,nn_track),P_e(:,:,nn_track));
                                    nn_track = nn_track - 1;
                            end
                        end
                     end % koniec cyklu cez vsetky tracky
                end
            */
             for( track=0; track<nn_track; track++ ) {
                obs = EMPTY;
                for( k=0; k< nn_obs; k++ ) {
                    if ( obs == EMPTY) {
                        if( MA[k][track] == 1 )
                            obs = k;
                    }
                }
                if ( obs != EMPTY) {
                    correction ( &Y_e[track][0], &P_e[track][0][0], &Y_p[track][0], &P_p[track][0][0], Z[obs][0], Z[obs][1], R );
                    OLGI[track] = 0;
                }
                else {
                    OLGI[track]++;
                    if (OLGI[track] >= min_OLGI) {
                         obs_less_gate_ident (&OLGI[nn_track-1], track, nn_track-1, &Y_e[nn_track-1][0], &P_e[nn_track-1][0][0],
                              &Y_e[track][0], &P_e[track][0][0], &OLGI[track]);
                         nn_track--;
                    }
                }
             }
        }


        /*
            % New Target Identification
            clearing = ones(1,nn_NTI); % premenna na nulovanie NTI
            for obs = 1 : nn_obs
                if max(M(obs,:)) == 0 % podmienka pre mozny vznik noveho tracku
                    [NTI,new_track,clearing,last_obs] = ...
                         new_tg_ident (nn_NTI,min_NTI,NTI,last_obs,Z(:,obs,im),...
                            dif_d,dif_fi,nn_track,clearing,max_nn_tr); % overovanie
                            %dostatocnej blizkosti a poctu nepriradenych pozorovani
                    if new_track > 0
                        % inicializacia noveho tracku
                        [Y_e(:,:,new_track),P_e(:,:,new_track)] = init_estimation ...
                                     (Z(:,nn_obs,im),Y_e_2_init,Y_e_4_init,P_init);
                        nn_track = new_track;
                    end
                end
            end
        */
        Clearing[0] = Clearing[1] = Clearing[2] = 1;
        for( obs=0; obs<nn_obs; obs++ ) {
            if( find_max ( M, obs) == 0 ) {
                new_tg_ident ( nn_NTI, min_NTI, NTI, &last_obs[0][0], &last_obs[1][0],
                        Z[obs][0], Z[obs][1], dif_d, dif_fi, nn_track, Clearing, MAX_N,
                        &new_track, Clearing );
                if( new_track > 0 ) {
                   init_estimation (Z[nn_obs-1][0],Z[nn_obs-1][1] , Y_e_2_init, Y_e_4_init, P_init, &Y_e[new_track-1][0], &P_e[new_track-1][0][0]);
                   nn_track = new_track;
                }
            }
        }
        /*
            for nti = 1 : nn_NTI % nulovanie identifikatora a posledneho pozorovania
                if clearing(nti) == 1;
                    NTI(nti) = 0;
                    last_obs(:,nti) = [0;0];
                end
            end
        */
        for( k=0; k< nn_NTI; k++ ) {
            if (Clearing[k] == 1) {
                NTI[k] = 0;
                last_obs[0][k] = 0;
                last_obs[1][k] = 0;
            }
        }

        /*
            for track = 1 : nn_track % prepocitavanie polarnych suradnic na kartez.
                T(:,track,im) = polar2cartesian (Y_e(:,1,track));
            end
        */

        for( k=0; k< MAX_N; k++ ) {
                    T[0][k] = T[1][k] = 0.0;         /* clear previous results */
        }

        for( track=0; track<nn_track; track++ ) {
            polar2cartesian ( &Y_e[track][0], &T[0][track], &T[1][track] );
        }
        break;
    }
}

mtt_pure::word mtt_pure::find_max ( mtt_pure::word in[][MAX_N], mtt_pure::word row ) {
    mtt_pure::word k;
    mtt_pure::word val = INT_MIN;
    for (k=0; k<MAX_N; k++) {
         if( in[row][k] > val )
                  val =  in[row][k];
    }
    return val;
}

/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------- supporting functions -------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */

mtt_pure::real mtt_pure::mean_vector(mtt_pure::real *inptr, mtt_pure::word samples)
{
    word    i;
    real    mean_value = 0;

    for( i=0; i<samples; i++ ) {
         mean_value += inptr[i];
    }
    return mean_value/samples;
}

mtt_pure::real mtt_pure::std_vector(mtt_pure::real *inptr, mtt_pure::word samples)
{
    word    i;
    real    mean_value;
    real    std_value = 0;
    real    temp;

    mean_value = mean_vector( inptr, samples);          // mean value of the vector

    for( i=0; i<samples; i++ ) {
         temp = inptr[i] - mean_value;
         std_value += temp*temp;
    }

    std_value = (real) sqrt(std_value/(samples-1));    // as used in Matlab

    return std_value;
}

mtt_pure::real mtt_pure::max_vector(mtt_pure::real *inptr, mtt_pure::word samples)
{
    word  i;
    real  max_value = -FLT_MAX;

    for( i=0; i<samples; i++ ) {
           if (max_value < inptr[i])
                max_value = inptr[i];
    }
    return max_value;
}

mtt_pure::real mtt_pure::max_abs_vector(mtt_pure::real *inptr, mtt_pure::word samples)
{
    word    i;
    real    max_abs_value = 0;

    for( i=0; i<samples; i++ ) {
           if (max_abs_value < (real) fabs(inptr[i]))
                max_abs_value = (real) fabs(inptr[i]);
    }
    return max_abs_value;
}

void mtt_pure::clear_vector(mtt_pure::real *inptr, mtt_pure::word samples)
{
    word    i;

    for( i=0; i<samples; i++ ) {
        inptr[i] = 0;
    }
}

void mtt_pure::clear_int_vector(mtt_pure::word *inptr, mtt_pure::word samples)
{
    word    i;

    for( i=0; i<samples; i++ ) {
        inptr[i] = 0;
    }
}

void mtt_pure::abs_vector(mtt_pure::real *inptr, mtt_pure::word samples)
{
    word    i;

    for( i=0; i<samples; i++ ) {
        inptr[i] = (real) fabs( inptr[i] );
    }
}

mtt_pure::real mtt_pure::sum_vector(mtt_pure::real *inptr, mtt_pure::word samples)
{
    word    i;
    real    sum = 0;

    for( i=0; i<samples; i++ ) {
        sum += inptr[i];
    }
    return sum;
}

mtt_pure::word mtt_pure::round2(mtt_pure::real in)
{
    word out;

    if (in >= 0)
            out = (word) (in + 0.5);
    else
            out = (word) (in - 0.5);
    return  out;
}

mtt_pure::word mtt_pure::max_of_2(mtt_pure::word a, mtt_pure::word b)
{
    if (a > b)
          return a;
    return b;
}

mtt_pure::word mtt_pure::find_vec(mtt_pure::word in[], mtt_pure::word n)
{
    word i;
    for( i=0; i<n; i++ )        // checks complete vector
         if (in[i])
              return i;
    return EMPTY;
}

void mtt_pure::matrix_mul_4x2(mtt_pure::real c[][2], mtt_pure::real a[][2], mtt_pure::real b[][2])
{
    word k,j;

    for( k=0; k<4; k++ ) {
       for( j=0; j<2; j++ ) {
            c[k][j] = (real) a[k][0]*b[0][j] + a[k][1]*b[1][j];
       }
    }
}

void mtt_pure::matrix_mul_4x4(mtt_pure::real c[][4], mtt_pure::real a[][4], mtt_pure::real b[][4])
{
    word k,j;

    for( k=0; k<4; k++ ) {
       for( j=0; j<4; j++ ) {
                      c[k][j] = a[k][0]*b[0][j] + a[k][1]*b[1][j] + a[k][2]*b[2][j] + a[k][3]*b[3][j];
       }
    }
}

void mtt_pure::matrix_transpose_4x4(mtt_pure::real out[][4], mtt_pure::real in[][4])
{
    word k,j;

    for( k=0; k<4; k++ ) {
       for( j=0; j<4; j++ ) {
            out[j][k] = in[k][j];
       }
    }
}

void mtt_pure::matrix_add_4x4(mtt_pure::real c[][4], mtt_pure::real a[][4], mtt_pure::real b[][4])
{
    word k,j;

    for( k=0; k<4; k++ ) {
       for( j=0; j<4; j++ ) {
            c[k][j] = a[k][j] + b[k][j];
       }
    }
}

void mtt_pure::cartesian2polar(mtt_pure::real x, mtt_pure::real y, mtt_pure::real *r, mtt_pure::real *fi)
{
    *r = 0;
    *fi = 0;

    *r = (real) sqrt(x*x + y*y);
    if (*r == 0.0)
         return;

    if (*r == 0.0) {
         return;
    }
    else {
         if( y > 0)
            *fi = (real) acos(x / *r);
         else
            *fi = (real) -1.0 * (real) acos(x / *r);
    }
}

void mtt_pure::connected_covering(mtt_pure::word value_ch_b, mtt_pure::word ch, mtt_pure::word cover_b, mtt_pure::word w, mtt_pure::word trace_con[], mtt_pure::word c[])
{
    UNUSED(value_ch_b);
    UNUSED(ch);
    UNUSED(cover_b);
    UNUSED(w);
    UNUSED(trace_con);
    UNUSED(c);
    // ??????
}

void mtt_pure::correction(mtt_pure::real Y_e[], mtt_pure::real *P_e, mtt_pure::real Y_p[], mtt_pure::real *P_p, mtt_pure::real r, mtt_pure::real fi, mtt_pure::real R[][2])
{
    word k,j;
    real _P_e[4][4], _P_p[4][4];

    word t;

    real K[4][2];   // Kalman gain matrix
    real A[2][2];
    real A_inv[2][2];
    real P_p_H[4][2];
    real H_Y_p[2];
    real EYE[4][4];
    real tmp;

    /* copy row vectors (parts of 3D arrays) to the 4x4 matrices */
    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
           _P_e[k][j] = P_e[4*k+j];
           _P_p[k][j] = P_p[4*k+j];
        }
    }

    /*
    Y_e = zeros(4,1);
    P_e = zeros(4,4);
    */
    for( t=0; t<4; t++ ) {
        Y_e[t] = 0.0;
        for( j=0; j<4; j++ ) {
           _P_e[t][j] = 0.0;
           EYE[t][j] = 0.0;
        }
    }
    for( t=0; t<4; t++ )
       EYE[t][t] = 1.0;

    /*
    K = zeros(4,2); % Kalman gain matrix

    */
    for( t=0; t<4; t++ ) {
        Y_e[t] = 0.0;
        for( j=0; j<2; j++ ) {
           K[t][j] = 0.0;
        }
    }

    /*
    A = H*P_p*H' + R; % matica 2x2
    */
    A[0][0] = _P_p[0][0] + R[0][0];
    A[0][1] = _P_p[0][2] + R[0][1];
    A[1][0] = _P_p[2][0] + R[1][0];
    A[1][1] = _P_p[2][2] + R[1][1];

    /*
    A_inv = [A(2,2) -A(1,2);-A(2,1) A(1,1)]/(A(1,1)*A(2,2)- A(1,2)*A(2,1));
    */
    tmp = A[0][0]*A[1][1] - A[0][1]*A[1][0];
    A_inv[0][0] = A[1][1]/tmp;
    A_inv[0][1] = -A[0][1]/tmp;
    A_inv[1][0] = -A[1][0]/tmp;
    A_inv[1][1] = A[0][0]/tmp;

    /*
    K = P_p*H'*A_inv;
    */
    P_p_H[0][0] = _P_p[0][0];
    P_p_H[1][0] = _P_p[1][0];
    P_p_H[2][0] = _P_p[2][0];
    P_p_H[3][0] = _P_p[3][0];
    P_p_H[0][1] = _P_p[0][2];
    P_p_H[1][1] = _P_p[1][2];
    P_p_H[2][1] = _P_p[2][2];
    P_p_H[3][1] = _P_p[3][2];

    matrix_mul_4x2(K,  P_p_H, A_inv);
    /*
    Y_e = Y_p + K*(Z - H*Y_p);
    */
    H_Y_p[0] = Y_p[0];
    H_Y_p[1] = Y_p[2];
    H_Y_p[0] = r - H_Y_p[0];
    H_Y_p[1] = fi - H_Y_p[1];

    Y_e[0] = Y_p[0] + K[0][0]*H_Y_p[0] + K[0][1]*H_Y_p[1];
    Y_e[1] = Y_p[1] + K[1][0]*H_Y_p[0] + K[1][1]*H_Y_p[1];
    Y_e[2] = Y_p[2] + K[2][0]*H_Y_p[0] + K[2][1]*H_Y_p[1];
    Y_e[3] = Y_p[3] + K[3][0]*H_Y_p[0] + K[3][1]*H_Y_p[1];

    /*
    P_e = (eye(4)-K*H)*P_p;
    */
    EYE[0][0] -= K[0][0];
    EYE[1][0] -= K[1][0];
    EYE[2][0] -= K[2][0];
    EYE[3][0] -= K[3][0];

    EYE[0][2] -= K[0][1];
    EYE[1][2] -= K[1][1];
    EYE[2][2] -= K[2][1];
    EYE[3][2] -= K[3][1];

    matrix_mul_4x4( _P_e, EYE, _P_p);

    /* copy results back to the row vector (part of 3D array) */
    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            P_p[4*k+j] = _P_p[k][j];
            P_e[4*k+j] = _P_e[k][j];
        }
    }

}

void mtt_pure::prediction(mtt_pure::real Y_e_p[], mtt_pure::real *P_e_p, mtt_pure::real Q[][4], mtt_pure::real Y_p[], mtt_pure::real *P_p)
{
    real f, T;
    real A[4][4];
    real TMP[4][4], A_TRAN[4][4];

    word k, j;
    real _P_e_p[4][4], _P_p[4][4];

    /* copy row vectors (parts of 3D arrays) to the 4x4 matrices */

    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            _P_e_p[k][j] = P_e_p[4*k+j];
            _P_p[k][j] = P_p[4*k+j];
        }
    }

    f = (real) FREQUENCY;
    T = SCANLENGHT/f;
    T *= uS*HA*SA;


    /*
    A = [1 T 0 0;
         0 1 0 0;
         0 0 1 T;
         0 0 0 1]; % state transition matrix

    Y_p = A*Y_e_p;
    */

    Y_p[0] = Y_e_p[0] + T* Y_e_p[1];
    Y_p[1] = Y_e_p[1];
    Y_p[2] = Y_e_p[2] + T* Y_e_p[3];
    Y_p[3] = Y_e_p[3];

    /*
    P_p = A*P_e_p*A' + Q;
    */
    A[0][2] = A[0][3] = 0.0;
    A[1][0] = A[1][2] = A[1][3] = 0.0;
    A[2][0] = A[2][1] = 0.0;
    A[3][0] = A[3][1] = A[3][2] = 0.0;
    A[0][0] = A[1][1] = A[2][2] = A[3][3] = 1.0;
    A[0][1] = A[2][3] = T;

    matrix_mul_4x4( TMP, A, _P_e_p );
    matrix_transpose_4x4( A_TRAN, A );
    matrix_mul_4x4( _P_p, TMP, A_TRAN );
    matrix_add_4x4( _P_p, _P_p, Q );

    /* copy results back to the row vector (part of 3D array) */
    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            P_p[4*k+j] = _P_p[k][j];
        }
    }

}

void mtt_pure::covering(mtt_pure::word value_ch_b, mtt_pure::word *ch, mtt_pure::word m, mtt_pure::word trace_con[], mtt_pure::word c[], mtt_pure::word *cover_b)
{
    word chip;
    word cover_b_found;
    //word i; // seems to be unsued

    c[0] = c[1] = c[2] = 0;
    cover_b_found = 0;
    chip = *ch +1;
    while( (cover_b_found == 0) && (chip <= *ch+2*m) ) {
       if (trace_con[ chip ] == 3) {
               *cover_b = chip;
               cover_b_found = 1;
       }
       else {
               chip += 1;
       }
    }
    c[0] = (*cover_b) + ((*ch+2*m-(*cover_b)+1)/2);
    c[1] = *ch+2*m-(*cover_b)+1;
    c[2] = value_ch_b;
    *ch = (*cover_b) + 2*m + 1;
}

void mtt_pure::different_values(mtt_pure::word trace_con[], mtt_pure::word *ch, mtt_pure::word m, mtt_pure::word value_ch_b, mtt_pure::word value_ch_e, mtt_pure::word nn_ch, mtt_pure::word c[])
{
    word pos_three[SCANLENGHT];
    word nn_three, k, pos;

    c[0] = c[1] = c[2] = 0;
    /*
    pos_three = find(trace_con(ch:ch+2*m)==3);
    nn_three = length(pos_three);
    */

    nn_three = 0;
    for( k=*ch; k<= (*ch+2*m); k++ ) {
       if( trace_con[k] == 3 ) {
            pos_three[ nn_three ] = k - *ch;
            nn_three++;
       }
    }

    if (nn_three == 0) {
    /*
    if nn_three == 0 % bez prekrytia
        ch = ch - 1 + find(trace_con(ch:ch+2*m)~=value_ch_b,1);
    */
    pos = -1;
    for( k=*ch; k<= *ch+2*m; k++ ) {
        if( (trace_con[k] != value_ch_b) && (pos==-1) ) {
           pos = k;
        }
    }
    *ch = pos;                // vector is searched in absolute positions!
    /*
                     if (pos < 0 )
                 *ch = pos;            // Empty matrix in Matlab
             else
                 *ch = *ch + pos;      // -1 removed!!!
    */
    }
    else {
    /*
        c(1) = floor((pos_three(1)+pos_three(end))/2) + ch - 1;
        c(2) = nn_three;
        if value_ch_e == 0
            c(3) = 1;
        else
            c(3) = 3 - value_ch_e;
        end

        ch = floor(c(1) + nn_three/2);
        while trace_con(ch) ~= 0 & ch + 1 < nn_ch
            ch = ch + 1;
        end
    */
         c[0] = (pos_three[0]+pos_three[nn_three-1])/2 + *ch;   // -1 removed
         c[1] = nn_three;
         if (value_ch_e == 0)
              c[2] = 1;
         else
              c[2] = 3 - value_ch_e;
         *ch = ( c[0] + nn_three/2 );
         while( (trace_con[*ch] != 0) & ((*ch+2) < nn_ch) )
               *ch += 1;

    }
}

void mtt_pure::gate_checker(mtt_pure::real r, mtt_pure::real fi, mtt_pure::real Y_p[], mtt_pure::real R[][2], mtt_pure::real *P_p, mtt_pure::word *m, mtt_pure::real *c)
{
    word  k, j;
    real  c0, c1, res_d, res_fi, temp;
    real sigma_d, sigma_fi;
    real _P_p[4][4];

    /* copy row vector (part of 3D arrays) to the 4x4 matrix */

    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            _P_p[k][j] = P_p[4*k+j];
        }
    }

    /*
    k = 3; % from "3 sigma rule"
    m = 0;
    c = 10^2; % prednastavena lubovolne velka hodnota
    */

    k = 3;
    *m = 0;
    *c = LARGE_NUMBER;

    /*
    res_d = abs(Z(1) - Y_p(1,1));
    res_fi = abs(Z(2) - Y_p(3,1));
    */
    res_d = (real) fabs( r - Y_p[0] );
    res_fi = (real) fabs( fi - Y_p[2] );

    /*
    sigma_d = sqrt(R(1,1) + P_p(2,2));
    sigma_fi = sqrt(R(2,2) + P_p(4,4));
    */
    sigma_d = (real) sqrt( R[0][0] + _P_p[1][1] );
    sigma_fi = (real) sqrt( R[1][1] + _P_p[3][3] );


    /*
    if res_d <= k*sigma_d && res_fi <= k*sigma_fi
        m = 1;
        c = ([res_d res_fi]*[P_p(3,3)+R(2,2) -P_p(1,3); -P_p(3,1) P_p(1,1)+R(1,1)]*[res_d res_fi]')...
            /((P_p(1,1)+R(1,1))*(P_p(3,3)+R(2,2))-P_p(1,1)*P_p(3,3));
    end
    */
    if( (res_d <= k*sigma_d) && (res_fi <= k*sigma_fi) ) {
         *m = 1;
         temp = (_P_p[0][0]+R[0][0])*(_P_p[2][2]+R[1][1])-_P_p[0][0]*_P_p[2][2];
                 if ( fabs(temp) > FLT_MIN ) {                             // MD addiional protection
             c0 = res_d*( _P_p[2][2]+R[1][1]) - res_fi*_P_p[2][0];
             c1 = res_fi*( _P_p[0][0]+R[0][0]) - res_d* _P_p[0][2];
             *c = (c0*res_d + c1*res_fi)/temp;
         }
    }
}

void mtt_pure::identical_values(mtt_pure::word value_ch_b, mtt_pure::word *ch, mtt_pure::word m, mtt_pure::word max_nn_tg, mtt_pure::word center_previous[][3], mtt_pure::word c[3])
{
    UNUSED(max_nn_tg);
    UNUSED(center_previous);

    word obs_found, obs = 1; // seems to be unused

    UNUSED(obs_found);
    UNUSED(obs);


    c[0] = c[1] = c[2] = 0;

    if (value_ch_b == 3) {
       c[0] = *ch + m;
       c[1] = 2*m+1;
       c[2] = 0;
    }
    /*else {   !!!osamotene TOA, vypnute
       obs_found = 0;
       obs = 0;
       while ( center_previous[obs][0] > 0 && (obs_found == 0) ) {
            if ( (center_previous[obs][0] >= *ch) && (center_previous[obs][0] <= *ch+2*m)  ) {
               c[0] = center_previous[obs][0];
               c[1] = center_previous[obs][1];
               c[2] = center_previous[obs][2];
               obs_found = 1;
            }
            else {
               if ( obs+1 <= max_nn_tg )
                    obs += 1;
               else
                    obs_found = 1;
            }
       }

    }*/
    *ch = *ch + 2*m + 1;
}

void mtt_pure::init_estimation(mtt_pure::real r, mtt_pure::real fi, mtt_pure::real Y_e_2_init, mtt_pure::real Y_e_4_init, mtt_pure::real P_init[][4], mtt_pure::real Y_e[], mtt_pure::real *P_e)
{
    word k, j;

    Y_e[0] = r;
    Y_e[1] =  Y_e_2_init;
    Y_e[2] = fi;
    Y_e[3] = Y_e_4_init;

    for( k=0; k<4; k++ )
        for( j=0; j<4; j++ )
                P_e[4*k+j] = P_init[k][j];
}

void mtt_pure::t_integration(mtt_pure::word *inptr, mtt_pure::word t_size, mtt_pure::word min_int, mtt_pure::word nn_ch)
{
    word integ[ SCANLENGHT ];
    word   i, sum;


    sum = 0;
    for( i=0; i<t_size; i++ ) {      // "growing" window at the beginning of the vector
            sum += inptr[i];
                            integ[i] = sum;
    }
            integ[0] = 0;                    // Matlab code starts from integ[1] ...

    for( i=t_size; i<nn_ch; i++ ) {  // "sliding" window up to the end of vector
            sum += inptr[i] - inptr[i-t_size];
                            integ[i] = sum;
    }

    for( i=0; i<nn_ch; i++ ) {
            inptr[ i ] = integ[ i ] >=  min_int;
    }
}

void mtt_pure::intersection2ellipses(mtt_pure::real d0, mtt_pure::real d1, mtt_pure::real x1, mtt_pure::real x2, mtt_pure::real *x, mtt_pure::real *y)
{
    real k1, k2, tmp;

    k1 = (real) 0.5*(d0*d0-x1*x1);     // auxiliary variable
    k2 = (real) 0.5*(d1*d1-x2*x2);     // auxiliary variable
    *x = -(k1*d1-k2*d0)/(x1*d1 - x2*d0);

    tmp = (x1*(*x)+k1)/d0;
    tmp =  tmp*tmp-(*x)*(*x);
    if ( tmp > 0 ) {
         *y = (real) sqrt( tmp );
    }
    else {
         tmp = (x2*(*x) + k2)/d1;
         tmp = tmp*tmp - (*x)*(*x);
         if ( tmp > 0 )
               *y = (real) sqrt( tmp );
         else {
               *x = 0;
               *y = 0;
         }
    }
    if (fabs( *y ) < MIN_Y_COORDINATE ) { //  added by MD in order to remove ill conditioned computations
        *x = 0;
        *y = 0;
    }
}

float *mtt_pure::trace_connection(int *inptr1, int *inptr2, int *TC, int t_size, int min_int, int m, int max_nn_tg, float *TOA_mem)
{
    int k;
    for( k=0; k<SCANLENGHT; k++ ){
        out_det[0][k]=inptr1[k];
        out_det[1][k]=inptr2[k];
    }
    trace_connection2(out_det, t_size, min_int, m, max_nn_tg, center_previous,TC,TOA_m);
    for(k=0;k<MAX_N;k++){
        TOA_mem[2*k]=TOA_m[0][k];
        TOA_mem[2*k+1]=TOA_m[1][k];
    }

    return TOA_mem;
}

void mtt_pure::trace_connection2(mtt_pure::word out_det[][SCANLENGHT], mtt_pure::word t_size, mtt_pure::word min_int, mtt_pure::word m, mtt_pure::word max_nn_tg, mtt_pure::word center_previous[][3], mtt_pure::word *TC, mtt_pure::real TOA_m[][MAX_N])
{
    UNUSED(max_nn_tg);

    /*
    %-------------------------------------------------------------------------%
    % INICIALIZACIA
    %-------------------------------------------------------------------------%
    [nn_ch, nn_im, nn_rx] = size(out_det);
    % od tejto etapy pocet impulzovych odpovedi je poslednym rozmerom
    TOA_m = zeros(nn_rx,max_nn_tg,nn_im);
    TC = zeros(nn_ch,nn_im);
    center_previous = zeros (max_nn_tg,3);
    c2 =  zeros(max_nn_tg,nn_im);
    */
    word k, j;
    word nn_obs;
    word value_ch_b, value_ch_e;
    word cover_b;
    word c2[MAX_N]; UNUSED(c2); // seems to be unused
    word center[MAX_N][3];
    word first_reflection[2][SCANLENGHT];
    word IR[SCANLENGHT];
    word trace_con[SCANLENGHT];
    word c[3];
    word ch;
    word pos;
    /*FR = zeros(nn_rx,max_nn_tg,nn_im);*/
    for( k=0; k<MAX_N; k++ ) {
       TOA_m[0][k] = TOA_m[1][k] = 0.0;
       c2[k] = 0;
    }
    for( k=0; k<SCANLENGHT; k++ ){
        TC[k] = 0;
    }

    /*
    %-------------------------------------------------------------------------%
    % HLAVNY PROGRAM
    %-------------------------------------------------------------------------%
    for im = 1 : nn_im % cyklus cez vsetky impulzove odovede (IR)
        center = zeros (max_nn_tg,3); % vyznam stlpcov: stred prekrytia, sirka
                                      % prekrytia, umiestnenie TOA z Rx1
        first_reflection = zeros (nn_ch,nn_rx);
    */
    for( k=0; k<MAX_N; k++ )
        center[k][0] = center[k][1] = center[k][2] = 0;

    /*  not necessary (see clearing inside point_targets)
    for( k=0; k<SCANLENGHT; k++ ) {
        first_reflection[0][k] = 0;
        first_reflection[1][k] = 0;
    }
    */


    /*
    %-------------------------------------------------------------------------%
    % integracia IR a vytvorenie bodovych cielov prebehne zvlast pre oba Rx
    %-------------------------------------------------------------------------%
        for rx = 1 : nn_rx % cyklus cez oba prijimace

            IR = out_det(:,im,rx); % nacitanie impulzovej odpovede po detekcii

            int_IR = integration(nn_ch,IR,t_size,min_int); % vystupom
            % integrovania su zhluky jednotiek na tych chipovych poziciach, kde
            % integrovanie dosiahlo hodnoty vyssie ako "min_int", stale je to
            % vektor dlzky 511 chipov

            first_reflection(:,rx) = point_targets(nn_ch,int_IR,min_int,m,rx);
            % rozlozene ("distributed") ciele sa nahradia bodovymi cielmi, ich
            % chipova pozicia zodpoveda prvemu odrazu od rozlozeneho ciela a je
            % oznacena "1"-kami v pripade Rx1, "2"-kami pre Rx2; ich sirka je
            % umelo rozsirena o "m" chipov nadol a nahor tak, aby sa pretli tie
            % dvojice TOA, ktore zodpovedaju cielom a nie "duchom"

        end % koniec cyklu cez oba prijimace
    */
    for(k=0; k<2; k++ ) {
        for( j=0; j<SCANLENGHT; j++ ) {
            IR[j] = out_det[k][j];
        }
        t_integration( IR, t_size, min_int, SCANLENGHT );
        point_targets( SCANLENGHT, IR, min_int, m, k, &first_reflection[k][0] );

    }

    /*
    %-------------------------------------------------------------------------%
    % vystupy z oboch Rx sa spoja do matice "trace_con", ktora obsahuje 3 druhy
    % hodnot: "1" predstavuju prve odrazy od cielov ziskane z Rx1, "2" z Rx2 a
    % "3" indikuju dvojice TOA, ktore umoznia vypocitat poziciu ciela - pre ne
    % je potrebne najst stred prekrytia, sirku prekrytia a umiestnenie TOA z
    % Rx1, co je ciel nasledujucich 4 funkcii
    %-------------------------------------------------------------------------%
        trace_con = first_reflection(:,1) + first_reflection(:,2);
        TC(:,im) = trace_con; % prvy z vystupov funkcie "trace_connection",
        % sluzi na vykreslenie spojenych stop, v prototype ho mozu vyuzit ako
        % vstup pre vytvaranie radarovych obrazov, avsak kvoli sirokym stopam
        % ciel bude znacne velky

    */
    for( j=0; j<SCANLENGHT; j++ ) {
        trace_con[j] = first_reflection[0][j] + first_reflection[1][j];
        TC[j] = trace_con[j];
    }

    /*
        nn_obs = 0; % pocet najdenych prekryti
        ch = find(trace_con > 0,1); % prva kladna chipova hodnota
    */
    nn_obs = 0;
    ch = find_vec( trace_con, SCANLENGHT );       // -1 if "empty"

    /*
        while ch < nn_ch - 2*m  % prehladavanie celej impulzovej odpovede
    */
        /*doplnit if*/
    /*********************************/
        /*if isempty(ch)==0*/
    //	if(ch!=-1){
    while( (ch < (SCANLENGHT-2*m))&&(ch>=0)  ) {   // ch < 0 if there is no nonzero element

    // MD
    //      fprintf(stdout, "\now %d", ch );
    /*
    if trace_con(ch) > 0 % hladanie kladnej hodnoty
    */
    if( trace_con[ch] > 0 ) {
    /*
    %-------------------------------------------------------------------------%
    % prvy odraz od ciela zabera po umelom rozsireni 2*m+1 chipov; ak sa
    % pozriem na pociatocnu a koncovu hodnotu najdeneho odrazu, viem rozlisit,
    % ktory z 3 moznych pripadov nastal
    %-------------------------------------------------------------------------%
    value_ch_b = trace_con(ch);
    value_ch_e = trace_con(ch+2*m);
    */
    value_ch_b = trace_con[ch];
    value_ch_e = trace_con[ch+2*m];
    /*
    if value_ch_b == value_ch_e
    */
    if( value_ch_b == value_ch_e ) {

    /*
    %-------------------------------------------------------------------------%
    % 1.pripad: rovnake hodnoty naznacuju, ze bud mam uplne prekrytie odrazov z
    % oboch Rx alebo mam osamoteny odraz, vtedy kontrolujem, ci v
    % predchadzajucej IR nebol v jeho blizkosti niektory z najdenych stredov
    % prekryti
    %-------------------------------------------------------------------------%
    [c,ch] = identical_values ...
                   (value_ch_b,ch,m,max_nn_tg,center_previous);
    if c ~= 0
        if nn_obs + 1 <= max_nn_tg
            nn_obs = nn_obs + 1;
            center(nn_obs,:) = c;
        else
            ch = nn_ch;
        end
    end
    */

    identical_values (value_ch_b, &ch, m, MAX_N, center_previous, c);
    /*zmena, v ife len 1. hodnota*/
    if  (c[0] != 0) {
        if ( (nn_obs+1) <= MAX_N  ) {
           center[nn_obs][0] = c[0];
           center[nn_obs][1] = c[1];
           center[nn_obs][2] = c[2];
           nn_obs++;
        }
        else
           ch = SCANLENGHT;
    }
    /*
    elseif value_ch_e == 3
    */
    }
    else {
      if ((value_ch_e == 3)) {
    /*
    %-------------------------------------------------------------------------%
    % 2.pripad: prekrytie odrazov, je potrebne skontrolovat, ci nedoslo k
    % spojenym prekrytiam
    %-------------------------------------------------------------------------%
    if nn_obs + 1 <= max_nn_tg
        nn_obs = nn_obs + 1;
        [center(nn_obs,:),ch,cover_b] = covering ...
                                   (value_ch_b,ch,m,trace_con);
    else
        ch = nn_ch;
    end
    */
                      /*doplnit dalsie zmeny*/
    /*************************************************************/
    if (nn_obs +1 <= MAX_N) {
       covering (value_ch_b, &ch, m, trace_con, &center[nn_obs][0], &cover_b);
                       nn_obs++;
    }
    else
       ch = SCANLENGHT;


    /*
    if ch < nn_ch % pridane!!!
        if trace_con(ch-1) == 3 % spojene prekrytia
            if nn_obs + 1 <= max_nn_tg
                nn_obs = nn_obs + 1;
                center(nn_obs,:) = connected_covering ...
                                     (value_ch_b,ch,cover_b,...
                                 center(nn_obs-1,2),trace_con);
            else
                ch = nn_ch;
            end
        end
    end
    */
      if( ch < SCANLENGHT ) {
           if( trace_con[ch] > 0 ) {
              if ( nn_obs +1 <=MAX_N ) {
                // connected_covering (value_ch_b, ch, cover_b, center[nn_obs-1][1], trace_con, &center[nn_obs][0]);
                 /*zmenit za connected_reflections*/
                 connected_reflections(&ch,trace_con,&center[nn_obs][0],m,&nn_obs);
                 nn_obs++;
              }
              else
                 ch = SCANLENGHT;
          }
      }

    }
    /*
    else
    */
    else {
    /*
    %-------------------------------------------------------------------------%
    % 3.pripad: nerovnake hodnoty nastanu pri nestandardne sirokych
    % odrazoch (vzniknu napr. spojenim viacerych rovnakych odrazov), moze ale
    % nemusi vzniknut prekrytie
    %-------------------------------------------------------------------------%
        [c,ch] = different_values ...
                      (trace_con,ch,m,value_ch_b,value_ch_e,nn_ch);
        if c ~= 0
            if nn_obs + 1 <= max_nn_tg
                nn_obs = nn_obs + 1;
                center(nn_obs,:) = c;
            else
                ch = nn_ch;
            end
        end

    end % koniec cyklu pre porovnavanie pociatocnej a koncovej
        % hodnoty najdeneho odrazu
    */
        different_values( trace_con, &ch, m, value_ch_b, value_ch_e, SCANLENGHT, c );
        /*zmena v ife*/
        if  (c[0] != 0){
            if ( (nn_obs+1) <= MAX_N  ) {
               center[nn_obs][0] = c[0];
               center[nn_obs][1] = c[1];
               center[nn_obs][2] = c[2];
               nn_obs++;
            }
            else
               ch = SCANLENGHT;
        }
    }
    /*
            else
    */
        }

    }
    else  {
    /*
    %-------------------------------------------------------------------------%
    % ak na danej chipovej hodnote nezacina ziaden odraz, posuniem sa na
    % nasledujucu kladnu chipovu hodnotu; prikazu find sa da vyhnut jednoduchym
    % zvysovanim chipovej hodnoty o 1
    %-------------------------------------------------------------------------%
                ch = ch + find(trace_con(ch+1:end) > 0,1);
    */
    pos = find_vec( &trace_con[ch+1], SCANLENGHT - ch -1 );
    if (pos < 0)
         ch = pos;              // if "empty" vector
    else
         ch += 1+pos;

    /*
       end % koniec "if" cyklu
    */
              }

    /*
        end % koniec "while" cyklu
    */
           }
            /*tu ma koncit doplneny if*/
    /***************************************************************/

          // }

    /*
    %-------------------------------------------------------------------------%
    % na zaklade hodnot ulozenych v matici "center" viem spatne vypocitat TOA
    % z Rx1 a TOA z Rx2, ktore patria k sebe; casy sa prepocitaju na
    % vzdialenosti pomocou frekvencie radara a rychlosti svetla
    %-------------------------------------------------------------------------%
        TOA_m(:,:,im) = TOA_couple (max_nn_tg,center,f,m);

        center_previous = center; % "center" z predchadzajucej IR sa vyuziva vo
    */

    for( k=0; k<MAX_N; k++ ) {
          center_previous[k][0] = center[k][0];
          center_previous[k][1] = center[k][1];
          center_previous[k][2] = center[k][2];
    }
    TOA_couple (MAX_N, center, m, TOA_m);


    /*
                                  % funkcii "identical_values"
    %     c2(:,im) = center(:,1); % pomocna premenna na vykreslenie stredov
                  % prekryti, v prototype nebude, sluzi na kontrolu vystupov!!!

    end
    */
}

void mtt_pure::new_tg_ident(mtt_pure::word nn_NTI, mtt_pure::word min_NTI, mtt_pure::word NTI[], mtt_pure::real r_last_obs[], mtt_pure::real fi_last_obs[], mtt_pure::real r, mtt_pure::real fi, mtt_pure::real dif_d, mtt_pure::real dif_fi, mtt_pure::word nn_track, mtt_pure::word cl[], mtt_pure::word max_nn_tr, mtt_pure::word *new_track, mtt_pure::word clearing[])
{
    word check, empty_nti;
    word k, nti_;
    real dif1, dif2;

    check = 0;
    empty_nti = -1;
    for( k=0; k<nn_NTI; k++ )
        clearing[k] = cl[k];
    *new_track = 0;

    nti_ = 0;


    while ( (nti_ < nn_NTI)  && (check == 0) ) {

        if ( NTI[nti_] > 0 ) {
            dif1 = (real) fabs( r_last_obs[nti_] - r );
            dif2 = (real) fabs( fi_last_obs[nti_] - fi );
            if( (dif1 <= dif_d) && (dif2 <= dif_fi) ) {
                NTI[nti_] = NTI[nti_] + 1;
                check = 1;
                if (NTI[nti_] >= min_NTI) {

                    if ( (nn_track+1) <= max_nn_tr )
                        *new_track = nn_track+1;
                    else
                        *new_track = max_nn_tr;

                    clearing[nti_] = 1;
                }
                else
                    clearing[nti_] = 0;
            }
        }
        else
            empty_nti = nti_;

        nti_ = nti_ + 1;
    }
    /*
    if check == 0 && empty_nti > 0
        NTI(empty_nti) = 1;
        last_obs(:,empty_nti) = Z;
        clearing(empty_nti) = 0;
    end
    */
    if ( (check == 0) && (empty_nti >= 0) ) {  // indexing from 0 in C
        NTI[empty_nti] = 1;
        r_last_obs[empty_nti] = r;
        fi_last_obs[empty_nti] = fi;
        clearing[empty_nti] = 0;
    }
}

void mtt_pure::obs_less_gate_ident(mtt_pure::word *OLGI_last, mtt_pure::word track, mtt_pure::word nn_track, mtt_pure::real Y_e_last[], mtt_pure::real *P_e_last, mtt_pure::real Y_e[], mtt_pure::real *P_e, mtt_pure::word *OLGI)
{
    real _P_e_last[4][4], _P_e[4][4];
    word k, j;

    /* copy row vector (part of 3D arrays) to the 4x4 matrix */
    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
              _P_e_last[k][j] = P_e_last[4*k+j];
        }
    }

    for (k=0; k<4; k++ ) {
         Y_e[k] = 0.0;
         for ( j=0; j<4; j++ ) {
             _P_e[k][j] = 0.0;
         }
    }

    *OLGI = 0;

    if (track != nn_track ) {
         for (k=0; k<4; k++ ) {
             Y_e[k] = Y_e_last[k];
             for ( j=0; j<4; j++ ) {
                 _P_e[k][j] = _P_e_last[k][j];
             }
         }
         *OLGI = *OLGI_last;
    }

    for (k=0; k<4; k++ ) {
         Y_e_last[k] = 0.0;
         for ( j=0; j<4; j++ ) {
             _P_e_last[k][j] = 0.0;
         }
    }
    *OLGI_last = 0;

    /* copy results back to the row vector (part of 3D array) */
    for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
            P_e[4*k+j] = _P_e[k][j];
            P_e_last[4*k+j] = _P_e_last[k][j];
        }
    }
}

void mtt_pure::polar2cartesian(mtt_pure::real Y_e[], mtt_pure::real *x, mtt_pure::real *y)
{
    *x = *y = 0.0;

    *x = Y_e[0]* (real)cos(Y_e[2]);
    *y = Y_e[0]* (real)sin(Y_e[2]);
}

void mtt_pure::point_targets(mtt_pure::word nn_ch, mtt_pure::word int_IR[], mtt_pure::word min_int, mtt_pure::word m, mtt_pure::word rx, mtt_pure::word first_reflection[])
{
    word dif_int_IR[ SCANLENGHT ];
    word pos_ones[ SCANLENGHT ];
    word k, j, pp, ch_pos, f_ch, l_ch;

    /*
    first_reflection = zeros (nn_ch,1);
    dif_int_IR = zeros (nn_ch,1);
    */
    clear_int_vector( first_reflection, nn_ch );
    clear_int_vector( dif_int_IR, nn_ch );
    /*
    dif_int_IR(2:end) = int_IR(2:end) - int_IR(1:end-1);
    */
    for( k=1; k<nn_ch; k++ ) {
         dif_int_IR[k] = int_IR[k] - int_IR[k-1];
    }
    /*
    pos_ones = find(dif_int_IR==1);
    */
    pp = 0;
    for( k=1; k<nn_ch; k++ ) {
         if (dif_int_IR[k] == 1) {
             pos_ones[ pp ] = k;
             pp++;
         }
    }
    if (pp > 0 ) {
        for( k=0; k<pp; k++ ) {
             ch_pos = pos_ones[k] - min_int;

             f_ch = 0;
             while( ch_pos-m+f_ch < 0 )
                   f_ch += 1;

             l_ch = 0;
             while( ch_pos+m-l_ch > nn_ch )       // strange, but I copy Matlab code ...
                   l_ch += 1;

             for( j=ch_pos-m+f_ch; j<=ch_pos+m-l_ch; j++ )
                   first_reflection[j] = (rx+1);  // Matlab code uses values 1 and 2!!!
        }
    }
}

float *mtt_pure::MT_localization(float *TOA_mem, float x1, float x2)
{
    word obs, obs_full, k, j;
    float TOA_m[2][MAX_N];
    float P[2][MAX_N];


    for( k=0; k<2; k++ ) {
        for( j=0; j<MAX_N; j++ ){
             P[k][j] = 0.0;
        }
    }
    /*
     * Transformacia jednorozmerneho vstupu na dvojrozmerne pole
     */
    for(k=0;k<MAX_N;k++){
        TOA_m[0][k]=TOA_mem[2*k];
        TOA_m[1][k]=TOA_mem[2*k+1];
    }
    obs = 0;
    obs_full = 0;
    //
    while( ((TOA_m[0][obs]!=0) || (TOA_m[1][obs]!=0)) && (obs_full == 0) ) {
        intersection2ellipses( TOA_m[0][obs],TOA_m[1][obs],x1,x2,&P[0][obs],&P[1][obs] );
        if( obs < (MAX_N-1) )
             obs++;
        else
             obs_full = 1;
    }
    /*
     *Transformacia dvojrozmerneho pola na jednorozmerny vystup
     */
    for(k=0;k<MAX_N;k++){
        TOA_mem[2*k]=P[0][k];
        TOA_mem[2*k+1]=P[1][k];
    }
    return TOA_mem;
}

float *mtt_pure::normalizing(float *inptr)
{
    int i;
    float max_abs_value;
    float mean; UNUSED(mean); // seems to be unused

    mean = mean_vector( inptr, SCANLENGHT);
    max_abs_value = max_abs_vector( inptr, SCANLENGHT);
    if ( max_abs_value == 0) {
        clear_vector( inptr, SCANLENGHT );
    }
    else {
        for( i=0; i<SCANLENGHT; i++ )
                inptr[i] /= max_abs_value;
    }
    return inptr;
}

float *mtt_pure::normalizing_new(float *inptr)
{
    int i;
    float max_abs_value;
    float mean; UNUSED(mean); // seems to be unused

    mean = mean_vector( inptr, SCANLENGHT);
    max_abs_value=max_abs_vector(inptr, SCANLENGHT);
    if(max_abs_value == 0){
        clear_vector(inptr, SCANLENGHT);
    }
    else{
        for(i=0;i<SCANLENGHT; i++){
            if(fabs(inptr[i])==max_abs_value){
                inptr[i]/=max_abs_value;
                if(i!=SCANLENGHT-1){
                    max_abs_value=max_abs_vector(inptr+i+1, SCANLENGHT-i-1);
                }
            }else{
                inptr[i]/=max_abs_value;
            }
        }
    }
    return inptr;
}

void mtt_pure::connected_reflections(mtt_pure::word *ch, mtt_pure::word trace_con[], mtt_pure::word c[], mtt_pure::word m, mtt_pure::word *nn_obs)
{
    word pos=0;
    word i,j;
    word reflection_e=0;
    word cover_e=0;
    c[0] = c[1] = c[2] = 0;
    //	(*ch)+=1;
    //	(*nn_obs)+=1;

    /*   reflection_e = ch + find(trace_con(ch+1:end)==0,1) - 1; first zero entry
    reflection_e = ch + find(trace_con(ch+1:end)==0,1) - 1;*/

    for(i= (*ch)+1,j=0;i<=SCANLENGHT;i++,j++){
        if(trace_con[i]==0){
            pos=j;
            break;
        }
    }
    reflection_e= *ch+pos-1;
    pos=-1;

    /*cover_e = reflection_e - find(trace_con(reflection_e:-1:reflection_e-2*m)==3,1) + 1;*/
    for(i=reflection_e,j=0;i>=reflection_e-2*m;i--,j++){
        if(trace_con[i]==3){
            pos=j;
            break;
        }
    }
    cover_e=reflection_e-pos+1;


    /*if isempty(cover_e) == 0
        c(3) = 3 - trace_con(reflection_e);
        c(2) = 2*m + 1 - (reflection_e - cover_e);
        c(1) = cover_e - floor(c(2)/2);
        if mod(c(2),2)==0
            c(1) = c(1) + 1;
        end
    else
        nn_obs = nn_obs - 1;
    end
         if(find_vec(cover_e)!=EMPTY){*/
    if(pos!=-1){
         c[2]=3 - trace_con[reflection_e];
         c[1]=2*m + 1 - (reflection_e - cover_e);
         c[0]=cover_e - c[2]/2;
         if(c[1]%2==0){
             c[0]++;
         }
    }
    else{
        (*nn_obs)--;
    }


    /*ch = reflection_e + 1;*/
    *ch=reflection_e+1;
}

void mtt_pure::munkres(mtt_pure::real costMat[][MAX_N], mtt_pure::word rows, mtt_pure::word cols, mtt_pure::real *cost, mtt_pure::word assign[][MAX_N])
{
    word validCol[ MAX_N ];
    word validRow[ MAX_N ];
    real dMat[ MAX_N ][ MAX_N ], minval;
    real M[ MAX_N ][ MAX_N ];
    word M_row, M_col;
    real min_row;

    word zP[ MAX_N ][ MAX_N ];
    word starZ[ MAX_N ][ MAX_N ];
    word primeZ[ MAX_N ][ MAX_N ];
    word coverColumn[ MAX_N ];
    word coverRow[ MAX_N ];
    word stz[ MAX_N ];
    word rowZ1[ MAX_N ];

    word nRows, nCols, j, k, n, r, c, uZr, uZc, Step;

    /*
    nRows = sum(validRow);
    nCols = sum(validCol);
    n = max(nRows,nCols);
    if ~n
        return
    end
    */
       nRows = rows;
       nCols = cols;

       n = MAX( nRows, nCols);
       if (n==0)
            return;

    /*
    validCol = any(validMat);
    */
       for( k=0; k<nRows; k++ ) {
            validRow[k] = FALSE;
            for( j=0; j<nCols; j++ ) {
                 if( costMat[k][j] != 0 ) {
                      validRow[k] = TRUE;       // can braek here
                 }
            }
       }
    /*
    validRow = any(validMat,2);
    */
       for( j=0; j<nCols; j++ ) {
            validCol[j] = FALSE;
            for( k=0; k<nRows; k++ ) {
                 if( costMat[k][j] != 0 ) {
                      validCol[j] = TRUE;       // can braek here
                 }
            }
       }

    /*
    assignment = false(size(costMat));
    cost = 0;
    */
       for( j=0; j<nRows; j++ ) {
            for( k=0; k<nCols; k++ ) {
                assign[j][k] = FALSE;
            }
       }
       *cost = 0;


    /*
    dMat = zeros(n);
    */
       zeros( dMat, n, n );

    /*
    dMat(1:nRows,1:nCols) = costMat(validRow,validCol);
    */

       extract_valid( costMat, validRow, validCol, dMat, nRows, nCols);

    /*#if(0)
    %*************************************************
    % Munkres' Assignment Algorithm starts here
    %*************************************************

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %   STEP 1: Subtract the row minimum from each row.
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
     dMat = bsxfun(@minus, dMat, min(dMat,[],2));
    #endif*/

       for( j=0; j<n; j++ ) {
            min_row = FLT_MAX;
            for( k=0; k<n; k++ ) {
                if (min_row > dMat[j][k])
                    min_row = dMat[j][k];
            }
            for( k=0; k<n; k++ ) {
                dMat[j][k] -= min_row;
            }

       }

    /*
    %**************************************************************************
    %   STEP 2: Find a zero of dMat. If there are no starred zeros in its
    %           column or row start the zero. Repeat for each zero
    %**************************************************************************
    zP = ~dMat;
    starZ = false(n);
    while any(zP(:))
        [r,c]=find(zP,1);
        starZ(r,c)=true;
        zP(r,:)=false;
        zP(:,c)=false;
    end
    */
        log_negate( dMat, n, n, zP );
        falses( starZ, n, n );
        while( any( zP, n, n) ) {
            find( zP, n, n, &r, &c );
            starZ[r][c] = TRUE;
            for(j=0; j<n; j++)
                    zP[r][j] = FALSE;
            for(j=0; j<n; j++)
                    zP[j][c] = FALSE;
        }

    /*
    while 1
    %**************************************************************************
    %   STEP 3: Cover each column with a starred zero. If all the columns are
    %           covered then the matching is maximum
    %**************************************************************************
        primeZ = false(n);
        coverColumn = any(starZ);
        if ~any(~coverColumn)
            break
        end
        coverRow = false(n,1);
    */

    while( 1 ) {
        falses( primeZ, n, n );
        any_col( starZ, n, n, coverColumn );
        if( !any_vector_not_selected ( coverColumn, n ) )
            break;
        falses_vector( coverRow, n );

    /*
        while 1
            %**************************************************************************
            %   STEP 4: Find a noncovered zero and prime it.  If there is no starred
            %           zero in the row containing this primed zero, Go to Step 5.
            %           Otherwise, cover this row and uncover the column containing
            %           the starred zero. Continue in this manner until there are no
            %           uncovered zeros left. Save the smallest uncovered value and
            %           Go to Step 6.
            %**************************************************************************
            zP(:) = false;
            zP(~coverRow,~coverColumn) = ~dMat(~coverRow,~coverColumn);
            Step = 6;
    */
     while( 1 ) {
         falses( zP, n, n);
         for( j=0; j<n; j++ ) {
            if ( !coverRow[j] ) {
                 for( k=0; k<n; k++ ) {
                    if( !coverColumn[k] )
                         if (dMat[j][k] == 0)
                         zP[j][k] = TRUE;
                 }
            }
         }
         Step = 6;
    /*
            while any(any(zP(~coverRow,~coverColumn)))
                [uZr,uZc] = find(zP,1);
                primeZ(uZr,uZc) = true;
                stz = starZ(uZr,:);
                if ~any(stz)
                    Step = 5;
                    break;
                end
                coverRow(uZr) = true;
                coverColumn(stz) = false;
                zP(uZr,:) = false;
                zP(~coverRow,stz) = ~dMat(~coverRow,stz);
            end
    */

        while( any_not_selected ( zP, coverRow, coverColumn, n, n ) ) {
            find( zP, n, n, &uZr, &uZc );
            primeZ[uZr][uZc] = TRUE;
            copy_row( starZ, uZr, n, stz );
            if (!any_vector( stz,n ) ) {
                Step = 5;
                break;
            }
            coverRow[uZr] = TRUE;

            for( j=0; j<n; j++ )
                if( stz[j] )
                    coverColumn[ j ] = FALSE;

            for( j=0; j<n; j++ )
                 zP[uZr][j] = FALSE;

            for( j=0; j<n; j++ ) {
                if ( !coverRow[j] ) {
                   for( k=0; k<n; k++ ) {
                       if( stz[k] )
                           zP[j][k] = !dMat[j][k];
                   }
                }
            }

        }

    /*
            if Step == 6
                % *************************************************************************
                % STEP 6: Add the minimum uncovered value to every element of each covered
                %         row, and subtract it from every element of each uncovered column.
                %         Return to Step 4 without altering any stars, primes, or covered lines.
                %**************************************************************************
                M=dMat(~coverRow,~coverColumn);
                minval=min(min(M));
                if minval==inf
                    return
                end
                dMat(coverRow,coverColumn)=dMat(coverRow,coverColumn)+minval;
                dMat(~coverRow,~coverColumn)=M-minval;
            else
                break
            end
        end
    */
        if (Step == 6) {

             extract_not_valid( dMat, coverRow, coverColumn, n, n, M, &M_row, &M_col );
             minval = find_min( M, M_row, M_col);

             for( j=0; j<n; j++ ) {
                if ( coverRow[j] ) {
                       for( k=0; k<n; k++ ) {
                          if( coverColumn[k] )
                                  dMat[j][k] += minval;
                          }
                }
             }

             for( j=0; j<M_row; j++ ) {
                       for( k=0; k<M_col; k++ ) {
                                   M[j][k] -= minval;
                       }
             }

             insert_to_not_valid( M, M_row, M_col, dMat, coverRow, coverColumn );
        }
        else
            break;
    }
    /*
        %**************************************************************************
        % STEP 5:
        %  Construct a series of alternating primed and starred zeros as
        %  follows:
        %  Let Z0 represent the uncovered primed zero found in Step 4.
        %  Let Z1 denote the starred zero in the column of Z0 (if any).
        %  Let Z2 denote the primed zero in the row of Z1 (there will always
        %  be one).  Continue until the series terminates at a primed zero
        %  that has no starred zero in its column.  Unstar each starred
        %  zero of the series, star each primed zero of the series, erase
        %  all primes and uncover every line in the matrix.  Return to Step 3.
        %**************************************************************************
        rowZ1 = starZ(:,uZc);
        starZ(uZr,uZc)=true;
        while any(rowZ1)
            starZ(rowZ1,uZc)=false;
            uZc = primeZ(rowZ1,:);
            uZr = rowZ1;
            rowZ1 = starZ(:,uZc);
            starZ(uZr,uZc)=true;
        end

        }
    */

    copy_col ( starZ, n, uZc, rowZ1 );
    starZ[uZr][uZc] = TRUE;

    while( any_vector( rowZ1, n) ) {
                for( j=0; j<n; j++ ) {
                        if ( rowZ1[j] ) {
                            starZ[j][uZc] = FALSE;
                        }
                }
                for( j=0; j<n; j++ ) {
                       if ( rowZ1[j] ) {
                          for( k=0; k<n; k++ ) {
                               if(primeZ[j][k])
                               uZc = k;
                          }
                       }
                }

                uZr = find_vec( rowZ1, n );
                copy_col ( starZ, n, uZc, rowZ1 );
                starZ[uZr][uZc] = TRUE;
    }
    /*
    end
    */
    }

    /*
    % Cost of assignment
    assignment(validRow,validCol) = starZ(1:nRows,1:nCols);
    cost = sum(costMat(assignment));
    */
    insert_to_valid( starZ, nRows, nCols, assign, validRow, validCol );
    *cost = 0;

    for( j=0; j<n; j++ ) {
              for( k=0; k<n; k++ ) {
                     if( assign[j][k] ) {
                             *cost += costMat[j][k];
                     }
              }
    }
}

/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* ----------------------------- supporting functions end ------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* ------------------------------ MUNKRES SUPPORT functions ----------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */

void mtt_pure::zeros( mtt_pure::real in[][MAX_N], mtt_pure::word m, mtt_pure::word n )
{
    mtt_pure::word i,j;
        for( i=0; i<m; i++ )
            for( j=0; j<n; j++ )
                    in[i][j] = 0.0;
}

void mtt_pure::log_negate( mtt_pure::real in[][MAX_N], mtt_pure::word m, mtt_pure::word n, mtt_pure::word out[][MAX_N]  )
{
    mtt_pure::word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                if( in[i][j] == 0)
                      out[i][j] = TRUE;
                else
                      out[i][j] = FALSE;
}

void mtt_pure::falses( mtt_pure::word in[][MAX_N], mtt_pure::word m, mtt_pure::word n )
{
    mtt_pure::word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                in[i][j] = FALSE;

}

void mtt_pure::falses_vector( mtt_pure::word in[], mtt_pure::word n )
{
    mtt_pure::word i;
    for( i=0; i<n; i++ )
                in[i] = FALSE;
}


mtt_pure::word mtt_pure::any( mtt_pure::word in[][MAX_N], mtt_pure::word m, mtt_pure::word n )
{
    mtt_pure::word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                     if (in[i][j])
                             return TRUE;
    return FALSE;
}

mtt_pure::word mtt_pure::any_vector( mtt_pure::word in[], mtt_pure::word n )
{
    mtt_pure::word i;
    for( i=0; i<n; i++ )
        if (in[i])
                return TRUE;
    return FALSE;
}

mtt_pure::word mtt_pure::any_not_selected (mtt_pure::word in[][MAX_N], mtt_pure::word row_sel[], mtt_pure::word col_sel[], mtt_pure::word m, mtt_pure::word n )
{
    mtt_pure::word i,j;
    for( i=0; i<m; i++ )
        if( !row_sel[i] ) {
                for( j=0; j<n; j++ ) {
                        if( !col_sel[j] )
                                if (in[i][j])
                                        return TRUE;
                }
        }
    return FALSE;
}

mtt_pure::word mtt_pure::any_vector_not_selected ( mtt_pure::word in[], mtt_pure::word m )
{
    mtt_pure::word i;
    for( i=0; i<m; i++ ) {
        if( !in[i] )
            return TRUE;
    }
    return FALSE;
}

void mtt_pure::any_col( mtt_pure::word in[][MAX_N], mtt_pure::word m, mtt_pure::word n, mtt_pure::word out[]  )
{
    mtt_pure::word i,j;
    for( j=0; j<n; j++ ) {
        out[j] = FALSE;
        for( i=0; i<m; i++ )
                if (in[i][j])
                        out[j] = TRUE;
    }
}

void mtt_pure::find( mtt_pure::word in[][MAX_N], mtt_pure::word m, mtt_pure::word n, mtt_pure::word *r, mtt_pure::word *s )
{
    mtt_pure::word i,j;
    *r = -1;
    *s = -1;
    for( j=0; j<n; j++ )        // checks complete columns
                for( i=0; i<m; i++ )
                      if (in[i][j]) {
                           *r = i;
                           *s = j;
                            return;
                      }
    return;
}

/*
mtt_pure::word mtt_pure::find_vec( mtt_pure::word in[], mtt_pure::word n ) {
    mtt_pure::word i;
    for( i=0; i<n; i++ )        // checks complete vector
         if (in[i])
              return i;
    return -1;
}
*/

void mtt_pure::copy_row ( mtt_pure::word in[][MAX_N], mtt_pure::word m, mtt_pure::word n, mtt_pure::word out[] )
{
    mtt_pure::word i;
    for (i=0; i<n; i++) {
        out[i] = in[m][i];
    }
}

void mtt_pure::copy_col ( mtt_pure::word in[][MAX_N], mtt_pure::word m, mtt_pure::word n, mtt_pure::word out[] )
{
    mtt_pure::word i;
    for (i=0; i<m; i++) {
        out[i] = in[i][n];
    }
}

mtt_pure::real mtt_pure::find_min ( mtt_pure::real in[][MAX_N], mtt_pure::word m, mtt_pure::word n )
{
    mtt_pure::word i, j;
    mtt_pure::real val = FLT_MAX;
    for (i=0; i<m; i++) {
        for(j=0; j<n; j++ )
             if( in[i][j] < val )
                  val =  in[i][j];
    }

    return val;
}

void mtt_pure::extract_valid( mtt_pure::real in[][MAX_N], mtt_pure::word row_sel[], mtt_pure::word col_sel[], mtt_pure::real out[][MAX_N], mtt_pure::word m, mtt_pure::word n)
{
    mtt_pure::word i,j, i_out, j_out;
    i_out = 0;
    for(i=0; i<m; i++ ) {
       if( row_sel[i] ) {                // process only valid rows
            j_out = 0;
            for(j=0; j<n; j++) {         // check all cols
                if( col_sel[j] ) {       // only valid cols
                    out[i_out][j_out] = in[i][j];
                }
                j_out++;
            }
            i_out++;
       }
    }
}

void mtt_pure::extract_not_valid( mtt_pure::real in[][MAX_N], mtt_pure::word row_sel[], word col_sel[], mtt_pure::word m, mtt_pure::word n, mtt_pure::real out[][MAX_N], mtt_pure::word *r, mtt_pure::word *c)
{
    UNUSED(m);

    word i,j, i_out, j_out;

    j_out = 0;
    for(j=0; j<n; j++ ) {
        if( !col_sel[j] ) {              // process only non valid cols
            i_out = 0;
            for(i=0; i<n; i++) {         // check all rows
                if( !row_sel[i] ) {      // only non valid rows
                    out[i_out][j_out] = in[i][j];
                    i_out++;
                }
            }
            j_out++;
       }
    }
    *r = i_out;
    *c = j_out;
}

void mtt_pure::insert_to_not_valid( mtt_pure::real in[][MAX_N], mtt_pure::word m, mtt_pure::word n, mtt_pure::real out[][MAX_N], mtt_pure::word row_sel[], mtt_pure::word col_sel[] )
{
    mtt_pure::word i,j, i_in, j_in;
    i_in = 0; i=0;
    while( i_in <m ) {
        if( !row_sel[i] ) {        // process only non valid rows
             j=0; j_in = 0;
             while( j_in <n ) {
                     if( !col_sel[j] ) {
                            out[i][j] = in[i_in][j_in];
                            j_in++;
                     }
                     j++;
             }
             i_in++;
        }
        i++;
    }
}

void mtt_pure::insert_to_valid( mtt_pure::word in[][MAX_N], mtt_pure::word m, mtt_pure::word n, word out[][MAX_N], mtt_pure::word row_sel[], mtt_pure::word col_sel[] )
{
    mtt_pure::word i,j, i_in, j_in;
    i_in = 0; i = 0;
    while( i_in <m ) {
        if( row_sel[i] ) {
              j=0; j_in = 0;
              while( j_in <n ) {
                      if( col_sel[j] ) {
                         out[i][j] = in[i_in][j_in];
                         j_in++;
                      }
                      j++;
              }
              i_in++;
        }
        i++;
    }
}

/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* ---------------------------- MUNKRES SUPPORT functions end --------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------- radar data processing functions --------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------- */

float *mtt_pure::exponential_bg_subtraction(float *data_in, int ch, float exp_factor)
{
    int k;

    /*    if (start_ex_av != 0) {
        start_ex_av = 0;
    for( k=0; k<SCANLENGHT; k++ ) {
                IR_buffer[0][k] = 0;
                IR_buffer[1][k] = 0;
        }
    }*/

    for( k=0; k<SCANLENGHT; k++ ) {
                bg_estimation[k] = data_in[k] * (float) (1.0-exp_factor)+ exp_factor*IR_buffer[ch][k];
                IR_buffer[ch][k] = bg_estimation[k];
    }

    for( k=0; k<SCANLENGHT; k++ ) {
        data_out[k] = data_in[k] - bg_estimation[k];
    }
    for( k=0; k<SCANLENGHT; k++ ) {
        data_in[k] = data_out[k];
    }
    return data_in;
}

int *mtt_pure::detector_cfar(float *inptr, int *outptr, float beta_fast, float beta_slow, float alpha_det)
{
      int i;
    //   int* outptr;
       float  X_initial, Y_initial, S_2_initial;



    //   outptr=malloc(SCANLENGHT*sizeof(int));

    /*
        data_in = data_in - repmat(mean(data_in),n_r,1);
    */
       for( i=0; i<SCANLENGHT; i++ ) {
               data[i] = inptr[i];     // temporary storage
       }

    /*
       data_in_magnitude = abs(data_in);
       data_in_energy = data_in.^2;
    */
       for( i=0; i<SCANLENGHT; i++ ) {
               data[i] = (float) fabs( data[i]);  // signal magnitude
               tmp[i] = data[i]*data[i];         // signal energy
       }

    /*
        X_initial = zeros(1,n_c);
        Y_initial = zeros(1,n_c);
        S_2_initial = zeros(1,n_c);

        for i=1:200
            X_initial = beta_fast*data_in_magnitude(i,:)+(1-beta_fast)*X_initial;
            Y_initial = beta_slow*data_in_magnitude(i,:)+(1-beta_slow)*Y_initial;
            S_2_initial = beta_slow*data_in_energy(i,:)+(1-beta_slow)*S_2_initial;
        end
    */
        X_initial = Y_initial = S_2_initial = 0;
        for( i=0; i<200; i++ ) {
            X_initial = beta_fast*data[i] + (1-beta_fast)*X_initial;
            Y_initial = beta_slow*data[i] + (1-beta_slow)*Y_initial;
            S_2_initial = beta_slow*tmp[i] + (1-beta_slow)*S_2_initial;
        }

    /*
        X = zeros(n_r,n_c);
        X(1,:) = X_initial;

        Y = zeros(n_r,n_c);
        Y(1,:) = Y_initial;

        S_2 = zeros(n_r,n_c);
        S_2(1,:) = S_2_initial;

        for i=1:n_r-1
            X(i+1,:) = beta_fast*data_in_magnitude(i,:)+(1-beta_fast)*X(i,:);
            Y(i+1,:) = beta_slow*data_in_magnitude(i+1,:)+(1-beta_slow)*Y(i,:);
            S_2(i+1,:) = beta_slow*data_in_energy(i+1,:)+(1-beta_slow)*S_2(i,:);
        end
    */
        X[0] = X_initial;
        Y[0] = Y_initial;
        S_2[0] = S_2_initial;
        for (i=0; i<SCANLENGHT-1;i++) {
            X[i+1] = beta_fast*data[i] + (1-beta_fast)*X[i];
            Y[i+1] = beta_slow*data[i+1] + (1-beta_slow)*Y[i];
            S_2[i+1] = beta_slow*tmp[i+1] + (1-beta_slow)*S_2[i];
        }


    /*
        S = sqrt(S_2);
        data_out = alfa*S+Y;
        out_det(:,:,k) = (X > data_out);
        out_det(1:5,:,k) = zeros(5,n_c);
    */
    outptr[0] = outptr[1] = outptr[2] = outptr[3] = outptr[4] =0;
    for (i=5; i<SCANLENGHT;i++) {
        data[i] = (float) (alpha_det*sqrt(S_2[i]) + Y[i]);
        outptr[i] = (int) (X[i] > data[i]);
    }
    return outptr;
}

void mtt_pure::TOA_couple(mtt_pure::word max_nn_tg, mtt_pure::word center[][3], mtt_pure::word m, mtt_pure::real TOA_m[][MAX_N])
{
    UNUSED(max_nn_tg);

    word track;
    word i,j;
    real c;
    word sgn;
    word TOA_ch[2][MAX_N];

    for(i=0;i<MAX_N;i++){
        TOA_ch[0][i]= 0;
        TOA_ch[1][i]= 0;
    }
    c =  (real) SPEED_OF_LIGHT;
    sgn = 0;

    for( track = 0; track < MAX_N; track++ ) {
        if( center[track][0] > 0 ) {
             if (center[track][2] == 1)
                  sgn = 1;
             else if(center[track][2] ==2)
                  sgn = -1;
             else
                 sgn = 0; /*pridane kvoli uplnemu prekrytiu*/

                 TOA_ch[0][track] = ((center[track][0]+INDEX_CORRECTION)- (sgn*round((m - center[track][1]/2))) ); /*pridane zaokruhlenie*/
                 TOA_ch[1][track] = ((center[track][0]+INDEX_CORRECTION)+ (sgn*round((m - center[track][1]/2))) );/*pridane zaokruhlenie*/
                 if(center[track][1] % 2 == 0){ /*pridane posunutie*/
                     if(TOA_ch[0][track]<TOA_ch[1][track]){
                         TOA_ch[0][track]-=1;
                     }
                     else if(TOA_ch[0][track] > TOA_ch[1][track]){
                         TOA_ch[1][track]-=1;
                     }
                     else if(sgn == 1){
                         TOA_ch[0][track]-=1;
                     }
                     else TOA_ch[1][track]-=1;
                 }
            }
   }
   for(i=0;i<2;i++){  /*zmena prepocet*/
       for(j=0;j<MAX_N;j++){
           TOA_m[i][j]=(real)(c/FREQUENCY)*TOA_ch[i][j];
       }
   }
}

void mtt_pure::InitScan()
{
    // ???????????
}

void mtt_pure::trace_online(mtt_pure::real *inptr, mtt_pure::word ch, mtt_pure::word t_size, mtt_pure::real thr, mtt_pure::word min_integration, mtt_pure::word h_window, mtt_pure::word m, mtt_pure::word samples)
{
    UNUSED(inptr);
    UNUSED(ch);
    UNUSED(t_size);
    UNUSED(thr);
    UNUSED(min_integration);
    UNUSED(h_window);
    UNUSED(m);
    UNUSED(samples);
    // ???????????
}
