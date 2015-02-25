#ifndef STDDEFS
#define STDDEFS

/**
 * @brief The reciever_method enum covers all possible kinds of recieve methods for UWB sensor network.
 */
enum reciever_method
{
    UNDEFINED = 0, ///< May be used for situations when no method is needed at all (idle method)
    SYNTHETIC = 1 ///< Is used when the data are read by server application from file and sent throught pipe
};


#endif // STDDEFS

