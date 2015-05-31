#include "radarunit.h"

radarUnit::radarUnit(int radarId, double x_pos, double y_pos, double rot_Angle, bool enable)
{
    radar_id = radarId;
    start = 1;
    max_recursion = 5;

    xpos = x_pos;
    ypos = y_pos;
    rotAngle = rot_Angle;

    tempX = tempY = 0.0;

    dataList = new QList<rawData * >;

    enabled = enable;
}

radarUnit::~radarUnit()
{
    while(!dataList->isEmpty())
    {
        delete dataList->first();
        dataList->removeFirst();
    }
}

bool radarUnit::processNewData(rawData *data, bool enableMTT)
{

    reciever_method method = data->getRecieverMethod();

    // constants for MTT
    float r[] = {0.1, 0.01};
    float q[] = {0.0, 0.01, 0.0, 0.0001};
    float diff_d = 1.0;
    float diff_fi = 0.6;
    int min_NT = 10;
    int min_OLGI = 10;

    if(method==SYNTHETIC || method==RS232)
    {
        #if defined MTT_ARRAY_FIT && MTT_ARRAY_FIT==1
            zeroEmptyPositions(data);
        #endif


        if(enableMTT)
        {
            qDebug() << "Running MTT";

            if(data->getRecieverMethod()==RS232)
                this->MTT(data->getUwbPacketCoordinates(), r, q, diff_d, diff_fi, min_OLGI, min_NT);
            #if defined (__WIN32__)
            else if(data->getRecieverMethod()==SYNTHETIC)
                this->MTT(data->getSyntheticCoordinates(), r, q, diff_d, diff_fi, min_OLGI, min_NT);
            #endif
            else qDebug() << "MTT could not run, because of unknown reciever method";
        }

        if(dataList->count()>=max_recursion)
        {
            // remove the oldest record
            delete dataList->first();
            dataList->removeFirst();
            dataList->append(data);
        }
        else
        {
            // just append the new record
            dataList->append(data);
        }

        return true;
    }
    else
    {
        // no known method
        qDebug() << "Method was not recognized.";
        return false;
    }

    return false;
}

int radarUnit::getNumberOfTargetsLast()
{
    if(dataList->isEmpty()) return -1; // default return

    reciever_method method = dataList->last()->getRecieverMethod();
    if(method==RS232) return dataList->last()->getUwbPacketTargetsCount();
    #if defined (__WIN32__)
    else if(method==SYNTHETIC) return dataList->last()->getSyntheticTargetsCount();
    #endif
    else return 0;
    return 0;
}

float *radarUnit::getCoordinatesLast()
{
    if(dataList->isEmpty()) return NULL; // default return

    reciever_method method = dataList->last()->getRecieverMethod();
    if(method==RS232) return dataList->last()->getUwbPacketCoordinates();
    #if defined (__WIN32__)
    else if(method==SYNTHETIC) return dataList->last()->getSyntheticCoordinates();
    #endif
    else return NULL;
    return NULL;
}

int radarUnit::getNumberOfTargetsAt(int index)
{
    if(dataList->isEmpty()) return -1; // default return

    reciever_method method = dataList->at(index)->getRecieverMethod();

    if(method==RS232) return dataList->at(index)->getUwbPacketTargetsCount();
    #if defined (__WIN32__)
    else if(method==SYNTHETIC) return dataList->at(index)->getSyntheticTargetsCount();
    #endif
    else return 0;
    return 0;
}

float *radarUnit::getCoordinatesAt(int index)
{
    if(dataList->isEmpty()) return NULL; // default return

    reciever_method method = dataList->at(index)->getRecieverMethod();
    if(method==RS232) return dataList->at(index)->getUwbPacketCoordinates();
    #if defined (__WIN32__)
    else if(method==SYNTHETIC) return dataList->at(index)->getSyntheticCoordinates();
    #endif
    else return NULL;
    return NULL;
}

void radarUnit::doTransformation(float x, float y)
{
    // Preparing objects for 2D transformation
    vector< vector<float> * > * radarUnitShiftMatrix;
    vector< vector<float> * > * radarUnitRotationMatrix = new vector< vector<float> * >;
    vector< vector<float> * > * radarUnitCoordinateMatrix;

    //qDebug() << "Starting transformation of x: " << x << " y: " << y;

    //qDebug() << "Radar parameters x: " << xpos << " y: " << ypos << " angle: " << rotAngle;

    float shiftM[] = { (float)(xpos), (float)(ypos) };
    float cosAngle = (float)cos(rotAngle);
    float sinAngle = (float)sin(rotAngle);

    //qDebug() << "Sin " << sinAngle << " Cos " << cosAngle;

    float rotationMrowA[] = { cosAngle, (-1.0)*sinAngle };
    float rotationMrowB[] = { sinAngle, cosAngle };
    float coordinateM[] = { x, y };

    addRow(radarUnitRotationMatrix, rotationMrowA, sizeof(rotationMrowA));
    addRow(radarUnitRotationMatrix, rotationMrowB, sizeof(rotationMrowB));

    radarUnitCoordinateMatrix = createColumnMatrix(coordinateM, sizeof(coordinateM));
    radarUnitShiftMatrix = createColumnMatrix(shiftM, sizeof(shiftM));

    // apply transformation equation
    vector< vector<float> * > * productedPart = productMatrix(radarUnitRotationMatrix, radarUnitCoordinateMatrix);
    vector< vector<float> * > * transformationResult = sumMatrix(radarUnitShiftMatrix, productedPart);

    // save calculated values
    tempX = getVal(1, 1, transformationResult);
    tempY = getVal(2, 1, transformationResult);

    //qDebug() << "Result of transformation is x: " << tempX << " y: " << tempY;

    // free memory
    deleteMatrix(radarUnitShiftMatrix);
    deleteMatrix(radarUnitRotationMatrix);
    deleteMatrix(radarUnitCoordinateMatrix);
    deleteMatrix(productedPart);
    deleteMatrix(transformationResult);
}

void radarUnit::zeroEmptyPositions(rawData * array)
{
    // TOAS ARE NOT PROCESSED AS COORDINATES YET. SO FAR THERE WAS NO NEED TO DO THAT.
    if(array==NULL) return;

    reciever_method r_method = array->getRecieverMethod();

    int count = 0;
    float * values = NULL;

    if(r_method==RS232)
    {
        count = array->getUwbPacketTargetsCount();
        values = array->getUwbPacketCoordinates();
    }
    #if defined (__WIN32__)
    else if(r_method==SYNTHETIC)
    {
        count = array->getSyntheticTargetsCount();
        values = array->getSyntheticCoordinates();
    }
    #endif
    else return; // unknown method

    // catch the error pointers
    if(values==NULL || values<=0) return;

    if(count<MAX_N)
    {
        // if less than MAX_N, need to put zeros on unused positions
        int i;
        for(i=count; i<MAX_N; i++) values[i*2] = values[i*2+1] = 0.0;
    }
    else if(count>MAX_N)
    {
        // NOTE: IN THE LATEST IMPLEMENTATION, MACRO MTT_ARRAY_FIT IS USED TO UNLOCK CODE FOR ALLOCATING MAX_N SIZE OF ARRAY
        // IN TIME OF DATA RECIEVING. SINCE THIS FUNCTION WILL BE UNLOCKED AT THE SAME CONDITIONS AS THE MENTIONED CODES,
        // REALLOCATION OF ARRAY IS NOT NEEDED. HOWEVER, STILL MAY BE USEFUL FOR SOME LATER MODIFICATIONS, THEREFORE REALLOCATION
        // IS JUST COMMENTED.

        // if more, need to realloc array to fit the MTT requirements
        /*
        float * newArr = new float[MAX_N*2];

        memcpy(newArr, values, MAX_N*2*sizeof(float));

        delete [] values;

        values = newArr;

        // save new array pointer
        if(r_method==SYNTHETIC)
            array->setSyntheticCoordinatesPointer(values);
        else if(r_method==RS232)
            array->setUwbPacketCoordinates(values);
        else return; // although this will never happen due to above conditions
        */
    }

}

/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/**************************************** DATA PROCESSING FUNCTIONS *************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/


/************ Supporting "matrixr" & Matlab  functions *********************/
void radarUnit::matrix_mul_4x2( real c[][2], real a[][2], real b[][2]  ) {
word k,j;

     for( k=0; k<4; k++ ) {
        for( j=0; j<2; j++ ) {
             c[k][j] = (real) a[k][0]*b[0][j] + a[k][1]*b[1][j];
        }
     }
}

void radarUnit::matrix_mul_4x4( real c[][4], real a[][4], real b[][4]  ) {
word k,j;

     for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
                       c[k][j] = a[k][0]*b[0][j] + a[k][1]*b[1][j] + a[k][2]*b[2][j] + a[k][3]*b[3][j];
        }
     }
}

void radarUnit::matrix_transpose_4x4( real out[][4], real in[][4]  ) {
word k,j;

     for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
             out[j][k] = in[k][j];
        }
     }

}

void radarUnit::matrix_add_4x4( real c[][4], real a[][4], real b[][4] ) {
word k,j;

     for( k=0; k<4; k++ ) {
        for( j=0; j<4; j++ ) {
             c[k][j] = a[k][j] + b[k][j];
        }
     }
}

/* 4x4 matrices (P_e_p[][4], real P_p[][4]) are reffered as 16x1 vectors as 3D arrays are used in MTT function */

/* code can be simplified ... */

void radarUnit::prediction ( real Y_e_p[], real *P_e_p, real Q[][4], real Y_p[], real *P_p ) {
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

/* P_e is stored as [4x4] matrix!!! */
void radarUnit::init_estimation (real r, real fi, real Y_e_2_init, real Y_e_4_init, real P_init[][4], real Y_e[], real *P_e ) {
word k, j;

    Y_e[0] = r;
    Y_e[1] =  Y_e_2_init;
    Y_e[2] = fi;
    Y_e[3] = Y_e_4_init;

    for( k=0; k<4; k++ )
        for( j=0; j<4; j++ )
                P_e[4*k+j] = P_init[k][j];

}


void radarUnit::cartesian2polar( real x, real y, real *r, real *fi) {
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

/* 4x4 matrix (P_p[][4]) are reffered as 16x1 vectors as 3D arrays are used in MTT function */

void radarUnit::gate_checker ( real r, real fi, real Y_p[], real R[][2], real *P_p, word *m, real *c) {
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

/* 4x4 matrices (P_e_last[][4], P_e[][4] ) are reffered as 16x1 vectors as 3D arrays are used in MTT function */
void radarUnit::obs_less_gate_ident (word *OLGI_last,word track, word nn_track, real Y_e_last[], real *P_e_last,
                          real Y_e[], real *P_e, word *OLGI) {

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

void radarUnit::zeros( real in[][MAX_N], word m, word n ) {
    word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                in[i][j] = 0.0;

}

void radarUnit::log_negate( real in[][MAX_N], word m, word n, word out[][MAX_N]  ) {
    word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                if( in[i][j] == 0)
                      out[i][j] = TRUE;
                else
                      out[i][j] = FALSE;
}

void radarUnit::falses( word in[][MAX_N], word m, word n ) {
    word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                in[i][j] = FALSE;

}

void radarUnit::falses_vector( word in[], word n ) {
    word i;
    for( i=0; i<n; i++ )
                in[i] = FALSE;
}


word radarUnit::any( word in[][MAX_N], word m, word n ) {
    word i,j;
    for( i=0; i<m; i++ )
        for( j=0; j<n; j++ )
                     if (in[i][j])
                             return TRUE;
    return FALSE;
}

word radarUnit::any_vector( word in[], word n ) {
    word i;
    for( i=0; i<n; i++ )
        if (in[i])
                return TRUE;
    return FALSE;
}

word radarUnit::any_not_selected ( word in[][MAX_N], word row_sel[], word col_sel[], word m, word n ) {
    word i,j;
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

word radarUnit::any_vector_not_selected ( word in[], word m ) {
    word i;
    for( i=0; i<m; i++ ) {
        if( !in[i] )
            return TRUE;
    }
    return FALSE;
}

void radarUnit::any_col( word in[][MAX_N], word m, word n, word out[]  ) {
    word i,j;
    for( j=0; j<n; j++ ) {
        out[j] = FALSE;
        for( i=0; i<m; i++ )
                if (in[i][j])
                        out[j] = TRUE;
    }
}

void radarUnit::find( word in[][MAX_N], word m, word n, word *r, word *s ) {
    word i,j;
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


word radarUnit::find_vec( word in[], word n ) {
    word i;
    for( i=0; i<n; i++ )        // checks complete vector
         if (in[i])
              return i;
    return -1;
}

void radarUnit::copy_row ( word in[][MAX_N], word m, word n, word out[] ) {
    word i;
    for (i=0; i<n; i++) {
        out[i] = in[m][i];
    }
}

void radarUnit::copy_col ( word in[][MAX_N], word m, word n, word out[] ) {
    word i;
    for (i=0; i<m; i++) {
        out[i] = in[i][n];
    }
}

real radarUnit::find_min ( real in[][MAX_N], word m, word n ) {
    word i, j;
    real val = FLT_MAX;
    for (i=0; i<m; i++) {
        for(j=0; j<n; j++ )
             if( in[i][j] < val )
                  val =  in[i][j];
    }

    return val;
}

word radarUnit::find_max ( word in[][MAX_N], word row ) {
    word k;
    word val = INT_MIN;
    for (k=0; k<MAX_N; k++) {
         if( in[row][k] > val )
                  val =  in[row][k];
    }
    return val;
}

void radarUnit::extract_valid( real in[][MAX_N], word row_sel[], word col_sel[], real out[][MAX_N], word m, word n) {
    word i,j, i_out, j_out;
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

/*
M=dMat(~coverRow,~coverColumn)
*/
void radarUnit::extract_not_valid( real in[][MAX_N], word row_sel[], word col_sel[], word m, word n, real out[][MAX_N], word *r, word *c) {
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

/*
        dMat(~coverRow,~coverColumn)=M-minval;
*/
void radarUnit::insert_to_not_valid( real in[][MAX_N], word m, word n, real out[][MAX_N], word row_sel[], word col_sel[] ) {
    word i,j, i_in, j_in;
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




/*
        assignment(validRow,validCol) = starZ(1:nRows,1:nCols);
*/
void radarUnit::insert_to_valid( word in[][MAX_N], word m, word n, word out[][MAX_N], word row_sel[], word col_sel[] ) {
    word i,j, i_in, j_in;
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




void radarUnit::munkres( real costMat[][MAX_N], word rows, word cols, real *cost, word assign[][MAX_N] ) {

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

    #if(0)
    %*************************************************
    % Munkres' Assignment Algorithm starts here
    %*************************************************

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %   STEP 1: Subtract the row minimum from each row.
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
     dMat = bsxfun(@minus, dMat, min(dMat,[],2));
    #endif

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

/* 4x4 matrices (P_e[][4], real P_p[][4]) are reffered as 16x1 vectors as 3D arrays are used in MTT function */
void radarUnit::correction ( real Y_e[], real *P_e, real Y_p[], real *P_p, real r, real fi, real R[][2]) {

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

void radarUnit::new_tg_ident ( word nn_NTI, word min_NTI, word NTI[], real r_last_obs[], real fi_last_obs[],
                    real r, real fi, real dif_d, real dif_fi, word nn_track, word cl[], word max_nn_tr,
                    word *new_track, word clearing[]
                  ) {
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

void radarUnit::polar2cartesian ( real Y_e[], real *x, real *y) {
   *x = *y = 0.0;

   *x = Y_e[0]* (real)cos(Y_e[2]);
   *y = Y_e[0]* (real)sin(Y_e[2]);
}

void radarUnit::MTT2 (real P[][MAX_N], real r[], real q[], real dif_d, real dif_fi, word min_OLGI, word min_NTI, real T[][MAX_N], word *start ) {

    word k, j, t, track;
    real cost;
    word A[MAX_N][MAX_N];
    word max_MA, tmp;
    word Clearing[3];
    word new_track;


    word nn_NTI;

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

float* radarUnit::MTT(float* P_mem,float r[], float q[],float dif_d,float dif_fi, int min_OLGI,int min_NTI){
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
