#ifndef RAWDATA_H
#define RAWDATA_H

#include <stdlib.h>

/**
 * @file rawdata.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class which provides interface for storing raw data from radar network
 *
 * @section DESCRIPTION
 *
 * The 'rawData' class is very simple class that serves as a data storage for all kinds
 * of 'reciever' methods/algorithms. It contains all types of structures that can be
 * required by program and have methods for manupulating with those data. As the only
 * class for data concentrating it is very easy to migrate these across the higher classes
 * if needed and represents the only base return value of 'reciever' class.
 */

class rawData
{
public:
    /**
     * @brief The contructor of 'rawData' initializes all structure pointers to NULL
     *
     * The only purpose of constructor is to initialize all structure pointers to NULL.
     * Memory is then allocated when needed. It is always the responsibility of higher
     * classes to ensure that data are saved or processed before they are overwritten
     * so no stack is needed.
     */
    rawData();  
    ~rawData();

    /**
     * @brief Returns radar id if the value is availible else -1.
     * @return The return value is the id of radar.
     */
    short getSyntheticRadarId(void) { return (syntheticData!=NULL) ? syntheticData->radar_id : -1; }

    /**
     * @brief Returns time of data measure/generation if the value is availible else -1.
     * @return The return value is the time of data measure/generation.
     */
    double getSyntheticTime(void) { return (syntheticData!=NULL) ? syntheticData->time : -1; }

    /**
     * @brief Returns the number of visible targets if the value is availible, else -1.
     * @return The return value is the number of visible targets.
     */
    short getSyntheticTargetsCount(void) { return (syntheticData!=NULL) ? syntheticData->targets_count : -1; }

    /**
     * @brief Returns the pointer to the array where all coordinates are stored if is availible else NULL.
     * @return The return value is the pointer to the array where all coordinates are stored.
     */
    double * getSyntheticCoordinates(void) { return (syntheticData!=NULL) ? syntheticData->coordinates : NULL; }

    /**
     * @brief Returns the pointer to the array where all toas are stored if is availible else NULL.
     * @return The return value is the pointer to the array where all toas are stored.
     */
    double * getSyntheticToas(void) { return (syntheticData!=NULL) ? syntheticData->toas : NULL; }

    /**
     * @brief Sets new radar ID
     * @param The only parameter is radar ID
     *
     * If structure does not exist, new structure is created, else the old value is overwritten.
     */
    void setSyntheticRadarId(short id);

    /**
     * @brief Sets new time of data measure/generation.
     * @param The only parameter is time of data measure/generation.
     *
     * If structure does not exist, new structure is created, else the old value is overwritten.
     */
    void setSyntheticTime(double time);

    /**
     * @brief Sets visible targets count for the current radar unit.
     * @param The only parameter is targets count for the current radar unit.
     *
     * If structure does not exist, new structure is created, else the old value is overwritten.
     */
    void setSyntheticTargetsCount(short count);

    /**
     * @brief Sets new array pointer of coordinates for all targets.
     * @param The only parameter is the array pointer of coordinates for all targets.
     *
     * If structure does not exist, new structure is created and pointer set. Else the
     * old array is deleted and new pointer is set.
     */
    void setSyntheticCoordinates(double * coords);

    /**
     * @brief Sets new array pointer of toas for all targets.
     * @param The only parameter is the array pointer of toas for all targets.
     *
     * If structure does not exist, new structure is created and pointer set. Else the
     * old array is deleted and new pointer is set.
     */
    void setSyntheticToas(double * toas);

private:

    /**
     * @brief Method for allocating memory and initialization of 'syntheticData' structure
     */
    void createSyntheticDataStruct(void);

    /**
     * @brief The 'synthetic_data' structure holds data obtained from one string catched in pipe
     *
     * The structure contains all information from one radar unit for all targets. The coordinates
     * and TOAs are stored in arrays gradually. For example the [x, y] and TOAs [left, right] of the
     * second target are positioned on indexes 2 and 3 of 'coordinates' and 'toas'.
     */
    struct synthetic_data {
        short radar_id; ///< Stores the main radar identificator
        double time;    ///< Represents the time, when all values were recieved
        short targets_count; ///< The number of targets visible by radar with 'radar_id'
        double * coordinates; ///< The array of coordinates [x, y] gradually
        double * toas; ///< The TOA value measured in time or distance depending on generator setup for left and right antenna gradually
    };

    synthetic_data * syntheticData; ///< The structure for storing synthetic data
};

#endif // RAWDATA_H
