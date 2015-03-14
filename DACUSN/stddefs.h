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

enum visualization_schema
{
    COMMON_FLOW = 0, ///< This schema displays objects as a very simple circles fastly changing their positions.
    COMET_EFFECT = 1 ///< Is displaying targets as moving comet with a little history positions. The history may be changed as the animation duration.
};


#endif // STDDEFS

