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

#ifndef RAWDATA_H
#define RAWDATA_H

#include <stdlib.h>
#include <QDebug>

#include "stddefs.h"

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
     * @brief This function serves higher classes to find out, what method was used for the particular data
     * @return The return value is the 'reciever_method' structure.
     *
     * As far as the user may change the data input source when the application is running, the data recieved
     * may require some special treatment in some cases. Also higher classes must know what set of methods they
     * should to use. In all situations this method remains the same so the 'reciever_method' check is convinient.
     */
    reciever_method getRecieverMethod(void) { return method; }

    /**
     * @brief This function serves 'reciever' class to save, what method was used for the particular data.
     * @param[in] recieverMethod Represents the enum identificator of method itself.
     *
     * As far as the user may change the data input source when the application is running, the data recieved
     * may require some special treatment in some cases. Also higher classes must know what set of methods they
     * should to use. Saving the method what the particular data were recieved is important for higher classes.
     */
    void setRecieverMethod(reciever_method recieverMethod) { method = recieverMethod; }

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
    float * getSyntheticCoordinates(void) { return (syntheticData!=NULL) ? syntheticData->coordinates : NULL; }

    /**
     * @brief Returns the pointer to the array where all toas are stored if is availible else NULL.
     * @return The return value is the pointer to the array where all toas are stored.
     */
    float * getSyntheticToas(void) { return (syntheticData!=NULL) ? syntheticData->toas : NULL; }

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
    void setSyntheticCoordinates(float * coords);

    /**
     * @brief Sets new pointer for synthetic coordinates. Pointer must point to extisting array (no control is done here).
     * @param[in] coords Is the pointer to array of float numbers initialized by higher classes.
     *
     * This function is used when some operations are done by another classes and new array is requiered to be set instead. (e.g.
     * Zeroing unneeded space). This function just replace the pointer. No deletions or controls are implemented and the higher
     * classes are responsible for doing so.
     */
    void setSyntheticCoordinatesPointer(float *coords);

    /**
     * @brief Sets new array pointer of toas for all targets.
     * @param The only parameter is the array pointer of toas for all targets.
     *
     * If structure does not exist, new structure is created and pointer set. Else the
     * old array is deleted and new pointer is set.
     */
    void setSyntheticToas(float * toas);

    /**
     * @brief Allows to set new radar ID to appropriate structure. If structure does not exist yet, it will be created.
     * @param[in] id New radar id.
     */
    void setUwbPacketRadarId(int id);

    /**
     * @brief Sets the new radar time for packet.  If structure does not exist yet, it will be created.
     * @param[in] time New radar time for packet.
     */
    void setUwbPacketRadarTime(int time);

    /**
     * @brief Allows to set new packet number. If structure does not exist yet, it will be created.
     * @param[in] number New packet number.
     */
    void setUwbPacketPacketNumber(int number);

    /**
     * @brief Used when needed to update targets count in packet. Use carefully! If will not be equall to coordinates array count divided by 2, program may crash. If structure does not exist yet, it will be created.
     * @param[in] count New number of targets in array.
     */
    void setUwbPacketTargetsCount(int count);

    /**
     * @brief Sets the new pointer to the array of coordinates [x, y]. If some array is already set, will be deleted first. If structure does not exist yet, it will be created.
     * @param[in] coordinates New pointer to array of coordinates.
     */
    void setUwbPacketCoordinates(float * coordinates);

    /**
     * @brief Retrieves the radar id which was the packet send from.
     * @return Radar id as integer number. If uwb packet structure was not created yet, return value is -1.
     */
    int getUwbPacketRadarId(void) { return (uwbPacketData!=NULL) ? uwbPacketData->radar_id : -1; }

    /**
     * @brief Retrieves radar time as integer number. This time is used for cross radar packet synchronization.
     * @return Radar time as integer number. If uwb packet structure was not created yet, return value is -1.
     */
    int getUwbPacketRadarTime(void) { return (uwbPacketData!=NULL) ? uwbPacketData->radar_time : -1; }

    /**
     * @brief Allows to obtain packet number. Each next packet must have the previous packet number +1. If this rule is not holded, some packets are lost.
     * @return Integer number of packet number.  If uwb packet structure was not created yet, return value is -1.
     */
    int getUwbPacketPacketNumber(void) { return (uwbPacketData!=NULL) ? uwbPacketData->packet_count : -1; }

    /**
     * @brief Retrieves the number of targets seen by specific radar.
     * @return The return value is number of targets as integer number. If uwb packet structure was not created yet, return value is -1.
     */
    int getUwbPacketTargetsCount(void) { return (uwbPacketData!=NULL) ? uwbPacketData->targets_count : -1; }

    /**
     * @brief Function allows access to pointer of array where all [x, y] coordinates for each target is stored.
     * @return Pointer to array of float numbers with [x, y] coordinates.
     */
    float * getUwbPacketCoordinates(void) { return (uwbPacketData!=NULL) ? uwbPacketData->coordinates : NULL; }


private:

    reciever_method method;

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
        float * coordinates; ///< The array of coordinates [x, y] gradually
        float * toas; ///< The TOA value measured in time or distance depending on generator setup for left and right antenna gradually
    };

    /**
     * @brief Creates new uwb packet radar data.
     *
     * The structure contains all information extracted from one packet of one radar unit. Coordinates
     * of targets are stored in array which can be easily read since also targets count is availible.
     * Structure contains radar time and packet count information for synchronization and error check reasons.
     */
    void createUwbPcketDataStruct(void);

    /**
     * @brief The 'uwb_packet' structure holds data obtained from one packet (of one radar unit)
     *
     * The structure contains all information from one radar unit for all targets. The coordinates
     * are stored in array and their count is twice the number of targets. Radar time is used for time
     * synchronization and corrections when synchronizing data from few radars. Packet counts can
     * detect if some packets were lost.
     */
    struct uwb_packet {
        int radar_id; ///< Stores radar identifier (each radar must have own identifier, OPERATOR has 0)
        float * coordinates; ///< Array of coordinates [x, y]
        int targets_count; ///< Number of targets (or [x, y] combinations in array)
        int radar_time; ///< Radar time, for synchronization
        int packet_count; ///< Packet count, used for detecting lost packets
    };

    synthetic_data * syntheticData; ///< The structure for storing synthetic data

    uwb_packet * uwbPacketData; ///< The structure for storing uwb radar packet data
};

#endif // RAWDATA_H
