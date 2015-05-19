#include "rawdata.h"

rawData::rawData()
{
    syntheticData = NULL;
    uwbPacketData = NULL;
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
    if(uwbPacketData!=NULL)
    {
        delete uwbPacketData;
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

void rawData::setSyntheticCoordinates(float *coords)
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

void rawData::setSyntheticCoordinatesPointer(float *coords)
{
    if(coords!=NULL) syntheticData->coordinates = coords;
}

void rawData::setSyntheticToas(float *toas)
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

void rawData::setUwbPacketRadarId(int id)
{
    if(uwbPacketData!=NULL)
    {
        uwbPacketData->radar_id = id;
    } else {
        createUwbPcketDataStruct();
        uwbPacketData->radar_id = id;
    }
}

void rawData::setUwbPacketRadarTime(int time)
{
    if(uwbPacketData!=NULL)
    {
        uwbPacketData->radar_time = time;
    } else {
        createUwbPcketDataStruct();
        uwbPacketData->radar_time = time;
    }
}

void rawData::setUwbPacketPacketNumber(int number)
{
    if(uwbPacketData!=NULL)
    {
        uwbPacketData->packet_count = number;
    } else {
        createUwbPcketDataStruct();
        uwbPacketData->packet_count = number;
    }
}

void rawData::setUwbPacketTargetsCount(int count)
{
    if(uwbPacketData!=NULL)
    {
        uwbPacketData->targets_count = count;
    } else {
        createUwbPcketDataStruct();
        uwbPacketData->targets_count = count;
    }
}

void rawData::setUwbPacketCoordinates(float *coordinates)
{
    if(uwbPacketData!=NULL)
    {
        if(uwbPacketData->coordinates!=NULL) delete uwbPacketData->coordinates;

        uwbPacketData->coordinates = coordinates;
    } else {
        createUwbPcketDataStruct();
        uwbPacketData->coordinates = coordinates;
    }
}

void rawData::createSyntheticDataStruct()
{
    syntheticData = new synthetic_data;
    syntheticData->coordinates = NULL;
    syntheticData->toas = NULL;
    syntheticData->radar_id = syntheticData->targets_count = syntheticData->time = 0;
}

void rawData::createUwbPcketDataStruct()
{
    uwbPacketData = new uwb_packet;
    uwbPacketData->coordinates = NULL;
    uwbPacketData->packet_count = uwbPacketData->radar_id = uwbPacketData->radar_time = uwbPacketData->targets_count = 0;
}

