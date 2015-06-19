/**
 * @file radarunit.cpp
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Definitions of radarUnit class methods.
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

#include "radarunit.h"

radarUnit::radarUnit(int radarId, double x_pos, double y_pos, double rot_Angle, bool enable)
{
    radar_id = radarId;
    max_recursion = 5;

    xpos = x_pos;
    ypos = y_pos;
    rotAngle = rot_Angle;

    tempX = tempY = 0.0;

    dataList = new QList<rawData * >;

    enabled = enable;

    /* FOR TEST PURPOSES ONLY - TEST OF MTT_PURE LIB */
    mtt_p = new mtt_pure;
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
            qDebug() << "Running MTT for radar: " << radar_id;

            if(data->getRecieverMethod()==RS232)
                mtt_p->MTT(data->getUwbPacketCoordinates(), r, q, diff_d, diff_fi, min_OLGI, min_NT);
            #if defined (__WIN32__)
            else if(data->getRecieverMethod()==SYNTHETIC) mtt_p->MTT(data->getSyntheticCoordinates(), r, q, diff_d, diff_fi, min_OLGI, min_NT);
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

void radarUnit::resetMTT()
{
    delete mtt_p;
    mtt_p = new mtt_pure;
}
