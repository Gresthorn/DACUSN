#ifndef RECIEVER_H
#define RECIEVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <ctime>
#include <QDebug>

#include "stddefs.h"
#include "rawdata.h"
#include "rs232.h"
#include "uwbpacketclass.h"

/**
 * @file reciever.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class which provides interface for obtaining data from radar network
 *
 * @section DESCRIPTION
 *
 * The reciever class provides the interface for recieving data from
 * UWB sensor network. Its used to get data depending on protocol
 * used for communication. Supported protocols/methods have their internal
 * codes. code must be specified manually as well as method for data
 * parsing. This is possible from the external approach by public
 * methods. Data are saved into structure in needed format which pointer
 * can be retrieved by public methods. Reciever class does not free
 * allocated memory of structures and relies on higher functions.
 * New data are automatically saved into structures and returned to higher
 * functions. After that, new data can be obtained.
 *
 */

class reciever
{
public:

    /**
     * @brief                         Main constructor of reciever class
     * @param[in]   recieve_method    Is used to identify the method by which data should be get
     *
     * Class constructor. The parameter is specifying the method of data recieving. Different
     * approaches are used when test environment pipe or shared memory are used, or the real
     * radar network is connected. Of course also different protocols may require different
     * maintainance algorithms. This parameter is saved internally and used for making those
     * choices but can be changed at any time.
     */
    reciever(reciever_method recieveMethod);

    /**
     * @brief                       Specially if serial link communication is selected, also basic COM port configurations must be passed. This is overloaded constructor.
     * @param[in] recieveMethod     Is used to identify the method by which data should be get
     * @param[in] comport_ID        New comport index for opening the comport
     * @param[in] baud_rate         New baudrate for serial link communication
     * @param[in] comport_mode      Mode for comport communication (parity, stop bits, one send word length)
     */
    reciever(reciever_method recieveMethod, int comport_ID, int baud_rate, char * comport_mode);

    ~reciever();

    /**
     * @brief   This method is responsible for waiting for data and its conversion into needed format
     * @return  Return value is the object with all important data in their for higher classes discernible form
     *
     * This function waits for radar data acoording to method specified by 'r_method' value. If success,
     * it returns the object containing all data retrieved. Else it returns NULL value and stores the
     * error message in 'statusMsg' string.
     */
    rawData * listen(void);

    /**
     * @brief   Method returning the data getting method currently used
     * @return  The return value is simple short value identifying method currently used
     *
     * Method returning the identifier representing the data obtaining algorithm currently used by 'reciever'.
     */
    reciever_method curr_method_code(void) { return r_method; }

    /**
     * @brief                       Setting new data obtaining method
     * @param[in]   recieve_method  Parameter with the code of desired method.
     * @param[in]   kill            Parameter which ensures that running method will be replaced with UNDEFINED even if it could not be closed.
     * @return                      Boolean value representing if the modification was successfull.
     *
     * If higher classes or the user want to change the currently used method of data obtaining, this
     * method will try to do that. Note, that in some cases it may not be possible and it is recommended
     * to check the code after this function is finished. If no change is done, the message may be
     * stored in 'statusMsg' string. If 'kill' parameter is set to true, the currently running method will be
     * replaced by UNDEFINED state even if it could not be stopped correctly. Note that this will
     * ensure the new method start but may cause unexpected behaviour.
     */
    bool set_new_method_code(reciever_method recieveMethod, bool kill = false);

    /**
     * @brief Free the memory of last known recieved data object.
     */
    void free_last_data(void) { if(last_data_pt!=NULL) { delete last_data_pt; last_data_pt = NULL; } }

    /**
     * @brief Method for returning the last result of calibration.
     * @return The return value is true if new method was sucessfully set up and can be used.
     *
     * If calibration is used for setting up new data obtaining method, it is recommended to check if
     * everything was done alright by this method.
     */
    bool calibration_status(void) { return calibrationStatus; }

    /**
     * @brief Function return the pointer to last message produced by 'reciever' object.
     * @return The return value is constant pointer to string with the last message.
     *
     * If the 'reciever' object considers some action as too important so it requires brief description, this
     * message is stored inside the object and can be accessed by this method. Note that object may store only
     * one message at time.
     */
    const char * check_status_message(void) { return statusMsg; }

private:
    bool calibrationStatus; ///< The boolean result of wether the method was set up successfully.

    rawData * last_data_pt; ///< The object for storing data

    reciever_method r_method; ///< The code of currently used method.

    char * statusMsg; ///< The pointer to the last message produced by object

    /**
     * @brief Method for creating a message which can be retrieved by higher classes for brief information about status
     * @param[in] msg Is the only parameter containing the string
     *
     * This function is used internally to generate a message with brief information about whats happening internally in
     * object. Such messages are then used for example for brief information about the errors. Note that object can hold
     * only the one message at time. It is useless to call this function more than one time per another function.
     */
    void set_msg(const char * msg);

    /**
     * @brief                       Function for setting up new data obtaing method.
     * @param[in] recieve_method    The code of desired method.
     * @return                      Return value is true if the desired method was successfully set up.
     *
     * This function is responsible for all preparations and settings when data communication is establishing.
     * If everything is done successfully, the return value is true. If no, the return value is false and previous
     * active method is used.
     */
    bool calibrate(reciever_method recieveMethod);

    /**
     * @brief Function is responsible for secure cancellation of previous methd already running.
     * @return If the cancellation was successful,the return value is true, else false
     *
     * This function will try to quit from previous method. Each method may require another algorithms for such action.
     * If the cancellation is successful, the return value is true. This function relies on another functions to modify 'r_method'.
     * If UNDEFINED state is present, the 'listen' function has no effect and results in sleep cycle for 2 seconds. Note, that
     * if cancellation fails, probably it will not be possible to refresh the method again. In this case, UNDEFINED state
     * is specified by another functions. Else, previous state is set and the return value is false.
     */
    bool cancel_previous_method(void);

    //------------------------------------------ SERIAL LINK METHOD -------------------------------------------

    int comPort; ///< Specifies the index of COM port in operating system, which is used for data recieving

    int comPortBaudRate; ///< Holds the information about speed used for serial link commuication

    char * comPortMode; ///< Used for serial link communication initialization with some options. See this site for more information: http://www.teuniz.net/RS-232/

    bool comPortCallibration; ///< If COM port was successfuly opened, this is set to TRUE, else is set to FALSE

    uwbPacketRx * packetReciever; ///< If COM port communication is pending, this packet ensures data recieving (no more packet recieving objects are needed)

    /**
     * @brief When recieving data from the COM port, we are getting the UWB radar packet and so we need to read it and convert it to rawData format.
     * @return Pointer to the new rawData object with all values being correctly set.
     */
    rawData * extract_RS232_radar_packet(void);

    //------------------------------------------ PIPE METHOD --------------------------------------------------

    HANDLE pipe_connection_handler; ///< Handler of pipe communication if this kind of communication is desired
    const int maximum_pipe_size; ///< Is the maximum of characters possible in pipe buffer


    /**
     * @brief Closing pipe connection
     * @param[in] connection Is parameter with handler identifying connection.
     *
     * This function is used internally by 'calibrate()' function when setting up new  algorithms for obtaining radar
     * data if the previous method was pipe communication for closing this connection.
     */
    void closeConnection(HANDLE connection);

    /**
     * @brief Establishing the pipe channel with the server application.
     * @return The return value is handler of communication pipe.
     *
     * If pipe communication is used (usually for test purposes), the server application like 'UWB Coordinate reader'
     * can be used to demonstrate the UWB network behaviour. Data are obtained by pipe channel which is established in
     * this function and the handler is returned.
     */
    HANDLE connectToPipe(void);

    /**
     * @brief readFromPipe Function responsible for waiting for new data and their reading from pipe.
     * @param[in] connection Is handler of pipe channel used.
     * @param[in] buffer Is the 'char' array where the data are stored while their processing.
     * @return The return value identifies if reading was successfull.
     *
     * This function is responsible for reading data from pipe channel and their temporary storing in
     * 'char' array. If server is disconnected, or another error occured, the return value is false.
     */
    bool readFromPipe(HANDLE connection, char * buffer);

    /**
     * @brief Function for extraction of data from their text/character form. Used within pipe communication method.
     * @param msg Is pointer to the array of characters read from pipe channel.
     * @return The return value is complete object filled with extracted data.
     *
     * After the pipe connection is established and data are read, they are still in a text form which needs to be
     * retransformed into real numbers and radar data object which is created and then returned.
     */
    rawData * extract_synthetic(char * msg);

    /**
     * @brief Standard C++ 'strsep' function if it is not implemented in the standard <string.h> library
     *
     * You should delete this function prototype and its definition if it is availible in your library.
     * Do not use 'strtok' alternative.
     */
    char * strsep( char ** stringp, const char * delim );


};

#endif // RECIEVER_H
