#include "rawdata.h"

rawData::rawData()
{
    syntheticData = NULL;
}

rawData::~rawData()
{
    // free all memory
    if(syntheticData!=NULL)
    {
        if(syntheticData->coordinates != NULL) delete syntheticData->coordinates;
        if(syntheticData->toas != NULL) delete syntheticData->toas;

        delete syntheticData;
    }
}

void rawData::setSyntheticRadarId(short id)
{
    if(syntheticData!=NULL)
    {
        syntheticData->radar_id = id;
    } else {
        createSyntheticDataStruct();
        syntheticData->radar_id = id;
    }
}

void rawData::setSyntheticTime(double time)
{
    if(syntheticData!=NULL)
    {
        syntheticData->time = time;
    } else {
        createSyntheticDataStruct();
        syntheticData->time = time;
    }
}

void rawData::setSyntheticTargetsCount(short count)
{
    if(syntheticData!=NULL)
    {
        syntheticData->targets_count = count;
    } else {
        createSyntheticDataStruct();
        syntheticData->targets_count = count;
    }
}

void rawData::setSyntheticCoordinates(double *coords)
{
    if(syntheticData!=NULL)
    {
        if(syntheticData->coordinates!=NULL) delete syntheticData->coordinates;

        syntheticData->coordinates = coords;
    } else {
        createSyntheticDataStruct();
        syntheticData->coordinates = coords;
    }
}

void rawData::setSyntheticToas(double *toas)
{
    if(syntheticData!=NULL)
    {
        if(syntheticData->toas!=NULL) delete syntheticData->toas;

        syntheticData->toas = toas;
    } else {
        createSyntheticDataStruct();
        syntheticData->toas= toas;
    }
}

void rawData::createSyntheticDataStruct()
{
    syntheticData = new synthetic_data;
    syntheticData->coordinates = NULL;
    syntheticData->toas = NULL;
    syntheticData->radar_id = syntheticData->targets_count = syntheticData->time = 0;
}

